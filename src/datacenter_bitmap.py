"""
Estrategia 5 do catalogo: bitmap (vetor de bits) sobre o dominio conhecido.
Versao em Python + NumPy para o DATACENTER (limite: n = 10^7 em menos de 2 segundos).
A Embarca continua em C (src/embarca_heapsort.c), como o enunciado recomenda.

Ideia em 3 linhas:
  1. Cada leitura vale de 0 a 65535: existem so 65536 valores possiveis.
  2. Uma tabela com uma casa por valor possivel diz "este valor ja apareceu?".
     Marcar todas as leituras de uma vez (visto[v] = True) faz o trabalho do laco.
  3. Os valores marcados sao os unicos.

Tempo: O(n) (a marcacao roda em C dentro do NumPy). Espaco extra: tabela de 65536
casas, independente de n. Diferenca para a versao em C: a saida vem em ORDEM
CRESCENTE (o enunciado nao exige ordem de chegada).

Requisito:  pip install numpy
Rodar:      python src/datacenter_bitmap.py
"""
import statistics
import sys
import time
import tracemalloc

import numpy as np

LIMITE_SEGUNDOS = 2.0
REPETICOES = 5


def datacenter_dedup(v):
    """Recebe um array NumPy de leituras (0..65535) e devolve os valores unicos, crescentes."""
    visto = np.zeros(65536, dtype=bool)   # uma casa por valor possivel, tudo falso
    visto[v] = True                       # marca todas as leituras (repetidas: inofensivo)
    return np.flatnonzero(visto)          # indices marcados = unicos, em ordem crescente


def testes_corretude():
    casos = [
        [5, 3, 5, 1, 3],                  # com duplicatas
        [9, 2, 7],                        # sem duplicatas
        [4] * 10,                         # todas iguais
        [65535, 0, 65535, 0],             # extremos da faixa
        [42],                             # um elemento
        [],                               # vazio
    ]
    ok = True
    for entrada in casos:
        v = np.array(entrada, dtype=np.uint16)
        ok = ok and datacenter_dedup(v).tolist() == sorted(set(entrada))
    print("Corretude (n <= 10, com/sem duplicatas, vazio) ... " + ("OK" if ok else "FALHOU"))
    return ok


def main():
    ok = testes_corretude()

    print(f"\n{'n':>10} {'tempo_med_s':>12} {'tempo_max_s':>12} {'heap_B':>8} {'unicos':>7}")
    rng = np.random.default_rng(1)
    for n in (10**3, 10**5, 10**7):
        # Semente fixa = dados reprodutiveis. A funcao nao altera "base": sem copia por rodada.
        base = rng.integers(0, 65536, n, dtype=np.uint16)
        tempos = []
        for _ in range(REPETICOES):
            t0 = time.perf_counter()
            unicos = datacenter_dedup(base)
            tempos.append(time.perf_counter() - t0)

        tracemalloc.start()                              # memoria medida em rodada separada
        datacenter_dedup(base)
        _, pico = tracemalloc.get_traced_memory()
        tracemalloc.stop()

        if len(unicos) != len(set(base.tolist())):
            ok = False
        print(f"{n:>10} {statistics.median(tempos):>12.6f} {max(tempos):>12.6f} "
              f"{pico:>8} {len(unicos):>7}")
        if n == 10**7 and max(tempos) >= LIMITE_SEGUNDOS:
            ok = False

    print(f"\nDatacenter n = 10^7 em < {LIMITE_SEGUNDOS:.0f} s (pior das {REPETICOES} rodadas) ... "
          + ("OK" if ok else "FALHOU"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
