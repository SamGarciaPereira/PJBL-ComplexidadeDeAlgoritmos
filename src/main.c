/*
 * main.c - demonstracao, benchmark e testes obrigatorios (Secao 8).
 *
 * Compilar: gcc -O2 -std=gnu11 -Isrc -o main src/main.c src/embarca_heapsort.c \
 *                src/datacenter_bitmap.c src/instrumentacao.c
 * Rodar:    ./main [resultados.csv]    (o CSV e opcional)
 *
 * Roda: corretude (n <= 10, com/sem duplicatas, vazio); as 2 estrategias em
 * n = 10^3, 10^5 e 10^7 (tempo e memoria); e os limites (Embarca <= 256 B,
 * Datacenter < 2 s em 10^7). Retorna 0 so se tudo passar.
 * estrategias[0] = Embarca, estrategias[1] = Datacenter (os limites dependem da ordem).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datacenter_bitmap.h"
#include "embarca_heapsort.h"
#include "instrumentacao.h"

#define REPETICOES        5
#define LIMITE_EMBARCA    256     /* bytes */
#define LIMITE_DATACENTER 2.0     /* segundos para n = 10^7 */

typedef struct {
    const char *nome;
    const char *cliente;
    FuncaoDedup f;
} Estrategia;

static const Estrategia estrategias[] = {
    { "ordenacao_heapsort_inplace", "Embarca",    embarca_dedup    },
    { "bitmap_65536_bits",          "Datacenter", datacenter_dedup },
};

/* ---------- 1. Corretude (casos pequenos, verificaveis a mao) ---------- */

/* Roda f numa copia da entrada e compara com o esperado como conjunto (a ordem da saida difere). */
static int caso(FuncaoDedup f, const uint16_t *entrada, size_t n,
                const uint16_t *esperado, size_t m)
{
    uint16_t v[10];
    if (n) memcpy(v, entrada, n * sizeof *v);
    size_t k = f(v, n);
    if (k != m) return 0;
    for (size_t i = 0; i < m; i++) {            /* cada esperado aparece na saida */
        size_t j = 0;
        while (j < k && v[j] != esperado[i]) j++;
        if (j == k) return 0;
    }
    return 1;
}

static int testes_corretude(void)
{
    const uint16_t com_dup[]  = { 5, 3, 5, 1, 3 },  com_dup_ok[]  = { 1, 3, 5 };
    const uint16_t sem_dup[]  = { 9, 2, 7 },        sem_dup_ok[]  = { 2, 7, 9 };
    const uint16_t iguais[]   = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 }, iguais_ok[] = { 4 };
    const uint16_t extremos[] = { 65535, 0, 65535, 0 }, extremos_ok[] = { 0, 65535 };
    const uint16_t um[]       = { 42 };
    int falhas = 0;

    for (size_t e = 0; e < 2; e++) {
        FuncaoDedup f = estrategias[e].f;
        int ok = caso(f, com_dup, 5, com_dup_ok, 3)
              && caso(f, sem_dup, 3, sem_dup_ok, 3)
              && caso(f, iguais, 10, iguais_ok, 1)
              && caso(f, extremos, 4, extremos_ok, 2)
              && caso(f, um, 1, um, 1)
              && caso(f, NULL, 0, NULL, 0);      /* entrada vazia */
        printf("  %-10s corretude (n <= 10, com/sem duplicatas, vazio) ... %s\n",
               estrategias[e].cliente, ok ? "OK" : "FALHOU");
        falhas += !ok;
    }
    return falhas == 0;
}

/* ---------- 2. Demonstracao e medicoes ---------- */

/* Gerador pseudoaleatorio simples (xorshift32): mesma semente, mesmos dados. */
static uint32_t estado;
static uint16_t proxima_leitura(void)
{
    estado ^= estado << 13;
    estado ^= estado >> 17;
    estado ^= estado << 5;
    return (uint16_t)(estado >> 8);
}

