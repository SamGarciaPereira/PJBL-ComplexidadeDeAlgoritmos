# Análise teórica das dez estratégias

PjBL: Duas Empresas, Duas Restrições – Deduplicação de Leituras de Sensores

## 0. Notação e réguas

- **n**: leituras na entrada (10³, 10⁵, 10⁷). **u** = 2¹⁶ = 65 536 valores possíveis. **d**: valores distintos presentes (d ≤ min(n, u)). **log** = log₂.
- **Espaço extra** = pilha (variáveis locais e recursão) + heap (malloc/calloc), sem contar o vetor de entrada.
- **Embarca:** espaço extra ≤ **256 B** para qualquer n. **Datacenter:** n = 10⁷ em **< 2 s**, memória livre.

## 1. Resumo

| # | Estratégia | Tempo (melhor / médio / pior) | Pilha | Heap | Embarca (≤ 256 B) | Datacenter (< 2 s) |
|---|---|---|---|---|---|---|
| 1 | Força bruta | n / n² / n² | O(1) | 0 | aceita (muito lenta) | rejeitada |
| 2 | Ordenação + varredura (heapsort) | n log n nos três | O(1) | 0 | **aceita (implementada)** | borda: 1,95 s |
| 3 | Tabela hash | n / n / n² | O(1) | O(d) | rejeitada | aceita (sem garantia de pior caso) |
| 4 | AVL / Rubro-Negra | n / n log d / n log d | O(log d) | O(d) | rejeitada | borda (0,29 a 1,95 s) |
| 5 | Bitmap sobre o domínio | n nos três | O(1) | 8 192 B | rejeitada | **aceita (implementada)** |
| 6 | Filtro de Bloom | n·k nos três | O(1) | O(m) | rejeitada | aceita, perde dados |
| 7 | Bogosort + varredura | n / n·n! / ilimitado | O(1) | 0 | rejeitada | rejeitada |
| 8 | Power set recursivo | 2ⁿ nos três | O(n) | O(2ⁿ) | rejeitada | rejeitada |
| 9 | Codificação unária em disco | Σ valores | O(1) | 0 (disco: Σ bits) | viola a premissa | rejeitada |
| 10 | Matriz n×n de auditoria | n² nos três | O(1) | O(n²) | rejeitada | rejeitada |

## 2. Análise de cada estratégia

### 1. Força bruta

Cada leitura é comparada com as únicas já guardadas; se não houver igual, é guardada.

- **Tempo.** A i-ésima leitura faz até i − 1 comparações. Pior caso (todas distintas): 0 + 1 + … + (n − 1) = n(n − 1)/2 → **Θ(n²)**. Melhor caso (todas iguais): 1 comparação por leitura → **Θ(n)**. Como u = 65 536, a lista de únicas nunca passa disso, então o custo real é O(n · min(d, u)).
- **Espaço.** Pilha O(1) (índices). Heap 0 (as únicas são compactadas no próprio vetor).
- **Embarca:** cabe (≈ 70 B), mas para n = 10⁷ leva de ≈ 404 s (com o teto u) a 17,1 h (n²/2). **Datacenter:** rejeitada; mesmo com o teto, 404 s é **202 vezes** o limite de 2 s.

### 2. Ordenação + varredura de vizinhos

Ordena; iguais ficam lado a lado; uma varredura copia o primeiro de cada grupo.

- **Tempo.** A varredura é Θ(n); o que decide é a ordenação:

| Ordenação | Tempo (melhor / médio / pior) | Pilha | Heap |
|---|---|---|---|
| Heapsort (in-place) | n log n nos três | O(1) | 0 |
| Mergesort | n log n nos três | O(log n) | O(n) |
| Quicksort | n log n / n log n / n² | O(log n) médio, O(n) pior | 0 |

  Heapsort: montar o heap custa O(n) (a soma das alturas dos nós converge para 2n) e as n retiradas do máximo custam O(log n) cada → **Θ(n log n)** sempre.
