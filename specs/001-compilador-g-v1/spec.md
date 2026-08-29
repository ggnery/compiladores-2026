# Especificação: Compilador para a linguagem G-V1

**Feature Branch**: `001-compilador-g-v1`
**Criado em**: 2026-08-29
**Status**: Draft
**Fonte**: `materials/trabalho1.pdf` (Trabalho 1 — UFG/INF, Compiladores 1, 2026-2, Prof. Thierson Couto Rosa)
**Prazo de entrega**: 15/10/2026 · **Apresentações**: 15/10, 19/10 e 22/10 de 2026

## Visão geral

Construir um **compilador didático** para a linguagem G-V1: dado um arquivo-fonte
`.g`, o programa `g-v1` deve analisá-lo (léxico, sintático, semântico) e, se estiver
correto, gerar código em *assembly* (MIPS). Se estiver errado, deve reportar o erro e
a linha onde ele ocorreu.

G-V1 é uma linguagem pequena: um único módulo `principal`, estruturada em blocos
aninhados, com dois tipos (`int` e `car`), atribuição, expressões aritméticas/relacionais/
lógicas, `leia`, `escreva`, `novalinha`, `se/entao/senao/fimse` e `enquanto`.

## Histórias de usuário (fatias de entrega)

O "usuário" aqui é o próprio aluno/professor executando `./g-v1 teste.g`.
Cada história é uma **fase entregável e demonstrável sozinha**.

### História 1 — Reconhecer se o programa é válido (P1) · meta 31/08/2026

Como aluno, quero rodar `./g-v1 teste.g` e saber se o programa está léxica e
sintaticamente correto, para ter o esqueleto do compilador funcionando.

**Por que P1**: é o alicerce. Sem léxico+sintático integrados nada mais existe. Já é uma
entrega com valor: um *reconhecedor* da linguagem.

**Teste independente**: `make && ./g-v1 testes/ok1.g` não imprime erro;
`./g-v1 testes/erro_sintatico.g` imprime `ERRO: ...` com a linha.

**Cenários de aceitação**:

1. **Dado** um programa G-V1 sintaticamente correto, **quando** rodo `./g-v1 arq.g`,
   **então** o programa termina sem mensagem de erro.
2. **Dado** um arquivo com o caractere `@`, **quando** rodo o compilador, **então** ele
   imprime `ERRO: CARACTERE INVÁLIDO` seguido do número da linha.
3. **Dado** um comentário `/*` que nunca fecha, **então** imprime
   `ERRO: COMENTÁRIO NAO TERMINA` seguido da linha.
4. **Dado** uma cadeia `"..."` que atravessa uma quebra de linha, **então** imprime
   `ERRO: CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA` seguido da linha.
5. **Dado** `se (x > 1) entao x = 2;` sem `fimse`, **então** imprime `ERRO: ` com a linha.

---

### História 2 — Construir a árvore sintática abstrata (P2) · meta 08/09/2026

Como aluno, quero que a análise sintática construa uma **AST** na memória e consiga
imprimi-la, para ter a representação intermediária que alimenta as fases seguintes.

**Por que P2**: a AST é a espinha dorsal — semântico e geração de código são percursos nela.

**Teste independente**: `./g-v1 testes/ok1.g` imprime a árvore (percurso) e a estrutura
impressa corresponde ao programa de entrada.

**Cenários de aceitação**:

1. **Dado** um programa correto, **quando** a análise termina, **então** existe um ponteiro
   `raiz` para uma árvore que representa o programa inteiro.
2. **Dado** um nó de operador/identificador/constante, **então** ele guarda o **lexema** e a
   **linha** do fonte onde apareceu.
3. **Dado** um bloco com declarações e comandos, **então** as declarações ficam na subárvore
   **esquerda** e a lista de comandos na **direita** (formato exigido pelo percurso semântico).

---

### História 3 — Pilha de tabelas de símbolos (P3) · meta 18/09/2026

Como aluno, quero uma pilha de tabelas de símbolos com as 4 operações do enunciado, para
representar escopos aninhados.

**Por que P3**: é o que permite a análise semântica saber se um nome existe e qual seu tipo.

**Teste independente**: um `main` de teste (ou o percurso da AST) empilha escopos, insere
nomes, busca e desempilha, imprimindo o resultado.

**Cenários de aceitação**:

1. **Dado** a pilha vazia, **quando** inicio a pilha, **então** ela existe e está vazia.
2. **Dado** um bloco novo, **quando** crio um escopo, **então** uma nova tabela é empilhada.
3. **Dado** o nome `x` declarado num bloco externo e não no interno, **quando** busco `x`
   estando no interno, **então** a busca percorre do topo para a base e o encontra.
4. **Dado** `x` declarado no bloco interno e também no externo, **quando** busco `x`,
   **então** retorna o **interno** (o mais recente).
5. **Dado** o fim de um bloco, **quando** removo o escopo, **então** a tabela do topo some.
6. **Dado** um nome inexistente, **então** a busca retorna ponteiro vazio.

---

### História 4 — Análise semântica (P4) · meta 28/09/2026

Como aluno, quero percorrer a AST verificando escopo e tipos, reportando erros semânticos
com a linha.

**Por que P4**: é o que separa "programa que o parser aceita" de "programa com sentido".

