/*
 * instrumentacao.h
 * Ferramentas para MEDIR (sem profiler pronto) o que cada estrategia gasta:
 *   - tempo de execucao
 *   - bytes alocados no heap (pico)
 *   - bytes usados na pilha (pico)
 */
#ifndef INSTRUMENTACAO_H
#define INSTRUMENTACAO_H

#include <stddef.h>
#include <stdint.h>

/* Toda estrategia de deduplicacao tem esta assinatura:
 * recebe o vetor v com n leituras, deixa as leituras unicas no inicio
 * do proprio vetor e devolve quantas sobraram. */
typedef size_t (*FuncaoDedup)(uint16_t *v, size_t n);

/* Alocacao com contador: as estrategias usam estas funcoes em vez de
 * calloc/free, assim sabemos exatamente quantos bytes pediram ao heap. */
void  *inst_calloc(size_t bytes);
void   inst_free(void *p, size_t bytes);

/* Tempo (segundos) de uma chamada a f. Guarda em *n_unicos o resultado. */
double inst_medir_tempo(FuncaoDedup f, uint16_t *v, size_t n, size_t *n_unicos);

/* Memoria extra de uma chamada a f. Devolve o pico de PILHA (bytes) e
 * guarda em *heap_pico o pico de HEAP (bytes). */
size_t inst_medir_memoria(FuncaoDedup f, uint16_t *v, size_t n, size_t *heap_pico);

#endif
