#ifndef EMBARCA_HEAPSORT_H
#define EMBARCA_HEAPSORT_H

#include <stddef.h>
#include <stdint.h>

/* Remove duplicatas de v[0..n) no proprio vetor (resultado ORDENADO).
 * Devolve a quantidade de valores unicos. Usa O(1) de memoria extra. */
size_t embarca_dedup(uint16_t *v, size_t n);

#endif