- **Espaço.** Heapsort: só variáveis locais, **71 B medidos**. Mergesort: vetor auxiliar de 2n B = 20 MB em n = 10⁷. Quicksort: ≈ 23 níveis × ≥ 32 B ≈ 740 B.
- **Embarca:** aceita **só com heapsort** (mergesort e quicksort estouram 256 B). **Datacenter:** borda. Medido 1,95 s (mediana) para 10⁷, e uma das 5 rodadas passou de 2 s (2,08 s): não é seguro.

### 3. Tabela hash / conjunto

- **Tempo.** Consulta e inserção O(1) em média → melhor/médio **Θ(n)**. Pior caso (todas as chaves na mesma posição): cada operação custa O(d) → **O(n²)**.
- **Espaço.** Pilha O(1) (endereçamento aberto). Heap O(d): fator de carga 0,5 e 4 B por posição = **8 B por distinto** (teto de ≈ 512 KB, pois d ≤ 65 536).
- **Embarca:** rejeitada; com n = 10³ (990 distintos) são ≈ 8 KB, **31 vezes** o limite. **Datacenter:** aceita (≈ 0,05 s estimado), mas sem garantia de pior caso.

### 4. Árvore balanceada (AVL / Rubro-Negra)

- **Tempo.** Altura ≤ 1,44 log d, então busca e inserção custam O(log d) **garantido**: pior e médio **Θ(n log d)**; melhor (todas iguais, 1 nó) **Θ(n)**.
- **Espaço.** Heap O(d), ≈ **24 B por nó** (2 B de leitura, 2 ponteiros de 8 B, altura/cor, alinhamento). Pilha O(log d) se recursiva (≈ 23 níveis), O(1) se iterativa.
- **Embarca:** rejeitada (≈ 24 KB com 1 000 distintos, **94 vezes** o limite). **Datacenter:** estimativa otimista 0,29 s (1 passo simples por nível) e conservadora 1,95 s (constante do heapsort); a conservadora encosta nos 2 s.

### 5. Bitmap sobre o domínio conhecido

Um bit por valor possível: 65 536 bits = **8 192 B**, para qualquer n.

- **Tempo.** Um teste de bit e, se novo, uma marcação por leitura → **Θ(n)** nos três casos.
- **Espaço.** Heap 8 192 B fixos (O(1) em n). Pilha O(1). Medido: 8 192 B de heap + 287 B de pilha (quase tudo é a própria `calloc`) = 8 479 B.
- **Embarca:** rejeitada (8 192 B = **32 vezes** o limite). **Datacenter:** **aceita, a mais rápida**: 12,3 ms para 10⁷, **162 vezes** abaixo de 2 s.

### 6. Filtro de Bloom

Vetor de m bits e k hashes por leitura; nunca deixa passar duplicata, mas pode descartar por engano uma leitura nova (falso positivo).

- **Tempo.** k constante → **Θ(n·k) = Θ(n)**. Para falso positivo ≤ 1%: k = log₂ 100 ≈ 6,6 → **k = 7**.
- **Espaço.** Heap m bits, ≈ **9,6 bits por distinto** (1,2 B). Pilha O(1). Para 65 536 distintos ≈ 77 KiB.
- **Embarca:** rejeitada. Com 256 B (2 048 bits) e 1 000 distintos o falso positivo seria ≥ 38% com o melhor k (exigido ≤ 1%); para 1% seriam ≈ 1,2 KB. **Datacenter:** aceita com falso positivo ≤ 1% declarado, porém é dominada pelo bitmap (77 KiB contra 8 KiB, 7 acessos por leitura contra 1, e ainda perde dados).

### 7. Bogosort + varredura (absurda)

- **Tempo.** Cada tentativa custa O(n) (embaralhar e conferir). Com n valores distintos há n! ordens e uma só correta → **n! tentativas em média** → **O(n · n!)**. Melhor caso O(n) (já ordenado); pior caso sem limite.
- **Espaço.** Pilha O(1), heap 0.
- **Cálculo** (1 passo = 1,23 ns, constante medida no bitmap, Seção 5):

