# Análise teórica das dez estratégias

PjBL: Duas Empresas, Duas Restrições – Deduplicação de Leituras de Sensores

## 0. Notação e réguas

- **n**: leituras na entrada (10³, 10⁵, 10⁷). **u** = 2¹⁶ = 65 536 valores possíveis. **d**: valores distintos presentes (d ≤ min(n, u)). **log** = log₂.
- **Espaço extra** = pilha (variáveis locais e recursão) + heap (malloc/calloc), sem contar o vetor de entrada.
- **Embarca:** espaço extra ≤ **256 B** para qualquer n. **Datacenter:** n = 10⁷ em **< 2 s**, memória livre.

## 1. Resumo

| # | Estratégia | Tempo (melhor / médio / pior) | Pilha | Heap | Embarca (≤ 256 B) | Datacenter (< 2 s) |
|---|---|---|---|---|---|---|
| 1 | Força bruta | n / n·d/2 / n² | O(1) | 0 | aceita (muito lenta) | rejeitada |
| 2 | Ordenação + varredura (heapsort) | n log n nos três | O(1) | 0 | **aceita (implementada)** | borda: 1,94 s |
| 3 | Tabela hash | n / n / n² | O(1) | O(d) | rejeitada | aceita (sem garantia de pior caso) |
| 4 | AVL / Rubro-Negra | n / n log d / n log d | O(log d) | O(d) | rejeitada | borda (228 ms a 1,94 s) |
| 5 | Bitmap sobre o domínio | n nos três | O(1) | 8 192 B | rejeitada | **aceita (implementada)** |
| 6 | Filtro de Bloom | n·k nos três | O(1) | O(m) | rejeitada | aceita, perde dados |
| 7 | Bogosort + varredura | n / n·n! / ilimitado | O(1) | 0 | rejeitada | rejeitada |
| 8 | Power set recursivo | 2ⁿ nos três | O(n) | O(2ⁿ) | rejeitada | rejeitada |
| 9 | Codificação unária em disco | Σ valores | O(1) | 0 (disco: Σ bits) | viola a premissa | rejeitada |
| 10 | Matriz n×n de auditoria | n² nos três | O(1) | O(n²) | rejeitada | rejeitada |

## 2. Análise de cada estratégia

### 1. Força bruta

Cada leitura é comparada com as únicas já guardadas; se não houver igual, é guardada.

- **Tempo.** A i-ésima leitura faz até i − 1 comparações. Pior caso (todas distintas): 0 + 1 + … + (n − 1) = n(n − 1)/2 → **Θ(n²)**. Melhor caso (todas iguais): 1 comparação por leitura → **Θ(n)**. Caso médio com entrada uniforme: d chega a ≈ 65 536 depois de ≈ 7 × 10⁵ leituras (coletor de cupons) e cada repetida custa em média d/2 comparações → ≈ **n·d/2**. Como u = 65 536, a lista de únicas nunca passa disso: O(n · min(d, u)).
- **Espaço.** Pilha O(1) (só índices e contadores). Heap 0 (as únicas são compactadas no próprio vetor).
- **Embarca:** cabe (só índices, bem abaixo de 256 B), mas para n = 10⁷ leva **102 s medidos em C** (3,27 × 10¹¹ comparações a 0,31 ns). A extrapolação com a constante K₁ (§4.2) dá ≈ 321 s (com o teto u) e 13,6 h (n²/2). **Datacenter:** rejeitada; mesmo a medida de 102 s é **51 vezes** o limite de 2 s.

### 2. Ordenação + varredura de vizinhos

Ordena; iguais ficam lado a lado; uma varredura copia o primeiro de cada grupo.

- **Tempo.** A varredura é Θ(n); o que decide é a ordenação:

| Ordenação | Tempo (melhor / médio / pior) | Pilha | Heap |
|---|---|---|---|
| Heapsort (in-place) | n log n nos três | O(1) | 0 |
| Mergesort | n log n nos três | O(log n) | O(n) |
| Quicksort | n log n / n log n / n² | O(log n) médio, O(n) pior | 0 |

  Heapsort: montar o heap custa O(n) (a soma das alturas dos nós converge para 2n) e as n retiradas do máximo custam O(log n) cada → **Θ(n log n)** sempre.