static int comparar_double(const void *a, const void *b)
{
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

int main(int argc, char **argv)
{
    const size_t tamanhos[] = { 1000, 100000, 10000000 };   /* 10^3, 10^5, 10^7 */
    const size_t n_tam = sizeof tamanhos / sizeof tamanhos[0];
    const size_t n_max = tamanhos[n_tam - 1];

    printf("Casos de teste de corretude:\n");
    int corretude_ok = testes_corretude();

    FILE *csv = NULL;
    if (argc > 1) {
        csv = fopen(argv[1], "w");
        if (!csv) { perror("csv"); return 1; }
        fprintf(csv, "estrategia,cliente,n,repeticao,tempo_s,heap_pico_bytes,"
                     "pilha_pico_bytes,memoria_extra_bytes,n_unicos\n");
    }

    /* Vetor de entrada: malloc comum, fora dos contadores (nao e memoria extra). */
    uint16_t *base     = malloc(n_max * sizeof *base);
    uint16_t *trabalho = malloc(n_max * sizeof *trabalho);
    if (!base || !trabalho) { fprintf(stderr, "sem memoria\n"); return 1; }

    int embarca_ok = 1, datacenter_ok = 1, unicos_ok = 1;

    printf("\n%-10s %-27s %12s %12s %9s %9s %9s\n",
           "n", "estrategia", "tempo_med_s", "tempo_max_s",
           "heap_B", "pilha_B", "extra_B");

    for (size_t t = 0; t < n_tam; t++) {
        size_t n = tamanhos[t];

        estado = 2463534242u;                       /* mesma semente sempre */
        for (size_t i = 0; i < n; i++) base[i] = proxima_leitura();

        size_t unicos_referencia = 0;

        for (size_t e = 0; e < 2; e++) {
            double tempos[REPETICOES];
            size_t heap_max = 0, pilha_max = 0, unicos = 0;

            for (int r = 0; r < REPETICOES; r++) {
                /* Copia nova a cada medicao (a dedup altera o vetor); tempo e memoria separados. */
                size_t heap = 0;

                memcpy(trabalho, base, n * sizeof *base);    /* copia fresca */
                tempos[r] = inst_medir_tempo(estrategias[e].f, trabalho, n, &unicos);

                memcpy(trabalho, base, n * sizeof *base);    /* copia fresca */
                size_t pilha = inst_medir_memoria(estrategias[e].f, trabalho, n, &heap);

                if (heap  > heap_max)  heap_max  = heap;
                if (pilha > pilha_max) pilha_max = pilha;

                if (csv)
                    fprintf(csv, "%s,%s,%zu,%d,%.9f,%zu,%zu,%zu,%zu\n",
                            estrategias[e].nome, estrategias[e].cliente, n, r + 1,
                            tempos[r], heap, pilha, heap + pilha, unicos);
            }

            /* As duas estrategias tem que achar a mesma quantidade de unicos. */
            if (e == 0) unicos_referencia = unicos;
            else if (unicos != unicos_referencia) unicos_ok = 0;

            double ordenados[REPETICOES];
            memcpy(ordenados, tempos, sizeof tempos);
            qsort(ordenados, REPETICOES, sizeof ordenados[0], comparar_double);
            double mediana = ordenados[REPETICOES / 2];
            double maximo  = ordenados[REPETICOES - 1];
            size_t extra   = heap_max + pilha_max;

            printf("%-10zu %-27s %12.6f %12.6f %9zu %9zu %9zu\n",
                   n, estrategias[e].nome, mediana, maximo,
                   heap_max, pilha_max, extra);

            /* Embarca: so memoria reprova. Datacenter: pior rodada, so em n = 10^7. */
            if (e == 0 && extra > LIMITE_EMBARCA) embarca_ok = 0;
            if (e == 1 && n == n_max && maximo >= LIMITE_DATACENTER) datacenter_ok = 0;
        }
    }

    /* ---------- 3. Limites dos clientes ---------- */
    printf("\nLimites:\n");
    printf("  Embarca    memoria extra <= %d bytes em todos os n .... %s\n",
           LIMITE_EMBARCA, embarca_ok ? "OK" : "FALHOU");
    printf("  Datacenter n = 10^7 em < %.0f s (pior das %d rodadas) .. %s\n",
           LIMITE_DATACENTER, REPETICOES, datacenter_ok ? "OK" : "FALHOU");
    printf("  Mesma quantidade de unicos nas duas estrategias ....... %s\n",
           unicos_ok ? "OK" : "FALHOU");

    if (csv) fclose(csv);
    free(base);
    free(trabalho);
    return (corretude_ok && embarca_ok && datacenter_ok && unicos_ok) ? 0 : 1;
}
