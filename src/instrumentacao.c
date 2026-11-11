/*
 * instrumentacao.c - medicao manual (sem profiler).
 *
 * TEMPO : clock_gettime(CLOCK_MONOTONIC) antes e depois da chamada.
 * HEAP  : inst_calloc/inst_free somam e subtraem os bytes; o maximo e o pico.
 * PILHA : "pintura". Pinta 4096 B da pilha com 0xAA, chama a funcao e ve ate onde a
 *         tinta foi apagada: pilha = (referencia) - (ponto mais fundo tocado).
 *
 * Limitacoes: a pilha e limite superior (inclui o quadro de inst_medir_memoria),
 * depende de plataforma/compilador (159 B em Windows/MinGW GCC 16.1 -O2) e satura em
 * ~4096 B; o heap so conta o que passa por inst_calloc.
 */
#define _GNU_SOURCE                         /* habilita clock_gettime e extensoes */
#include "instrumentacao.h"                 /* tipo FuncaoDedup e prototipos */

#include <stdio.h>                          /* fprintf, stderr */
#include <stdlib.h>                         /* calloc, free, exit */
#include <time.h>                           /* clock_gettime, struct timespec */

/* ---------- HEAP ---------- */

static size_t heap_atual = 0;               /* bytes alocados neste momento */
static size_t heap_pico  = 0;               /* maior valor que heap_atual ja teve */

void *inst_calloc(size_t bytes)
{
    void *p = calloc(1, bytes);             /* aloca "bytes" zerados */
    if (p == NULL) {                        /* falhou? */
        fprintf(stderr, "sem memoria\n");   /* avisa no erro padrao */
        exit(1);                            /* encerra: nao da para medir sem memoria */
    }
    heap_atual += bytes;                    /* soma ao total atual */
    if (heap_atual > heap_pico) heap_pico = heap_atual;   /* atualiza o pico */
    return p;                               /* devolve o bloco ao chamador */
}

/* "bytes" deve ser o mesmo valor passado a inst_calloc. */
void inst_free(void *p, size_t bytes)
{
    free(p);                                /* devolve o bloco ao sistema */
    heap_atual -= bytes;                    /* subtrai do total atual */
}

/* ---------- TEMPO ---------- */

/* Mede so a chamada a f, com relogio monotonico (a resolucao limita n pequeno). */
double inst_medir_tempo(FuncaoDedup f, uint16_t *v, size_t n, size_t *n_unicos)
{
    struct timespec a, b;                   /* a = instante antes, b = instante depois */
    clock_gettime(CLOCK_MONOTONIC, &a);     /* le o relogio antes */
    *n_unicos = f(v, n);                    /* roda a estrategia (o que se mede) */
    clock_gettime(CLOCK_MONOTONIC, &b);     /* le o relogio depois */
    /* tempo = diferenca de segundos (tv_sec) + diferenca de nanossegundos (tv_nsec) / 10^9 */
    return (double)(b.tv_sec - a.tv_sec) + (double)(b.tv_nsec - a.tv_nsec) / 1e9;
}

/* ---------- PILHA ---------- */

#define TAM_PINTURA 4096                    /* bytes de pilha que serao pintados */
#define TINTA       0xAA                    /* valor da tinta (10101010 em binario) */

static volatile unsigned char *area_pintada;   /* inicio da area pintada */

/* Pinta a pilha. noinline: a area fica onde a funcao medida vai rodar em seguida. */
__attribute__((noinline)) static void pintar_pilha(void)
{
    volatile unsigned char area[TAM_PINTURA];                    /* vetor local: vive na pilha */
    for (size_t i = 0; i < TAM_PINTURA; i++) area[i] = TINTA;    /* pinta byte a byte */
    /* Guardar o endereco de um local e proposital: so o endereco importa. */
#pragma GCC diagnostic push                 /* guarda o estado dos avisos */
#pragma GCC diagnostic ignored "-Wdangling-pointer"   /* silencia o aviso de ponteiro para local */
    area_pintada = area;                    /* guarda onde a area esta */
#pragma GCC diagnostic pop                  /* restaura os avisos */
}                                           /* o quadro some, mas a tinta fica na memoria */

size_t inst_medir_memoria(FuncaoDedup f, uint16_t *v, size_t n, size_t *heap_pico_out)
{
    volatile char referencia;          /* marca o "topo" para medir a pilha */

    heap_atual = 0;                    /* zera o total atual */
    heap_pico  = 0;                    /* zera o pico: so conta o que f aloca */

    pintar_pilha();                    /* pinta 4096 B da pilha com 0xAA */
    f(v, n);                           /* roda a estrategia: ela apaga a tinta onde a pilha chega */

    /* Do endereco mais baixo para cima ate achar tinta apagada = ponto mais fundo. */
    size_t intactos = 0;               /* quantos bytes continuam pintados */
    while (intactos < TAM_PINTURA && area_pintada[intactos] == TINTA) intactos++;   /* conta a tinta intacta */

    size_t pilha = (size_t)((uintptr_t)&referencia -          /* endereco do topo */
                            (uintptr_t)(area_pintada + intactos));   /* menos o ponto mais fundo tocado */

    *heap_pico_out = heap_pico;        /* entrega o pico de heap ao chamador */
    return pilha;                      /* devolve a pilha usada, em bytes */
}
