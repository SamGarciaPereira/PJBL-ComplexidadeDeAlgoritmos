/*
 * Estrategia 2: ordenacao in-place (heapsort) + varredura de vizinhos. Cliente: EMBARCA.
 *
 * Ordena v sem vetor auxiliar; os iguais ficam vizinhos; uma varredura copia o
 * primeiro de cada grupo para o inicio.
 * Por que heapsort: mergesort exige vetor auxiliar O(n); quicksort recursivo gasta
 * pilha O(log n) (O(n) no pior caso); heapsort e iterativo, in-place e Theta(n log n).
 * Custo: tempo Theta(n log n) sempre, pilha O(1), heap 0 B (limite da Embarca: 256 B).
 */
#include "embarca_heapsort.h"

/* Desce v[i] no heap maximo v[0..n) (filhos de i: 2i+1 e 2i+2). O(log n), sem recursao. */
static void peneirar(uint16_t *v, size_t i, size_t n)
{
    uint16_t x = v[i];                      /* valor que vai descer */
    for (;;) {
        size_t filho = 2 * i + 1;           /* filho da esquerda */
        if (filho >= n) break;              /* sem filhos: terminou */
        if (filho + 1 < n && v[filho + 1] > v[filho])
            filho++;                        /* escolhe o maior dos dois filhos */
        if (v[filho] <= x) break;           /* ja esta no lugar certo */
        v[i] = v[filho];                    /* filho sobe */
        i = filho;                          /* x continua descendo */
    }
    v[i] = x;
}

size_t embarca_dedup(uint16_t *v, size_t n)
{
    /* Borda: vazio ou 1 elemento (v pode ser NULL se n = 0). */
    if (n < 2) return n;

    /* 1a: monta o heap maximo (nos n/2-1..0; o resto sao folhas): O(n).
     * "i-- > 0" evita underflow do size_t. */
    for (size_t i = n / 2; i-- > 0; )
        peneirar(v, i, n);

    /* 1b: leva o maior para o fim, n-1 vezes: O(n log n). Invariante: v[fim+1..n) ordenado. */
    for (size_t fim = n - 1; fim > 0; fim--) {
        uint16_t maior = v[0];
        v[0] = v[fim];
        v[fim] = maior;
        peneirar(v, 0, fim);                /* conserta o heap sem o maior */
    }

    /* 2/3: varredura Theta(n). Invariante: v[0..k] = unicos ja vistos e k <= i
     * (nada e sobrescrito antes de ser lido). */
    size_t k = 0;
    for (size_t i = 1; i < n; i++)
        if (v[i] != v[k])
            v[++k] = v[i];

    return k + 1;                           /* quantidade de valores unicos */
}
