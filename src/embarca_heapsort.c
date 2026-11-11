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
    for (;;) {                              /* repete ate achar o lugar de x */
        size_t filho = 2 * i + 1;           /* filho da esquerda */
        if (filho >= n) break;              /* sem filhos: terminou */
        if (filho + 1 < n && v[filho + 1] > v[filho])   /* filho da direita existe e e maior? */
            filho++;                        /* escolhe o maior dos dois filhos */
        if (v[filho] <= x) break;           /* ja esta no lugar certo */
        v[i] = v[filho];                    /* filho sobe */
        i = filho;                          /* x continua descendo */
    }
    v[i] = x;                               /* grava x na posicao final */
}

size_t embarca_dedup(uint16_t *v, size_t n)
{
    /* Borda: vazio ou 1 elemento (v pode ser NULL se n = 0). */
    if (n < 2) return n;                    /* nada a fazer: ja sem duplicatas */

    /* 1a: monta o heap maximo (nos n/2-1..0; o resto sao folhas): O(n).
     * "i-- > 0" evita underflow do size_t. */
    for (size_t i = n / 2; i-- > 0; )       /* i = n/2-1, ..., 0 */
        peneirar(v, i, n);                  /* desce v[i] ate o lugar certo */

    /* 1b: leva o maior para o fim, n-1 vezes: O(n log n). Invariante: v[fim+1..n) ordenado. */
    for (size_t fim = n - 1; fim > 0; fim--) {      /* fim = ultima posicao do heap */
        uint16_t maior = v[0];              /* o maior do heap esta na raiz */
        v[0] = v[fim];                      /* ultimo elemento vai para a raiz */
        v[fim] = maior;                     /* maior fica na posicao final */
        peneirar(v, 0, fim);                /* conserta o heap sem o maior */
    }

    /* 2/3: varredura Theta(n). Invariante: v[0..k] = unicos ja vistos e k <= i
     * (nada e sobrescrito antes de ser lido). */
    size_t k = 0;                           /* posicao do ultimo unico guardado */
    for (size_t i = 1; i < n; i++)          /* v[0] ja e o primeiro unico */
        if (v[i] != v[k])                   /* diferente do ultimo unico? */
            v[++k] = v[i];                  /* novo: avanca k e guarda */

    return k + 1;                           /* quantidade de valores unicos */
}