- **Espaço.** Heapsort: só variáveis locais, **159 B de pilha medidos** (limite superior) e 0 B de heap. Mergesort: vetor auxiliar de 2n B = 20 MB em n = 10⁷. Quicksort: ≈ 23 níveis × ≥ 32 B ≈ 740 B.
- **Embarca:** aceita **só com heapsort** (mergesort e quicksort estouram 256 B). **Datacenter:** borda. Medido 1,94 s (mediana da execução registrada, pior rodada 1,96 s); nas 5 execuções fixadas em um núcleo P, **7 de 25 rodadas passaram de 2 s** (máximo 3,31 s): não é seguro.

### 3. Tabela hash / conjunto

- **Tempo.** Consulta e inserção O(1) em média → melhor/médio **Θ(n)**. Pior caso (todas as chaves na mesma posição): cada operação custa O(d) → **O(n·d) ≤ O(n²)**. O custo amortizado vem do redimensionamento: dobrar a tabela custa O(n), mas espalhado pelas inserções dá O(1) amortizado por inserção; isso não garante o pior caso de cada operação.
- **Espaço.** Pilha O(1) (endereçamento aberto). Heap O(d): fator de carga 0,5 e 4 B por posição = **8 B por distinto** (teto de ≈ 512 KB, pois d ≤ 65 536).
- **Embarca:** rejeitada; com n = 10³ (990 distintos) são ≈ 8 KB, **31 vezes** o limite. **Datacenter:** aceita (≈ 39,3 ms estimado), mas sem garantia de pior caso.

### 4. Árvore balanceada (AVL / Rubro-Negra)

- **Tempo.** Altura ≤ 1,44 log d, então busca e inserção custam O(log d) **garantido**: pior e médio **Θ(n log d)**; melhor (todas iguais, 1 nó) **Θ(n)**.
- **Espaço.** Heap O(d), ≈ **24 B por nó** (2 B de leitura, 2 ponteiros de 8 B, altura/cor, alinhamento). Pilha O(log d) se recursiva (≈ 23 níveis), O(1) se iterativa.
- **Embarca:** rejeitada (≈ 24 KB com 1 000 distintos, **94 vezes** o limite). **Datacenter:** estimativa otimista 228 ms (1 passo simples por nível) e conservadora 1,94 s (constante do heapsort); a conservadora encosta nos 2 s.

### 5. Bitmap sobre o domínio conhecido

Um bit por valor possível: 65 536 bits = **8 192 B**, para qualquer n.

- **Tempo.** Um teste de bit e, se novo, uma marcação por leitura → **Θ(n)** nos três casos.
- **Espaço.** Heap 8 192 B fixos (O(1) em n). Pilha O(1). Medido: 8 192 B de heap + 1 079 B de pilha (limite superior; inclui o quadro do `calloc` da biblioteca C do Windows) = 9 271 B.
- **Embarca:** rejeitada (8 192 B = **32 vezes** o limite). **Datacenter:** **aceita, a mais rápida**: 9,82 ms para 10⁷, **204 vezes** abaixo de 2 s.

### 6. Filtro de Bloom

Vetor de m bits e k hashes por leitura; nunca deixa passar duplicata, mas pode descartar por engano uma leitura nova (falso positivo).

- **Tempo.** k constante → **Θ(n·k) = Θ(n)**. Para falso positivo ≤ 1%: k = log₂ 100 ≈ 6,6 → **k = 7**.
- **Espaço.** Heap m bits, ≈ **9,6 bits por distinto** (1,2 B). Pilha O(1). Para 65 536 distintos ≈ 77 KiB. Fórmulas: p ≈ (1 − e^(−k·d/m))^k; m = −d·ln p / (ln 2)²; k = (m/d)·ln 2.
- **Embarca:** rejeitada. Com 256 B (2 048 bits) o falso positivo de 1% só vale até ≈ 213 distintos; com 1 000 distintos seria ≥ 38% com o melhor k (exigido ≤ 1%); para 1% seriam ≈ 1,2 KB. **Datacenter:** aceita com falso positivo ≤ 1% declarado, porém é dominada pelo bitmap (77 KiB contra 8 KiB, 7 acessos por leitura contra 1, e ainda perde dados).

### 7. Bogosort + varredura (absurda)