| n | n! | tempo esperado (n · n! passos) |
|---|---|---|
| 10 | 3,6 × 10⁶ | 0,045 s |
| 12 | 4,8 × 10⁸ | **7,1 s** (n = 11: 0,54 s) |
| 20 | 2,4 × 10¹⁸ | 6,0 × 10¹⁰ s ≈ **1 902 anos** |
| 1 000 | ≈ 4 × 10²⁵⁶⁷ | ≈ 10²⁵⁶² s, ≈ 10²⁵⁴⁴ vezes a idade do universo (4,35 × 10¹⁷ s) |

- **Datacenter:** viola os 2 s já em n = 12 (7,1 s). **Embarca:** sem limite numérico de tempo, mas com n = 20 passam de mil anos. Rejeitada para ambos.

### 8. Enumeração recursiva de subconjuntos / power set (absurda)

- **Tempo:** **Θ(2ⁿ)** (todos os subconjuntos são visitados). **Espaço:** heap O(2ⁿ) se guardados; pilha O(n) pela profundidade da recursão.
- **Cálculo:**
  - Embarca: n = 9 já dá 2⁹ = 512 subconjuntos, mais que 256 B (mesmo com 1 B cada).
  - Datacenter, memória: 2³⁴ B > 8 GiB, já em n = 34.
  - Datacenter, tempo: n = 40 → 2⁴⁰ ≈ 1,1 × 10¹² passos ≈ 1 356 s. Para n = 10³: 2¹⁰⁰⁰ ≈ 10³⁰¹ subconjuntos (o universo observável tem ≈ 10⁸⁰ átomos).
  - Pilha: profundidade n com 32 B por quadro e pilha de 8 MiB (Linux) estoura em n = 8 MiB / 32 B = **262 144**; n = 10⁵ ainda cabe, n = 10⁷ daria **stack overflow**.
- Rejeitada para ambos.

### 9. Codificação unária em disco (absurda)

Cada leitura x vira uma sequência de x bits no disco; a RAM fica quase O(1) e o custo vai para o disco.

- **Tempo:** **O(Σ valores)**. **Espaço:** RAM O(1); disco Σ valores bits (cresce com o valor, exponencial no número de bits da leitura).
- **Cálculo** (leituras uniformes, média 32 767,5; 1 passo = 1,23 ns, só os bits):

| n | Σ valores | disco | tempo |
|---|---|---|---|
| 10³ | 3,3 × 10⁷ bits | ≈ 4,1 MB | 0,04 s |
| 10⁵ | 3,3 × 10⁹ bits | ≈ 410 MB | 4,0 s |
| 10⁷ | 3,3 × 10¹¹ bits | ≈ **41 GB** | **404 s** (sem contar latência de disco) |

- **Embarca:** a RAM fica em ≈ 70 B, mas o custo foi transferido para o disco (41 GB), que quebra a premissa de que "memória" é só RAM. **Datacenter:** 404 s ≫ 2 s. Rejeitada para ambos.

### 10. Matriz de adjacência n×n de auditoria (absurda)

Guarda o resultado de todas as comparações entre pares, sem que o problema peça rastreabilidade.

- **Tempo:** **Θ(n²)**. **Espaço:** heap **Θ(n²)** células (1 bit por célula no melhor caso: n²/8 B).
- **Cálculo** (1 bit por célula; 1 passo = 1,23 ns):

| n | células | memória | tempo |
|---|---|---|---|
| 10³ | 10⁶ | 125 KB | 1,2 ms |
| 10⁵ | 10¹⁰ | 1,25 GB | 12,3 s |
| 10⁷ | 10¹⁴ | **12,5 TB** | 123 356 s ≈ **34,3 h** |

- **Embarca:** 125 KB já em n = 10³ (**488 vezes** o limite). **Datacenter:** sem limite de memória não é memória infinita; 12,5 TB e 34,3 h contra 2 s. Rejeitada para ambos.

### Cenários em que as absurdas deixariam de ser absurdas

| # | Cenário hipotético | Por quê |
|---|---|---|
| 7 | n ≤ ≈ 10 leituras por lote | 10! = 3,6 milhões de tentativas ≈ 0,045 s |
| 8 | n ≤ ≈ 20 e o problema exigir enumerar subconjuntos | 2²⁰ ≈ 10⁶ subconjuntos ≈ 1 ms |
| 9 | Domínio minúsculo (ex.: 0–100) e disco barato | Σ valores ≤ 100 · n, vira uma contagem simples |
| 10 | n ≤ ≈ 10⁴ e auditoria exigida por contrato | 10⁸ células = 12,5 MB, ≈ 0,12 s |