**Teste independente**: arquivos de teste com variável não declarada, redeclaração, e
mistura de `int` com `car` produzem `ERRO:` com a linha certa; o programa correto passa.

**Cenários de aceitação**:

1. **Dado** o uso de um nome não declarado em escopo válido, **então** erro semântico + linha.
2. **Dado** `x : int;` e `x = 'a';`, **então** erro de tipo (lado direito deve ter o tipo da variável).
3. **Dado** `c : car;` e `c + 1`, **então** erro: aritmética só sobre `int`.
4. **Dado** operador relacional entre operandos de tipos diferentes, **então** erro.
5. **Dado** `se (expr)` onde `expr` não é de tipo `int`, **então** erro.
6. **Dado** uma variável local sombreando uma externa de mesmo nome, **então** **não** é erro:
   a local prevalece durante o bloco.

---

### História 5 — Geração de código assembly (P5) · meta 14/10/2026

Como aluno, quero um novo percurso na AST que produza código MIPS para um programa
semanticamente correto.

**Por que P5**: é a última fase; depende de todas as anteriores.

**Teste independente**: `./g-v1 teste.g` gera um `.asm` que roda no simulador MIPS e
produz a saída esperada.

**Cenários de aceitação**:

1. **Dado** um programa correto, **então** as tabelas de símbolos são recriadas no percurso
   e usadas para calcular **posição na memória** e **tamanho** de cada variável.
2. **Dado** `escreva`, `leia`, `novalinha`, **então** o assembly gerado executa a E/S correta.
3. **Dado** `se` e `enquanto`, **então** o assembly gerado usa rótulos e desvios corretos.

---

### Casos de borda

- Bloco sem seção de variáveis (`{ comandos }`) e bloco com seção (`{decls} { comandos }`).
- Bloco aninhado como comando, em qualquer profundidade.
- Comando vazio `;`.
- `escreva` de cadeia literal vs. `escreva` de expressão.
- `CARCONST` com escape (`'\n'`, `'\t'`).
- `INTCONST` com sinal (`+5`, `-5`).
- Comentário `/* ... */` ocupando várias linhas (válido) vs. não fechado (erro).
- Arquivo passado por argumento inexistente ou ausente (`./g-v1` sem argumento).

## Requisitos funcionais

- **RF-001**: O executável chama-se `g-v1` e recebe o arquivo-fonte em `argv[1]`; abre-o em
  `yyin` e o fecha ao final da análise.
- **RF-002**: O analisador léxico é gerado pelo **Flex**, reconhece todos os tokens da
  Tabela 1 do enunciado, devolve o lexema em `yytext` e descarta espaços e comentários.
- **RF-003**: Erros léxicos são impressos como `ERRO:` + espaço + uma das três mensagens
  exatas do enunciado + o número da linha, na mesma linha.
- **RF-004**: O analisador sintático é gerado pelo **Bison** (LALR) a partir da gramática da
  Seção 2 do enunciado; `yyerror()` imprime `ERRO: ` + descrição + o número da linha.
- **RF-005**: Ações semânticas no arquivo Bison constroem a AST; cada nó guarda espécie,
  linha e (quando houver) lexema.
- **RF-006**: Existe uma pilha de tabelas de símbolos com as operações: iniciar, empilhar
  novo escopo, buscar nome do topo para a base, e desempilhar escopo.
- **RF-007**: O analisador semântico percorre a AST da raiz para as folhas, da esquerda para
  a direita, mantendo a pilha de escopos, e verifica declaração-antes-do-uso e tipos.
- **RF-008**: Todo erro (léxico, sintático ou semântico) reporta a **linha** do fonte e
  encerra a compilação.
- **RF-009**: O gerador de código percorre a AST recriando os escopos para atribuir posição
  de memória e tamanho a cada variável, e emite assembly MIPS.
- **RF-010**: Um `Makefile` gera léxico, sintático, compila, liga e produz o executável com
  um único comando `make`.

## Requisitos de entrega (checklist do professor)

- [ ] `g-v1.l` — entrada do Flex
- [ ] `g-v1.y` — entrada do Bison, com ações semânticas que constroem a AST
- [ ] Tabela de símbolos em C + header
- [ ] Analisador semântico em C + header
- [ ] Gerador de código em C + header
- [ ] `Makefile` (gera léxico, sintático, compila, liga, produz executável com `make`)
- [ ] Pasta compactada submetida na Turing até 15/10/2026
- [ ] Aluno sabe **explicar** cada parte (nota zero caso contrário)

## Critérios de sucesso

- **CS-001**: `make` em um Linux limpo (gcc 14, flex 2.6.4, bison 3.8.2) produz `g-v1` sem erros.
- **CS-002**: Todo programa de teste sintaticamente correto é aceito; todo incorreto é
  rejeitado com a linha certa.
- **CS-003**: Cada uma das 5 fases pode ser demonstrada isoladamente.
- **CS-004**: O aluno consegue explicar, sem consultar, o papel de cada arquivo e o fluxo
  fonte → tokens → AST → tabela de símbolos → checagem → assembly.

## Fora de escopo (fica para o Trabalho 2 / G-V2)

- Funções e o tipo vetor (*array*).
- Variáveis globais fora do `principal`.
- Compilação separada / `extern` na linguagem-fonte.
- Otimização de código.