- **Tempo.** Cada tentativa custa O(n) (embaralhar e conferir). Com n valores distintos há n! ordens e uma só correta → **n! tentativas em média** → **O(n · n!)**. Com valores repetidos o número esperado de tentativas é n!/∏mᵢ! (mᵢ = quantas vezes o valor i aparece), ainda astronômico para n grande. Melhor caso O(n) (já ordenado); pior caso sem limite.
- **Espaço.** Pilha O(1), heap 0.
- **Cálculo** (1 passo = 0,98 ns, constante medida no bitmap, §4.2):

| n | n! | tempo esperado (n · n! passos) |
|---|---|---|
| 10 | 3,6 × 10⁶ | 35,6 ms |
| 12 | 4,8 × 10⁸ | **5,64 s** (n = 11: 431 ms) |
| 20 | 2,4 × 10¹⁸ | 4,78 × 10¹⁰ s ≈ **1 513 anos** |
| 1 000 | ≈ 4 × 10²⁵⁶⁷ | ≈ 10²⁵⁶² s, ≈ 10²⁵⁴⁴ vezes a idade do universo (4,35 × 10¹⁷ s) |

- **Datacenter:** viola os 2 s já em n = 12 (5,64 s). **Embarca:** sem limite numérico de tempo, mas com n = 20 passam de mil anos. Rejeitada para ambos.

### 8. Enumeração recursiva de subconjuntos / power set (absurda)

- **Tempo:** **Θ(2ⁿ)** (todos os subconjuntos são visitados). **Espaço:** heap O(2ⁿ) se guardados; pilha O(n) pela profundidade da recursão.
- **Cálculo:**
  - Embarca: n = 9 já dá 2⁹ = 512 subconjuntos, mais que 256 B (mesmo com 1 B cada).
  - Datacenter, memória: 2³⁴ B ≈ 17 GB já em n = 34.
  - Datacenter, tempo: n = 40 → 2⁴⁰ ≈ 1,1 × 10¹² passos ≈ 1 079 s; o maior n que cabe em 2 s é 30. Para n = 10³: 2¹⁰⁰⁰ ≈ 10³⁰¹ subconjuntos (o universo observável tem ≈ 10⁸⁰ átomos).
  - Pilha: profundidade n com 32 B por quadro. Com pilha de 8 MiB (padrão do Linux) estoura em n = 8 MiB / 32 B = **262 144**; com a pilha padrão de 1 MiB do Windows (a plataforma de medição), em n = 32 768. n = 10⁵ ainda cabe no primeiro caso, n = 10⁷ daria **stack overflow** nos dois.
- Rejeitada para ambos.

### 9. Codificação unária em disco (absurda)

Cada leitura x vira uma sequência de x bits no disco; a RAM fica quase O(1) e o custo vai para o disco.

- **Tempo:** **O(Σ valores)**. **Espaço:** RAM O(1); disco Σ valores bits (cresce com o valor, exponencial no número de bits da leitura).
- **Cálculo** (leituras uniformes, média 32 767,5; 1 passo = 0,98 ns, só os bits):

| n | Σ valores | disco | tempo |
|---|---|---|---|
| 10³ | 3,3 × 10⁷ bits | ≈ 4,10 MB | 32,2 ms |
| 10⁵ | 3,3 × 10⁹ bits | ≈ 410 MB | 3,22 s |
| 10⁷ | 3,3 × 10¹¹ bits | ≈ **41,0 GB** | **322 s** (sem contar latência de disco) |

- **Embarca:** a RAM fica em poucas dezenas de bytes, mas o custo foi transferido para o disco (41,0 GB), que quebra a premissa de que "memória" é só RAM. **Datacenter:** 322 s ≫ 2 s. Rejeitada para ambos.

### 10. Matriz de adjacência n×n de auditoria (absurda)

Guarda o resultado de todas as comparações entre pares, sem que o problema peça rastreabilidade.

- **Tempo:** **Θ(n²)**. **Espaço:** heap **Θ(n²)** células (1 bit por célula no melhor caso: n²/8 B).
- **Cálculo** (1 bit por célula; 1 passo = 0,98 ns):

| n | células | memória | tempo |
|---|---|---|---|
| 10³ | 10⁶ | 125 KB | 982 µs |
| 10⁵ | 10¹⁰ | 1,25 GB | 9,82 s |
| 10⁷ | 10¹⁴ | **12,5 TB** | 98 156 s ≈ **27,3 h** |

