/*
 * Estrategia 5: bitmap sobre o dominio conhecido (0..65535). Cliente: DATACENTER.
 *
 * 1 bit por valor possivel (65536 bits = 8192 B): bit 0 = valor novo (marca e guarda),
 * bit 1 = repetido (ignora). Uma passada; mantem a ordem de chegada. Resultado exato.
 * Custo: tempo Theta(n) sempre, heap 8192 B fixos, pilha O(1).
 * So serve porque o dominio e pequeno (32 bits seriam 512 MB); nao cabe na Embarca (256 B).
 */
#include "datacenter_bitmap.h"
#include "instrumentacao.h"

#define VALORES_POSSIVEIS 65536u                 /* 2^16 */
#define BYTES_DO_BITMAP   (VALORES_POSSIVEIS / 8u) /* 8192 */

size_t datacenter_dedup(uint16_t *v, size_t n)
{
    /* inst_calloc: conta o heap na instrumentacao. */
    uint8_t *visto = inst_calloc(BYTES_DO_BITMAP);   /* tudo zerado */

    /* Invariante: v[0..k) = unicos ja vistos e k <= i (gravar em v[k] e seguro). */
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
