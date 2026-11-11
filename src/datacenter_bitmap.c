/*
 * Estrategia 5 do catalogo: bitmap (vetor de bits) sobre o dominio conhecido
 * Escolhida para o DATACENTER (limite: n = 10^7 em menos de 2 segundos).
 *
 * Ideia em 3 linhas:
 *   1. Cada leitura vale de 0 a 65535, entao existem so 65536 valores possiveis.
 *   2. Guardamos 1 bit por valor possivel: "este valor ja apareceu?"
 *      65536 bits = 8192 bytes, qualquer que seja n.
 *   3. Percorremos o vetor uma unica vez: se o bit do valor e 0, ele e novo
 *      (marca o bit e guarda o valor); se e 1, e repetido (ignora).
 *
 * Tempo: O(n), cada leitura custa um teste de bit (tempo constante).
 * Espaco extra: 8192 bytes fixos (O(1) em relacao a n) - cabe no Datacenter,
 * que nao tem limite de memoria, e NAO caberia na Embarca (limite 256 bytes).
 * O resultado mantem a ordem de chegada das leituras.
 *
 * Complexidade: tempo Theta(n) sempre; heap 8192 B fixos; pilha O(1).
 * So funciona porque o dominio (16 bits) e pequeno: com 32 bits seriam 512 MB.
 * Atende o Datacenter (10^7 em < 2 s) e e EXATO, ao contrario do Bloom.
 */
#include "datacenter_bitmap.h"
#include "instrumentacao.h"

#define VALORES_POSSIVEIS 65536u                 /* 2^16 */
#define BYTES_DO_BITMAP   (VALORES_POSSIVEIS / 8u) /* 8192 */

size_t datacenter_dedup(uint16_t *v, size_t n)
{
    /* inst_calloc (nao calloc): assim o heap entra na instrumentacao. */
    uint8_t *visto = inst_calloc(BYTES_DO_BITMAP);   /* tudo zerado */

    /* Invariante: v[0..k) = unicos ja vistos (ordem de chegada) e k <= i,
     * entao gravar em v[k] e seguro e dispensa vetor de saida. */
    size_t k = 0;                                    /* quantos unicos ate agora */
    for (size_t i = 0; i < n; i++) {
        uint16_t x    = v[i];
        uint8_t  mask = (uint8_t)(1u << (x & 7u));   /* qual bit dentro do byte */
        size_t   byte = x >> 3;                      /* qual byte */

        if (!(visto[byte] & mask)) {                 /* primeira vez? */
            visto[byte] |= mask;                     /* marca como visto */
            v[k++] = x;                              /* guarda no inicio do vetor */
        }
    }

    inst_free(visto, BYTES_DO_BITMAP);
    return k;
}