- **Embarca:** 125 KB já em n = 10³ (**488 vezes** o limite). **Datacenter:** sem limite de memória não é memória infinita; 12,5 TB e 27,3 h contra 2 s. Rejeitada para ambos.

### Cenários em que as absurdas deixariam de ser absurdas

| # | Cenário hipotético | Por quê |
|---|---|---|
| 7 | n ≤ ≈ 10 leituras por lote | 10! = 3,6 milhões de tentativas ≈ 35,6 ms |
| 8 | n ≤ ≈ 20 e o problema exigir enumerar subconjuntos | 2²⁰ ≈ 10⁶ subconjuntos ≈ 1,03 ms |
| 9 | Domínio minúsculo (ex.: 0–100) e disco barato | Σ valores ≤ 100 · n, vira uma contagem simples |
| 10 | n ≤ ≈ 10⁴ e auditoria exigida por contrato | 10⁸ células = 12,5 MB, ≈ 98,2 ms |

## 3. Justificativa da escolha: heapsort (Embarca) e bitmap (Datacenter)

1. **Extremos opostos do compromisso tempo × espaço.** Heapsort: espaço mínimo (159 B medidos), tempo n log n (1,94 s para 10⁷). Bitmap: espaço fixo de 8 KiB, tempo n (9,82 ms para 10⁷).
2. **Cada uma cumpre a régua do seu cliente e quebra a do outro.** O bitmap tem 8 192 B, 32 vezes o limite da Embarca. O heapsort fica na borda dos 2 s (7 de 25 rodadas fixadas passaram) e é 198 vezes mais lento que o bitmap.
3. **Alternativas descartadas.**
   - Embarca: só as estratégias 1 e 2 cabem em 256 B. A força bruta leva 102 s medidos para 10⁷ contra 1,94 s do heapsort (52 vezes mais). Mergesort (vetor auxiliar de 20 MB) e quicksort (pilha ≈ 740 B) estouram a memória. Hash, AVL, Bloom e bitmap precisam de KB.
   - Datacenter: hash, AVL e Bloom também caberiam no tempo, mas o hash não tem garantia de pior caso (O(n²)), o AVL encosta nos 2 s na estimativa conservadora e o Bloom perde dados. O bitmap é exato, determinístico e o mais rápido.
4. Linguagem: C, que o enunciado (Seção 3) recomenda para a Embarca e a Seção 6 permite para as duas.

## 4. Medições e extrapolação

### 4.1 Como foi medido

**Máquina de medição:** notebook com Intel Core Ultra 5 125H (14 núcleos / 18 threads, híbrido: 4 de desempenho + 8 eficientes + 2 de baixo consumo; L2 14 MB, L3 18 MB), 15,4 GB de RAM, Windows 11 Home (10.0.26200), plano de energia Equilibrado, na tomada. Compilador: MinGW-W64 GCC 16.1.0 (UCRT) com `-O2 -std=gnu11`. Em CPU híbrida o sistema pode mover o processo entre núcleos de desempenho e eficientes, então cada execução foi **fixada em um núcleo de desempenho** (thread lógica 12) com prioridade alta. A máquina de referência da disciplina deve repetir a medição com `./main dados/resultados_empiricos.csv`.

Dados: leituras pseudoaleatórias (xorshift32, semente fixa). Cada ponto: 5 rodadas, mediana do tempo, cópia nova do vetor a cada rodada; tempo e memória em rodadas separadas. O vetor de entrada não conta como espaço extra. Heap: alocador contador (`inst_calloc`). Pilha: pinta 4 096 B abaixo da função com 0xAA e mede até onde foram sobrescritos (limite superior).

Foram feitas **5 execuções completas fixadas** em um núcleo de desempenho e 1 sem fixar. O CSV registra a execução de mediana (aquela cuja mediana do heapsort em 10⁷ é a mediana das cinco).

| n | Heapsort: mediana (mín–máx) | Heapsort: extra | Bitmap: mediana (mín–máx) | Bitmap: extra | distintos |
|---|---|---|---|---|---|
| 10³ | 48,3 µs (41,6 µs–67,5 µs) | 159 B | 1,60 µs (1,40 µs–4,60 µs) | 9 271 B | 990 |
| 10⁵ | 11,3 ms (11,0 ms–11,6 ms) | 159 B | 465 µs (458 µs–473 µs) | 9 271 B | 51 340 |
| 10⁷ | 1,94 s (1,77 s–1,96 s) | 159 B | 9,82 ms (9,20 ms–10,4 ms) | 9 271 B | 65 536 |

