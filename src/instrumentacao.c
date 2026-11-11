/*
 * instrumentacao.c
 * Como cada medida e feita:
 *
 * TEMPO   : relogio monotonico (clock_gettime) antes e depois da chamada.
 *
 * HEAP    : inst_calloc/inst_free somam e subtraem os bytes pedidos.
 *           O maior valor da soma durante a chamada e o "pico de heap".
 *
 * PILHA   : tecnica da "pintura". Antes de chamar a funcao, pintamos uma
 *           area de 4096 bytes da pilha (logo abaixo de onde a funcao vai
 *           rodar) com o valor 0xAA. Depois da chamada, olhamos ate onde a
 *           tinta foi apagada: o ponto mais fundo que a funcao tocou.
 *           A pilha cresce para enderecos menores, entao
 *           pilha usada = (endereco de referencia) - (ponto mais fundo).
 *           O endereco de referencia fica no quadro desta funcao, logo
 *           acima do quadro da funcao medida: o numero e um LIMITE SUPERIOR
 *           (pode sobrar algumas dezenas de bytes, nunca faltar).
 *
 * LIMITACOES (declarar no relatorio): a pilha e limite superior, depende de
 * plataforma/compilador (71 B Linux/GCC 13; 159 B Windows/MinGW GCC 16) e satura
 * em ~4096 B; o heap so conta o que passa por inst_calloc.
 */
#define _GNU_SOURCE
#include "instrumentacao.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---------- HEAP ---------- */

static size_t heap_atual = 0;
static size_t heap_pico  = 0;

void *inst_calloc(size_t bytes)
{
    void *p = calloc(1, bytes);
    if (p == NULL) {
        fprintf(stderr, "sem memoria\n");
        exit(1);
    }
    heap_atual += bytes;
    if (heap_atual > heap_pico) heap_pico = heap_atual;
    return p;
}

/* "bytes" deve ser o mesmo valor passado a inst_calloc. */
void inst_free(void *p, size_t bytes)
{
    free(p);
    heap_atual -= bytes;
}

/* ---------- TEMPO ---------- */

/* Mede so a chamada a f, com relogio monotonico (a resolucao limita n pequeno). */
double inst_medir_tempo(FuncaoDedup f, uint16_t *v, size_t n, size_t *n_unicos)
{
    struct timespec a, b;
    clock_gettime(CLOCK_MONOTONIC, &a);
    *n_unicos = f(v, n);
    clock_gettime(CLOCK_MONOTONIC, &b);
    return (double)(b.tv_sec - a.tv_sec) + (double)(b.tv_nsec - a.tv_nsec) / 1e9;
}

/* ---------- PILHA ---------- */

#define TAM_PINTURA 4096
#define TINTA       0xAA

static volatile unsigned char *area_pintada;   /* inicio da area pintada */

/* Pinta TAM_PINTURA bytes da pilha. "noinline" garante que a area fica
 * exatamente onde a funcao medida vai rodar logo depois. */
__attribute__((noinline)) static void pintar_pilha(void)
{
    volatile unsigned char area[TAM_PINTURA];
    for (size_t i = 0; i < TAM_PINTURA; i++) area[i] = TINTA;
    /* Guardar o endereco de uma variavel local e proposital: so queremos o
     * ENDERECO da area (nunca o conteudo) para olhar a pilha depois. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"
    area_pintada = area;
#pragma GCC diagnostic pop
}

size_t inst_medir_memoria(FuncaoDedup f, uint16_t *v, size_t n, size_t *heap_pico_out)
{
    volatile char referencia;          /* marca o "topo" para medir a pilha */

    heap_atual = 0;
    heap_pico  = 0;

    pintar_pilha();
    f(v, n);

    /* Do endereco mais baixo para cima ate achar tinta apagada = ponto mais fundo. */
    size_t intactos = 0;
    while (intactos < TAM_PINTURA && area_pintada[intactos] == TINTA) intactos++;

    size_t pilha = (size_t)((uintptr_t)&referencia -
                            (uintptr_t)(area_pintada + intactos));

    *heap_pico_out = heap_pico;
    return pilha;
}
