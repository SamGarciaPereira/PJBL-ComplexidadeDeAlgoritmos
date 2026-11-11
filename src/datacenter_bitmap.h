#ifndef DATACENTER_BITMAP_H
#define DATACENTER_BITMAP_H

#include <stddef.h>
#include <stdint.h>

/* Remove duplicatas de v[0..n) no proprio vetor, mantendo a ordem de chegada.
 * Devolve a quantidade de valores unicos. Usa 8192 bytes de heap (bitmap). */
size_t datacenter_dedup(uint16_t *v, size_t n);

#endif