Variabilidade entre as 5 execuções fixadas (mediana de cada execução): heapsort em 10⁷ de 1,64 s a 2,21 s, com a pior rodada isolada em 3,31 s; bitmap em 10⁷ de 7,52 ms a 9,87 ms. Sem fixar o núcleo: heapsort 1,67 s e bitmap 7,83 ms em 10⁷.

Evidências exigidas (Seção 8): Embarca usa 159 B ≤ 256 B em todos os n (heap 0 + pilha 159); Datacenter processa 10⁷ em 9,82 ms (pior rodada da execução 10,4 ms; pior de todas as 25 rodadas fixadas 10,4 ms) < 2 s; as duas acham as mesmas quantidades de distintos (990, 51 340 e 65 536). Teste independente: 8 768 casos aleatórios e de borda comparados com uma referência (qsort + dedup), sem divergência.

### 4.2 Modelo de extrapolação das oito estratégias não implementadas

Passos teóricos × uma constante medida:

- **K₁ = 0,98 ns por passo simples** = 9,82 ms ÷ 10⁷ (bitmap, n = 10⁷).
- **K₂ = 8,35 ns por unidade de n·log₂n** = 1,94 s ÷ (10⁷ × 23,25) (heapsort, n = 10⁷), para a família n log n.

| # | Tempo | Espaço extra |
|---|---|---|
| 1 Força bruta | K₁ · n²/2 | poucos bytes (índices) |
| 3 Hash | 4 · K₁ · n (hash, acesso, comparação, inserção) | 8 · n B |
| 4 AVL | K₂ · n · log₂n | 24 · n B |
| 6 Bloom (1%) | 7 · K₁ · n (k = 7) | 1,2 · n B |
| 7 Bogosort | K₁ · n · n! | poucos bytes |
| 8 Power set | K₁ · 2ⁿ | 2ⁿ B (1 B por subconjunto) |
| 9 Unária | K₁ · 32 767,5 · n | RAM: poucos bytes; disco 32 767,5 · n / 8 B |
| 10 Matriz | K₁ · n² | n²/8 B |

| # | n = 10³ | n = 10⁵ | n = 10⁷ |
|---|---|---|---|
| 1 tempo | 491 µs | 4,91 s | 13,6 h |
| 3 tempo / espaço | 3,93 µs / 8 KB | 393 µs / 800 KB | 39,3 ms / 80 MB |
| 4 tempo / espaço | 83,3 µs / 24 KB | 13,9 ms / 2,4 MB | 1,94 s / 240 MB |
| 6 tempo / espaço | 6,87 µs / 1,2 KB | 687 µs / 120 KB | 68,7 ms / 12 MB |
| 7 tempo | ≈ 10²⁵⁶² s | fora de escala | fora de escala |
| 8 tempo / espaço | ≈ 10²⁹² s / 10³⁰¹ B | fora de escala | fora de escala |
| 9 tempo / disco | 32,2 ms / 4,1 MB | 3,22 s / 410 MB | 322 s / 41 GB |
| 10 tempo / espaço | 982 µs / 125 KB | 9,82 s / 1,25 GB | 27,3 h / 12,5 TB |

As curvas completas estão nos gráficos do `relatorio.pdf` (tempo e espaço, 2 medidas + 8 extrapoladas no mesmo gráfico).

### 4.3 Teoria × prática e divergências

| n | Bitmap medido | previsto (K₁·n) | razão | Heapsort medido | previsto (K₂·n·log n) | razão |
|---|---|---|---|---|---|---|
| 10³ | 1,60 µs | 982 ns | 1,63 | 48,3 µs | 83,3 µs | 0,58 |
| 10⁵ | 465 µs | 98,2 µs | **4,74** | 11,3 ms | 13,9 ms | 0,81 |
| 10⁷ | 9,82 ms | 9,82 ms | 1,00 (calibração) | 1,94 s | 1,94 s | 1,00 (calibração) |

