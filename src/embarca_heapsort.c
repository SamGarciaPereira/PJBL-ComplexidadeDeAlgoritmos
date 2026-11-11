/*
 * Estrategia 2 do catalogo: ordenacao + varredura de vizinhos
 * Escolhida para a EMBARCA (limite: 256 bytes de memoria extra).
 *
 * Ideia em 3 linhas:
 *   1. Ordena o vetor NO PROPRIO LUGAR com heapsort (sem vetor auxiliar).
 *   2. Depois de ordenado, valores iguais ficam lado a lado.
 *   3. Varre o vetor uma vez e copia so o primeiro de cada grupo para o inicio.
 *
 * Por que heapsort e nao quicksort/mergesort?
 *   - mergesort precisa de vetor auxiliar O(n)      -> estoura os 256 bytes
 *   - quicksort e recursivo: pilha O(log n) no medio, O(n) no pior caso
 *   - heapsort e iterativo, usa so algumas variaveis: espaco O(1) e
 *     tempo O(n log n) garantido
 *
 * Memoria extra: so as variaveis locais (indices e uma leitura temporaria).
 * Nada de malloc: heap = 0 bytes.
 *
 * Complexidade: tempo Theta(n log n) sempre (sem pior caso quadratico);
 * pilha O(1) (sem recursao); heap 0 B. Atende a Embarca (<= 256 B para todo n).
 * Preco: mais lento que o bitmap; nao estavel (irrelevante: iguais sao identicos).
 */
#include "embarca_heapsort.h"

/* "Peneira" v[i] para baixo ate o lugar certo no heap maximo v[0..n).
 * Filhos de i: 2i+1 e 2i+2. Custo O(log n). Iterativa de proposito:
 * recursao gastaria pilha e arriscaria os 256 B. */
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
    /* Borda (Secao 6): vazio ou 1 elemento. Nao toca em v (pode ser NULL se n = 0). */
    if (n < 2) return n;

    /* Passo 1a: monta o heap maximo de baixo para cima (de n/2-1 a 0; o resto sao
     * folhas). "i-- > 0" evita o underflow do size_t. Custo O(n), nao n log n. */
    for (size_t i = n / 2; i-- > 0; )
        peneirar(v, i, n);

    /* Passo 1b: leva o maior (v[0]) para o fim, n-1 vezes: O(n log n).
     * Invariante: v[fim+1..n) ja esta ordenado e e >= v[0..fim]. */
    for (size_t fim = n - 1; fim > 0; fim--) {
        uint16_t maior = v[0];
        v[0] = v[fim];
        v[fim] = maior;
        peneirar(v, 0, fim);                /* conserta o heap sem o maior */
    }

    /* Passo 2/3: varredura, Theta(n). k = ultimo unico guardado. Invariante:
     * v[0..k] sao os unicos ja vistos e k <= i, entao v[++k] nunca apaga o que
     * falta ler. Depois de k+1 sobra lixo (so a quantidade e garantida). */
    size_t k = 0;
    for (size_t i = 1; i < n; i++)
        if (v[i] != v[k])
            v[++k] = v[i];

    return k + 1;                           /* quantidade de valores unicos */
}