## 3. Justificativa da escolha: heapsort (Embarca) e bitmap (Datacenter)

1. **Extremos opostos do compromisso tempo × espaço.** Heapsort: espaço mínimo (71 B), tempo n log n (1,95 s para 10⁷). Bitmap: espaço fixo de 8 KiB, tempo n (12,3 ms para 10⁷).
2. **Cada uma cumpre a régua do seu cliente e quebra a do outro.** O bitmap tem 8 192 B, 32 vezes o limite da Embarca. O heapsort fica na borda dos 2 s (uma rodada em 5 passou) e é 158 vezes mais lento que o bitmap.
3. **Alternativas descartadas.**
   - Embarca: só as estratégias 1 e 2 cabem em 256 B. A força bruta leva ≥ 404 s para 10⁷ contra 1,95 s do heapsort. Mergesort (vetor auxiliar de 20 MB) e quicksort (pilha ≈ 740 B) estouram a memória. Hash, AVL, Bloom e bitmap precisam de KB.
   - Datacenter: hash, AVL e Bloom também caberiam no tempo, mas o hash não tem garantia de pior caso (O(n²)), o AVL encosta nos 2 s na estimativa conservadora e o Bloom perde dados. O bitmap é exato, determinístico e o mais rápido.
4. Linguagem: C, que o enunciado (Seção 3) recomenda para a Embarca e a Seção 6 permite para as duas.

## 4. Medições e extrapolação

### 4.1 Como foi medido

Máquina de medição: Linux em VM, Xeon 2,1 GHz, 2 vCPU, GCC 13.3 `-O2` (a máquina de referência da disciplina deve repetir a medição com `./main dados/resultados_empiricos.csv`). Dados: leituras pseudoaleatórias (xorshift32, semente fixa). Cada ponto: 5 rodadas, mediana do tempo, cópia nova do vetor a cada rodada; tempo e memória em rodadas separadas. O vetor de entrada não conta como espaço extra. Heap: alocador contador (`inst_calloc`). Pilha: pinta 4 096 B abaixo da função com 0xAA e mede até onde foram sobrescritos (limite superior).

| n | Heapsort: mediana (mín–máx) | Heapsort: extra | Bitmap: mediana (mín–máx) | Bitmap: extra | distintos |
|---|---|---|---|---|---|
| 10³ | 52,7 µs (50,3–65,9) | 71 B | 1,74 µs (1,58–10,1) | 8 479 B | 990 |
| 10⁵ | 12,08 ms (9,92–14,81) | 71 B | 0,428 ms (0,423–0,444) | 8 479 B | 51 340 |
| 10⁷ | 1,951 s (1,937–2,079) | 71 B | 12,34 ms (11,99–12,66) | 8 479 B | 65 536 |

Evidências exigidas (Seção 8): Embarca usa 71 B ≤ 256 B em todos os n; Datacenter processa 10⁷ em 12,3 ms (pior rodada 12,7 ms) < 2 s; as duas acham as mesmas quantidades de distintos.

### 4.2 Modelo de extrapolação das oito estratégias não implementadas

Passos teóricos × uma constante medida:

- **K₁ = 1,23 ns por passo simples** = 12,34 ms ÷ 10⁷ (bitmap, n = 10⁷).
- **K₂ = 8,39 ns por unidade de n·log₂n** = 1,951 s ÷ (10⁷ × 23,25) (heapsort, n = 10⁷), para a família n log n.