Os dois crescem na ordem de grandeza prevista (heapsort ×172 de 10⁵ para 10⁷, previsto ×140; o bitmap cresce só ×21, menos que os ×100 de Θ(n), porque o ponto de 10⁵ está inflado pelo desvio descrito abaixo).

1. **Bitmap em n = 10⁵ está 4,7× acima da reta: erro de predição de desvio (verificado).** Com ≈ 51% de leituras novas o `if` é imprevisível. Teste feito para confirmar: o **mesmo** conjunto de 100 000 leituras (50 000 valores distintos, cada um duas vezes) em três ordens, no mesmo núcleo: pareada [a, a, b, b, …] (padrão nova/repetida previsível) 0,91–0,94 ns por leitura; em blocos (50 000 novas e depois 50 000 repetidas, previsível) 1,84–1,90 ns; embaralhada (imprevisível) 3,58–3,76 ns, ou seja, **2 a 4 vezes mais lenta** com o mesmo trabalho. Em 10³ (quase tudo novo) e em 10⁷ (99,3% repetidas) o desvio é previsível. Em 10³ pesa também o ruído fixo (`calloc` de 8 KB ≈ 0,2 µs e resolução do relógio). A ordem de grandeza continua Θ(n).
2. **Heapsort 8,5× mais lento que K₁·n·log n: duas causas medidas.** (a) O passo do heapsort é mais pesado que o do bitmap: custa ≈ 5,4 a 5,7 ns por n·log₂n mesmo com o vetor dentro do cache (de 10⁴ a 3 × 10⁶ leituras, 20 KB a 6 MB), contra 0,98 ns do bitmap. A causa provável é a comparação entre os dois filhos e o desvio de difícil previsão a cada nível (não medimos isso isoladamente). (b) Faltas de cache: o custo sobe para 6,6 ns com 11,7 MB e para **8,0 ns com 19,5 MB**, pois o vetor de 10⁷ leituras passa dos 18 MB do L3. Por isso a família n log n usa K₂ (calibrado no heapsort) e não K₁. O bitmap só mexe em 8 KB.

| n | 10³ | 10⁴ | 10⁵ | 3 × 10⁵ | 10⁶ | 3 × 10⁶ | 6 × 10⁶ | 10⁷ |
|---|---|---|---|---|---|---|---|---|
| vetor | 2 KB | 20 KB | 195 KB | 586 KB | 1,9 MB | 5,9 MB | 11,7 MB | 19,5 MB |
| ns por n·log₂n | 3,30 | 5,41 | 5,65 | 5,39 | 5,74 | 5,66 | 6,62 | 8,01 |

3. **Força bruta: 13,6 h (n²/2) × 321 s (com o teto u = 65 536) × 102 s medidos.** O gráfico usa a teoria do catálogo (n²/2); o teto do domínio explica a diferença de ≈ 153× e a medição em C (0,31 ns por comparação, n = 10⁵: 694 ms) mostra que K₁ (0,98 ns, calibrado no bitmap, que escreve em memória e tem desvio imprevisível) **superestima em ≈ 3,1×** uma busca linear sequencial, que custa menos por passo. A ordem de grandeza e o veredito não mudam (51× acima de 2 s mesmo medida).
4. **Hash, AVL e Bloom: O(n) na teoria, mas com platô na prática.** Como d ≤ 65 536 (hash ≤ 512 KB, AVL ≤ 1,5 MB, Bloom ≤ 77 KiB). O gráfico segue o catálogo; nenhum veredito muda, pois todos continuam muito acima de 256 B.
5. **AVL coincide com o heapsort no gráfico** porque usa a mesma constante K₂ por construção. A verdade está entre a estimativa otimista (228 ms) e a conservadora (1,94 s).
6. **Ambiente (cache, SO, núcleo, relógio):** ruído relevante em n = 10³ (microssegundos); por isso a mediana de 5 rodadas. Em 10⁷ o heapsort variou de 1,77 s a 1,96 s dentro da execução registrada e de 1,59 s a 3,31 s nas 25 rodadas das 5 execuções fixadas (7 acima de 2 s). Em CPU híbrida, rodar em núcleo eficiente (ou o sistema migrar o processo) torna o tempo mais alto e mais variável; plano de energia, turbo e temperatura também pesam. Por isso o resultado válido para a disciplina é o da máquina de referência.
