---
description: "Lista de tarefas — Compilador G-V1"
---

# Tarefas: Compilador para a linguagem G-V1

**Entrada**: documentos de design em `specs/001-compilador-g-v1/`

**Pré-requisitos**: `plan.md`, `spec.md`, `research.md`, `data-model.md`, `contracts/`

**Testes**: não há tarefas de teste automatizado. A validação é **manual**, com arquivos `.g`
em `testes/`, conforme `quickstart.md` — que é exatamente como o professor vai avaliar.

**Organização**: uma fase por história (US1..US5), na ordem do cronograma do enunciado.

## Formato: `[ID] [P?] [Story] Descrição`

- **[P]**: pode ser feito em paralelo (arquivo diferente, sem dependência pendente)
- **[US#]**: a qual história/fase a tarefa pertence
- Todo caminho de arquivo é relativo à raiz do repositório

---

## Fase 1: Setup (infraestrutura compartilhada)

**Objetivo**: ter as ferramentas certas e um programa de teste antes de escrever compilador.

- [x] T001 Instalar o Bison 3.8.2 com `brew install bison` e acrescentar `export PATH="/opt/homebrew/opt/bison/bin:$PATH"` ao `~/.zshrc` (o `/usr/bin/bison` é 2.3 e não serve — ver `research.md` D10)
- [x] T002 [P] Criar o arquivo `testes/ok1.g` com o programa correto de exemplo que está em `quickstart.md` seção 3
- [x] T003 [P] Criar o `Makefile` na raiz com as variáveis `CC`, `BISON` e `FLEX`, alvos `g-v1`, `sintatico.o`, `lexico.o` e `clean`, seguindo o modelo de `materials/examples/ex1/Makefile`
- [x] T004 Confirmar o ambiente rodando `bison --version` (precisa ser 3.8.x) e `flex --version` (2.6.4)

**Checkpoint**: `bison --version` diz 3.8.x.

---

## Fase 2: Fundação (pré-requisito bloqueante)

**Objetivo**: provar que Flex + Bison + Makefile estão ligados corretamente, **antes** de existir
gramática de verdade. É a parte que mais confunde — resolver ela isolada economiza horas.

**⚠️ Nenhuma história pode começar antes desta fase terminar.**

- [x] T005 Criar `g-v1.y` mínimo: prólogo `%{ ... %}` com `#include <stdio.h>`, `#include <stdlib.h>`, `#include <string.h>` e as declarações exigidas pelo enunciado (`extern int yylineno; extern char* yytext; extern int yylex(); extern FILE* yyin; void yyerror(char const* s);`), um `%token` de teste, uma produção qualquer, e a seção epílogo vazia
- [x] T006 Escrever `main(int argc, char** argv)` no epílogo de `g-v1.y`: valida `argc`, faz `yyin = fopen(argv[1], "r")`, chama `yyparse()` e **fecha o arquivo** ao final (exigência da Seção 3.2 do enunciado)
- [x] T007 Escrever `yyerror(char const* s)` no epílogo de `g-v1.y` imprimindo `ERRO: ` + mensagem + `yylineno`, terminando com `exit(1)`
- [x] T008 [P] Criar `g-v1.l` mínimo: prólogo com `#include "g-v1.tab.h"`, `%option yylineno`, `%option noyywrap`, uma regra que consome espaços e uma regra `.` provisória
- [x] T009 Rodar `make` e obter o executável `./g-v1`; rodar `./g-v1 testes/ok1.g` sem *crash*

**Checkpoint**: `make` produz `./g-v1`. A partir daqui é só preencher regras e produções.

---

## Fase 3: US1 — Reconhecer se o programa é válido (P1) 🎯 MVP

**Meta do enunciado: 31/08/2026**

**Objetivo**: `./g-v1 arq.g` diz se o programa é léxica e sintaticamente válido, e reporta a
linha do erro quando não é.

**Teste independente**: `./g-v1 testes/ok1.g` não imprime nada; cada arquivo de erro imprime a
mensagem exata do enunciado com a linha correta.

### Analisador léxico (`g-v1.l`)

- [x] T010 [US1] Adicionar em `g-v1.l` a seção de definições regulares: `DIGITO [0-9]`, `LETRA [A-Za-z_]` e a *start condition* `%x COMENTARIO`
- [x] T011 [US1] Adicionar as regras das 11 palavras reservadas de `contracts/tokens.md` (`principal`, `int`, `car`, `leia`, `escreva`, `novalinha`, `se`, `entao`, `senao`, `fimse`, `enquanto`) em `g-v1.l` — **antes** da regra de `IDENTIFICADOR`
- [x] T012 [US1] Adicionar em `g-v1.l` os operadores compostos (`||`, `&`, `==`, `!=`, `>=`, `<=`) e os caracteres simples devolvidos como ASCII (`{ } ( ) ; : , = < > + - * / !`)
- [x] T013 [US1] Adicionar em `g-v1.l` as regras de `IDENTIFICADOR`, `INTCONST`, `CARCONST` e `CADEIACARACTERES` (o `strdup` do lexema foi adiado para a Fase 4, onde a AST passa a consumi-lo — nesta fase o reconhecedor não guarda lexema)
- [x] T014 [US1] Implementar comentários em `g-v1.l` com a *start condition* `COMENTARIO`: `"/*"` entra, `"*/"` volta ao `INITIAL`, e o corpo consome qualquer coisa (incluindo `\n`), seguindo `materials/examples/ex2/lexico.l`
- [x] T015 [US1] Implementar os três erros léxicos em `g-v1.l` com o texto **exato** de `contracts/cli-e-erros.md`: regra `.` → `CARACTERE INVÁLIDO`; `<COMENTARIO><<EOF>>` → `COMENTÁRIO NAO TERMINA`; cadeia aberta com `\n` → `CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA`
- [x] T016 [US1] Adicionar em `g-v1.l` as regras que descartam espaços, tabulações e quebras de linha, e a regra `<<EOF>>`

### Analisador sintático (`g-v1.y`)

- [x] T017 [P] [US1] Substituir os `%token` de teste em `g-v1.y` pela lista completa de `contracts/gramatica-bison.md` (11 palavras reservadas + 6 operadores compostos + os 4 tokens com lexema)
- [x] T018 [US1] Transcrever em `g-v1.y` as produções da Seção 2 do enunciado, **literalmente** como estão em `contracts/gramatica-bison.md`, ainda sem ações semânticas, com `%start Programa`
- [x] T019 [US1] Ajustar `yyerror()` em `g-v1.y` para imprimir `ERRO: sintatico proximo a "<yytext>" - linha <yylineno>`

### Validação

- [x] T020 [US1] Rodar `make` e confirmar, na saída do Bison, **zero** avisos de conflito — a gramática original tem 0 shift/reduce e 0 reduce/reduce (`research.md` D2); qualquer conflito significa erro de transcrição
- [x] T021 [P] [US1] Criar `testes/erro_lexico1.g` (caractere `@` no meio do código) e `testes/ok2.g` (cobre `leia`, comando vazio, todos os operadores, bloco aninhado, menos unário e `CARCONST` com escape)
- [x] T022 [P] [US1] Criar `testes/erro_lexico2.g` (comentário `/*` que nunca fecha)
- [x] T023 [P] [US1] Criar `testes/erro_lexico3.g` (cadeia `"` com quebra de linha antes de fechar)
- [x] T024 [P] [US1] Criar `testes/erro_sintatico.g` (comando `se` sem o `fimse`)
- [x] T025 [US1] Rodar os 5 testes da tabela "Fase 1" de `quickstart.md` e conferir mensagem e número de linha de cada um
- [x] T026 [US1] Escrever `docs/EXPLICACAO-fase1.md` em 1 página: o que é token, o que o Flex gera, o que o Bison gera, e como o `.tab.h` liga os dois

**Checkpoint**: o compilador aceita programas válidos e rejeita inválidos com a linha certa.
**Isto já é uma entrega demonstrável.**

---

## Fase 4: US2 — Construir a árvore sintática abstrata (P2)

**Meta do enunciado: 08/09/2026** · **Depende de**: US1 completa

**Objetivo**: a análise sintática monta a AST na memória e o programa consegue imprimi-la.

**Teste independente**: `./g-v1 testes/ok1.g` imprime um percurso da árvore que corresponde ao
programa de entrada.

- [ ] T027 [US2] Criar `ast.h` com o `enum Especie`, o `enum Tipo`, a `struct no` (campos `especie`, `linha`, `lexema`, `tipo`, `filho1`, `filho2`, `filho3`) e os protótipos, exatamente como em `data-model.md` seção 1
- [ ] T028 [US2] Implementar `criaNo()` em `ast.c`: único `malloc` do projeto, preenche os campos, inicia `tipo` como `TIPO_NENHUM` e aborta com mensagem se a memória acabar
- [ ] T029 [US2] Adicionar `%union { char* lexema; No* no; }` em `g-v1.y`, marcar `%token <lexema>` nos 4 tokens com lexema e `%type <no>` em todos os não-terminais listados em `contracts/gramatica-bison.md`
- [ ] T030 [US2] Adicionar as ações das expressões em `g-v1.y` (`PrimExpr` até `OrExpr` e `Expr`): folhas com `criaNo(ID/INT_CONST/CAR_CONST, yylineno, $1, NULL,NULL,NULL)`, binários com os dois operandos, e `{ $$ = $1; }` nas produções de repasse
- [ ] T031 [US2] Adicionar as ações dos comandos em `g-v1.y` (`LEIA_CMD`, `ESCREVA_CMD`, `ESCREVA_STR`, `NOVALINHA_CMD`, `SE_CMD` com 3 filhos, `ENQUANTO_CMD`), seguindo a tabela de filhos de `data-model.md`
- [ ] T032 [US2] Adicionar as ações de `ListaComando` e `ListaDeclVar` em `g-v1.y`, encadeando à direita (`filho1` = item, `filho2` = resto, `NULL` no fim), como o `lstStmt` de `materials/examples/ex2/simpleLang.y`
- [ ] T033 [US2] Adicionar as ações de `Bloco`, `VarSection` e `Programa` em `g-v1.y`, garantindo a invariante: no nó `BLOCO`, **declarações em `filho1` e comandos em `filho2`** (é o que a Seção 6 do enunciado exige e o que faz o percurso semântico funcionar)
- [ ] T034 [US2] Declarar a variável global `No* raiz;` em `g-v1.y` e atribuí-la na ação de `Programa`
- [ ] T035 [P] [US2] Implementar `imprimeArvore(No* no, int nivel)` em `ast.c`: percurso recursivo com indentação por nível, imprimindo espécie, lexema e linha de cada nó
- [ ] T036 [P] [US2] Implementar `nomeEspecie(Especie e)` em `ast.c`: `switch` que devolve o nome legível de cada espécie (mesma ideia do `obtemEspecieNoEnumLin` do professor)
- [ ] T037 [US2] Chamar `imprimeArvore(raiz, 0)` na `main()` de `g-v1.y` depois de `yyparse()`
- [ ] T038 [US2] Acrescentar `ast.o` ao `Makefile` (regra de compilação e a ligação no alvo `g-v1`)
- [ ] T039 [US2] Rodar `./g-v1 testes/ok1.g` e conferir à mão que a árvore impressa corresponde ao programa: bloco com declarações à esquerda, `SE_CMD` com três filhos, lista de comandos encadeada
- [ ] T040 [US2] Escrever `docs/EXPLICACAO-fase2.md` em 1 página: o que é uma AST, por que a pilha semântica do Bison carrega ponteiros, e o que `$$`, `$1`, `$3` significam

**Checkpoint**: existe uma AST na memória, impressa e conferida.

---

## Fase 5: US3 — Pilha de tabelas de símbolos (P3)

**Meta do enunciado: 18/09/2026** · **Depende de**: Fase 2 (fundação) apenas

> 💡 Esta fase **não depende da AST**. Pode ser escrita em paralelo com a US2 se sobrar tempo,
> ou por outra pessoa da dupla. Ela só passa a ser *usada* na US4.

**Objetivo**: as 4 operações de escopo do enunciado (Seção 5) funcionando e demonstráveis.

**Teste independente**: um `main` de teste empilha escopos, insere nomes, busca do topo para a
base e desempilha, imprimindo o resultado.

- [ ] T041 [P] [US3] Criar `tabela.h` com as structs `Entrada` (`lexema`, `tipo`, `linha`, `deslocamento`, `prox`) e `Escopo` (`entradas`, `abaixo`) e os 5 protótipos de `data-model.md` seção 2
- [ ] T042 [US3] Implementar em `tabela.c` a operação (a) do enunciado: `iniciaPilha()` — topo da pilha vira `NULL`
- [ ] T043 [US3] Implementar em `tabela.c` a operação (b): `empilhaEscopo()` — cria `Escopo` com tabela vazia e o coloca na cabeça da pilha
- [ ] T044 [US3] Implementar em `tabela.c` a operação (d): `desempilhaEscopo()` — remove o escopo do topo e libera as entradas dele
- [ ] T045 [US3] Implementar `insere(char* lexema, Tipo tipo, int linha)` em `tabela.c`: insere no escopo do topo e devolve `NULL` se o nome **já existe naquele mesmo escopo** (redeclaração)
- [ ] T046 [US3] Implementar em `tabela.c` a operação (c): `busca(char* lexema)` — percorre **do topo para a base** e devolve a primeira entrada encontrada, ou `NULL`; é esta ordem que faz a variável local sobrepor a externa
- [ ] T047 [P] [US3] Implementar `imprimePilha()` em `tabela.c` para demonstrar os escopos na apresentação
- [ ] T048 [US3] Acrescentar `tabela.o` ao `Makefile`
- [ ] T049 [US3] Testar as 6 situações da tabela "Fase 3" de `quickstart.md`: escopo interno vence o externo, busca sobe até a base, nome inexistente devolve vazio, desempilhar remove o topo
- [ ] T050 [US3] Escrever `docs/EXPLICACAO-fase3.md` em 1 página: por que uma **pilha** de tabelas (e não uma tabela só), e por que a busca vai do topo para a base

**Checkpoint**: as 4 operações do enunciado existem, com nome e comportamento conferidos.

---

## Fase 6: US4 — Análise semântica (P4)

**Meta do enunciado: 28/09/2026** · **Depende de**: US2 (AST) + US3 (tabela)

**Objetivo**: percorrer a AST checando escopo e tipos, reportando erro com a linha.

**Teste independente**: `testes/ok1.g` passa em silêncio; cada teste de erro semântico imprime
`ERRO:` com a linha certa.

- [ ] T051 [P] [US4] Criar `semantico.h` com o protótipo `void analisaSemantica(No* raiz);`
- [ ] T052 [US4] Implementar `erroSemantico(char* msg, int linha)` em `semantico.c`: imprime `ERRO: ` + mensagem + `- linha N` e encerra com `exit(1)`, no formato de `contracts/cli-e-erros.md`
- [ ] T053 [US4] Implementar `visitaBloco(No* no)` em `semantico.c`: empilha escopo → visita as declarações (`filho1`) → visita os comandos (`filho2`) → desempilha escopo. É o percurso "esquerda antes da direita" descrito na Seção 6 do enunciado
- [ ] T054 [US4] Implementar `visitaDeclaracoes(No* no)` em `semantico.c`: percorre a lista `LISTA_DECL` chamando `insere()`; se `insere()` devolver `NULL`, erro de redeclaração no mesmo escopo
- [ ] T055 [US4] Implementar `Tipo visitaExpressao(No* no)` em `semantico.c` para as folhas: `INT_CONST` → `TIPO_INT`, `CAR_CONST` → `TIPO_CAR`, `ID` → `busca()` no escopo (erro "nao declarada" se não achar) e devolve o tipo da entrada
- [ ] T056 [US4] Estender `visitaExpressao` em `semantico.c` para os operadores, aplicando a tabela de regras de tipo de `data-model.md` seção 3: aritméticos exigem `int`; relacionais e de igualdade exigem operandos do **mesmo** tipo e resultam `int`; lógicos exigem `int`
- [ ] T057 [US4] Implementar a checagem de `ATRIB` em `semantico.c`: o tipo do lado direito precisa ser **igual** ao tipo da variável do lado esquerdo (não há conversão automática em G-V1)
- [ ] T058 [US4] Implementar `visitaComando(No* no)` em `semantico.c` para `LEIA_CMD`, `ESCREVA_CMD`, `ESCREVA_STR`, `NOVALINHA_CMD` e o caso `BLOCO` (bloco aninhado → chama `visitaBloco`, criando escopo novo)
- [ ] T059 [US4] Implementar em `semantico.c` a checagem de `SE_CMD` e `ENQUANTO_CMD`: a condição precisa ter tipo `int`; depois visita o corpo (e o `senao`, se `filho3` não for `NULL`)
- [ ] T060 [US4] Chamar `iniciaPilha()` e `analisaSemantica(raiz)` na `main()` de `g-v1.y`, depois de `yyparse()`; acrescentar `semantico.o` ao `Makefile`
- [ ] T061 [P] [US4] Criar `testes/erro_semantico1.g` (variável usada sem declarar)
- [ ] T062 [P] [US4] Criar `testes/erro_semantico2.g` (mesmo nome declarado duas vezes no mesmo bloco)
- [ ] T063 [P] [US4] Criar `testes/erro_semantico3.g` (variável `int` recebendo `car`, e `car + 1`)
- [ ] T064 [P] [US4] Criar `testes/ok_escopo.g` com bloco interno redeclarando um nome do bloco externo — este arquivo tem que passar **sem** erro (sombreamento é permitido)
- [ ] T065 [US4] Rodar os 6 casos da tabela "Fase 4" de `quickstart.md` e conferir mensagem e linha
- [ ] T066 [US4] Escrever `docs/EXPLICACAO-fase4.md` em 1 página: por que declarações ficam à esquerda na árvore, e como o percurso mantém a pilha de escopos sincronizada com os blocos

**Checkpoint**: programas com sentido passam; programas sem sentido são recusados com a linha.

---

## Fase 7: US5 — Geração de código assembly (P5)

**Meta do enunciado: 14/10/2026** · **Depende de**: US4 completa

**Objetivo**: novo percurso na AST emitindo MIPS para programas semanticamente corretos.

**Teste independente**: o `.asm` gerado roda no MARS/SPIM e produz a saída que o programa
G-V1 descreve.

- [ ] T067 [US5] Estudar o PDF *MIPS Assembly Language Programming* (Bloco 2 da Turing) e anotar em `docs/EXPLICACAO-fase5.md` a convenção escolhida: quais registradores, como a pilha cresce, quais `syscall` usar para ler/escrever
- [ ] T068 [P] [US5] Criar `codigo.h` com o protótipo `void geraCodigo(No* raiz);`
- [ ] T069 [US5] Implementar em `codigo.c` o percurso que recria os escopos (empilha/desempilha como na US4), agora preenchendo o campo `deslocamento` de cada entrada e somando o tamanho total a alocar por bloco
- [ ] T070 [US5] Implementar em `codigo.c` a emissão do esqueleto do programa: seção `.data`, seção `.text`, rótulo `main` e a saída (`syscall` 10)
- [ ] T071 [US5] Implementar em `codigo.c` a geração de código das expressões (percurso pós-ordem: gera o filho esquerdo, o direito, depois a operação), incluindo constantes, variáveis, aritméticos, relacionais e lógicos
- [ ] T072 [US5] Implementar em `codigo.c` a geração de `ATRIB`, `LEIA_CMD`, `ESCREVA_CMD`, `ESCREVA_STR` e `NOVALINHA_CMD` usando as `syscall` de E/S
- [ ] T073 [US5] Implementar em `codigo.c` a geração de `SE_CMD` e `ENQUANTO_CMD` com rótulos únicos (contador global) e desvios condicionais
- [ ] T074 [US5] Chamar `geraCodigo(raiz)` na `main()` de `g-v1.y` (só se a análise semântica passou) e acrescentar `codigo.o` ao `Makefile`
- [ ] T075 [US5] Rodar `./g-v1 testes/ok1.g`, carregar o assembly gerado no MARS/SPIM e conferir que a saída bate com o esperado
- [ ] T076 [US5] Completar `docs/EXPLICACAO-fase5.md`: como o deslocamento de cada variável é decidido e por que o percurso das expressões é pós-ordem

**Checkpoint**: o compilador está completo, do fonte ao assembly.

---

## Fase 8: Fechamento e entrega

**Prazo: 15/10/2026 (entrega) · Apresentação: 15, 19 e 22/10/2026**

- [ ] T077 Rodar `make clean && make` numa cópia limpa e confirmar que **só** `make` produz o executável (exigência explícita do enunciado)
- [ ] T078 Testar em Linux (ou máquina virtual/container) com gcc 14, flex 2.6.4 e bison 3.8.2 — é o ambiente de avaliação
- [ ] T079 [P] Revisar os comentários de `g-v1.l`, `g-v1.y`, `ast.c`, `tabela.c`, `semantico.c` e `codigo.c`: comentário em português no ponto onde a dúvida aparece, sem enfeite
- [ ] T080 [P] Conferir a lista de entrega da Seção 8 do enunciado item a item: `g-v1.l`, `g-v1.y`, tabela de símbolos (+`.h`), semântico (+`.h`), gerador (+`.h`), `Makefile`
- [ ] T081 Compactar a pasta (sem `.o`, sem executável, sem `materials/`) e submeter na Turing
- [ ] T082 Ensaiar a apresentação usando os 5 arquivos `docs/EXPLICACAO-faseN.md` — o enunciado é explícito: **compilador correto + aluno que não sabe explicar = nota zero**

---

## Dependências e ordem de execução

### Dependências entre fases

- **Setup (Fase 1)**: sem dependências — começa agora
- **Fundação (Fase 2)**: depende do Setup — **bloqueia todas as histórias**
- **US1** (Fase 3): depende da Fundação
- **US2** (Fase 4): depende da US1 (as ações semânticas entram na gramática já pronta)
- **US3** (Fase 5): depende **só da Fundação** — pode andar em paralelo com a US2
- **US4** (Fase 6): depende da US2 **e** da US3
- **US5** (Fase 7): depende da US4
- **Fechamento (Fase 8)**: depende de tudo

```
Setup → Fundação → US1 → US2 ─┐
                     │        ├→ US4 → US5 → Entrega
                     └→ US3 ──┘
```

> Diferente de um projeto web, aqui as histórias **não** são independentes entre si: cada fase
> do compilador consome a representação produzida pela anterior. O que cada fase mantém é a
> propriedade de ser **demonstrável sozinha** — que é o que o professor vai avaliar.

### Oportunidades de paralelismo

- **T002 e T003** (teste e Makefile) são arquivos diferentes — em paralelo
- **T017** (`g-v1.y`) é arquivo diferente de T010–T016 (`g-v1.l`) — em paralelo
- **T021–T024** (os 4 arquivos de teste) — em paralelo
- **T035 e T036** (impressão da árvore) e **T041/T047** (tabela) — arquivos diferentes
- **T061–T064** (testes semânticos) — em paralelo
- **Trabalho em dupla**: depois da US1, uma pessoa toca a US2 (AST) e a outra a US3 (tabela de símbolos), que não se cruzam. É o único ponto do trabalho com paralelismo real.

---

## Exemplo de paralelismo: US1

```bash
# Enquanto uma pessoa escreve as regras do léxico (mesmo arquivo, sequencial):
T010 → T011 → T012 → T013 → T014 → T015 → T016   em g-v1.l

# a outra transcreve a gramática (arquivo diferente, em paralelo):
T017 → T018 → T019                                em g-v1.y

# depois, juntos:
T020 (conferir 0 conflitos) → T021..T024 (testes, paralelos) → T025 (validar)
```

---

## Estratégia de implementação

### MVP primeiro (só a US1)

1. Fase 1: Setup
2. Fase 2: Fundação (**crítica** — bloqueia tudo)
3. Fase 3: US1
4. **PARE e VALIDE**: rode os 5 testes de `quickstart.md`
5. Esta é a entrega de **31/08** — já vale como marco do cronograma

### Entrega incremental

Cada fase termina com: código compilando + teste correto + teste com erro + `docs/EXPLICACAO-faseN.md`.
Nunca comece a fase N+1 com a fase N quebrada (Princípio III da constituição).

### Sobre o prazo

O cronograma do enunciado é apertado no começo: **a US1 vence em 2 dias (31/08)**. As tarefas
T001–T026 são a prioridade absoluta desta semana. A US3 (tabela de símbolos) é a fase mais
independente e pode ser adiantada se sobrar tempo.