| # | Tempo | Espaço extra |
|---|---|---|
| 1 Força bruta | K₁ · n²/2 | ≈ 71 B |
| 3 Hash | 4 · K₁ · n (hash, acesso, comparação, inserção) | 8 · n B |
| 4 AVL | K₂ · n · log₂n | 24 · n B |
| 6 Bloom (1%) | 7 · K₁ · n (k = 7) | 1,2 · n B |
| 7 Bogosort | K₁ · n · n! | ≈ 71 B |
| 8 Power set | K₁ · 2ⁿ | 2ⁿ B (1 B por subconjunto) |
| 9 Unária | K₁ · 32 767,5 · n | RAM ≈ 71 B; disco 32 767,5 · n / 8 B |
| 10 Matriz | K₁ · n² | n²/8 B |

| # | n = 10³ | n = 10⁵ | n = 10⁷ |
|---|---|---|---|
| 1 tempo | 0,6 ms | 6,2 s | 61 678 s (17,1 h) |
| 3 tempo / espaço | 4,9 µs / 8 KB | 0,49 ms / 800 KB | 49 ms / 80 MB |
| 4 tempo / espaço | 84 µs / 24 KB | 13,9 ms / 2,4 MB | 1,95 s / 240 MB |
| 6 tempo / espaço | 8,6 µs / 1,2 KB | 0,86 ms / 120 KB | 86 ms / 12 MB |
| 7 tempo | ≈ 10²⁵⁶² s | fora de escala | fora de escala |
| 8 tempo / espaço | ≈ 10²⁹² s / 10³⁰¹ B | fora de escala | fora de escala |
| 9 tempo / disco | 0,04 s / 4,1 MB | 4,0 s / 410 MB | 404 s / 41 GB |
| 10 tempo / espaço | 1,2 ms / 125 KB | 12,3 s / 1,25 GB | 34,3 h / 12,5 TB |

As curvas completas estão nos gráficos do `relatorio.pdf` (tempo e espaço, 2 medidas + 8 extrapoladas no mesmo gráfico).

### 4.3 Teoria × prática e divergências

| n | Bitmap medido | previsto (K₁·n) | razão | Heapsort medido | previsto (K₂·n·log n) | razão |
|---|---|---|---|---|---|---|
| 10³ | 1,74 µs | 1,23 µs | 1,41 | 52,7 µs | 83,6 µs | 0,63 |
| 10⁵ | 428 µs | 123 µs | **3,47** | 12,08 ms | 13,94 ms | 0,87 |
| 10⁷ | 12,34 ms | 12,34 ms | 1,00 (calibração) | 1,951 s | 1,951 s | 1,00 (calibração) |

Os dois crescem na ordem de grandeza prevista (heapsort ×161 de 10⁵ para 10⁷, previsto ×140).

1. **Bitmap em n = 10⁵ está 3,5× acima da reta.** Hipótese (não verificada com profiler): com ≈ 51% de leituras novas o `if` fica imprevisível e o processador erra o desvio com frequência; em 10³ (quase tudo novo) e 10⁷ (quase tudo repetido) o desvio é previsível. Em 10³ o ruído fixo (`calloc`, relógio) também pesa. A ordem de grandeza continua Θ(n).
2. **Heapsort 6,8× mais lento que K₁ · n log n.** Os acessos v[2i+1] saltam longe no vetor; em 10⁷ são 20 MB, maiores que o cache, então há muitas faltas de cache. Por isso a família n log n usa K₂ e não K₁. O bitmap só mexe em 8 KB, que cabem no cache.
3. **Força bruta: 61 678 s na teoria geral × ≈ 404 s com u = 65 536.** O gráfico usa a teoria do catálogo (n²/2); o teto do domínio explica a diferença de ≈ 150×.
4. **Hash, AVL e Bloom: O(n) na teoria, mas com platô na prática.** Como d ≤ 65 536 (hash ≤ 512 KB, AVL ≤ 1,5 MB, Bloom ≤ 77 KiB). O gráfico segue o catálogo; nenhum veredito muda, pois todos continuam muito acima de 256 B.
5. **AVL coincide com o heapsort no gráfico** porque usa a mesma constante K₂ por construção. A verdade está entre a estimativa otimista (0,29 s) e a conservadora (1,95 s).
6. **Ambiente (cache, SO, relógio):** ruído relevante em n = 10³ (microssegundos); por isso a mediana de 5 rodadas. Em n = 10⁷ o heapsort variou de 1,94 s a 2,08 s entre rodadas.
