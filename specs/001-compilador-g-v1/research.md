# Fase 0 — Pesquisa e decisões técnicas

Cada item: **Decisão**, **Por quê**, **O que foi descartado**. Tudo o mais simples possível.

---

## D1. Linguagem de implementação: **C** (não C++)

**Decisão**: C puro (gcc), padrão do compilador (sem `-std` exótico).

**Por quê**: os dois exemplos do professor (`ex1`, `ex2`) são em C; o Bison/Flex geram C por
padrão; a AST é uma `struct` com ponteiros, que é exatamente o conteúdo de Estrutura de Dados
que o enunciado manda usar. Menos conceitos novos = mais fácil de explicar.

**Descartado**: C++ (o Flex tem opção de gerar C++). Traria classes, `new/delete` e um
`yyFlexLexer` que ninguém no exemplo do professor usa. Complexidade sem ganho.

---

## D2. A gramática do enunciado é LALR(1) **sem conflitos** — verificado

**Decisão**: transcrever a gramática da Seção 2 do PDF **literalmente** para o `.y`, sem
declarações de precedência (`%left`, `%right`, `%nonassoc`).

**Por quê**: a gramática já é *estratificada* — a precedência está codificada na cascata
`Expr → OrExpr → AndExpr → EqExpr → DesigExpr → AddExpr → MulExpr → UnExpr → PrimExpr`.
Cada nível só desce para o próximo, então não há ambiguidade a resolver. O `senao` também
não gera o clássico *dangling else* porque todo `se` termina com `fimse` explícito.

**Evidência**: a gramática completa foi transcrita e passada ao Bison para teste.
Resultado: **0 conflitos shift/reduce e 0 reduce/reduce**. Nenhuma diretiva de precedência
é necessária.

**Descartado**: reescrever a gramática com `%left '+' '-'` etc. (estilo calculadora). Ficaria
mais curta, mas divergiria da gramática que o professor entregou — e o enunciado é a
referência de correção. Fidelidade ganha de brevidade.

---

## D3. Contagem de linhas: `%option yylineno`

**Decisão**: usar `%option yylineno` no Flex e ler `yylineno` (declarado `extern int yylineno;`
no `.y`) para carimbar linha nas mensagens de erro e nos nós da AST.

**Por quê**: o enunciado cita `yylineno` explicitamente na Seção 3.2. É uma linha de código
contra a alternativa de manter um contador manual `numLinha++` em cada regra `\n`.

**Descartado**:
- Contador manual `numLinha` (é o que o exemplo do professor faz). Funciona, mas exige lembrar
  de incrementar em **toda** regra que consome `\n` — inclusive dentro do comentário. Fonte
  clássica de bug.
- Guardar `{lexema, linha}` numa struct empurrada pela pilha semântica, para ter a linha exata
  do token (e não a linha em que a redução aconteceu). É mais preciso, mas adiciona um tipo
  novo. **Se** na Fase 2 as linhas dos erros saírem deslocadas em programas multilinha, esta é
  a correção — está registrada aqui de propósito.

---

## D4. Comentários e cadeias: *start conditions* do Flex

**Decisão**: `%x COMENTARIO` para `/* ... */` (igual ao exemplo do professor) e uma única
expressão regular para `CADEIACARACTERES`, que **não** aceita `\n` dentro.

**Por quê**: comentário pode ocupar várias linhas e pode conter qualquer coisa — uma regex
única fica ilegível e gulosa. *Start condition* é o mecanismo do Flex feito para isso, e o
`<COMENTARIO><<EOF>>` dá de graça o erro "COMENTÁRIO NAO TERMINA".
A cadeia é o contrário: **não pode** cruzar linha, então a regex `\"[^\"\n]*\"` já resolve, e
o caso `\"[^\"\n]*` sem fechar vira o erro "CADEIA ... OCUPA MAIS DE UMA LINHA".

**Descartado**: regex única `/\*([^*]|\*+[^*/])*\*+/` para comentário. Correta, mas é o tipo de
linha que o aluno não consegue explicar no quadro.

---

## D5. Pilha semântica: `%union` com 2 campos

**Decisão**:

```c
%union {
    char* lexema;      /* para IDENTIFICADOR, INTCONST, CARCONST, CADEIACARACTERES */
    No*   no;          /* para todos os não-terminais: ponteiro para nó da AST */
}
```

**Por quê**: é o mínimo que funciona. Terminais com lexema carregam `char*` (o Flex faz
`strdup(yytext)`); não-terminais carregam o ponteiro do nó da árvore que acabaram de construir.
É exatamente o desenho do `ex2` do professor (`Tpont` + `numLexema`).

**Descartado**: um `%union` com um campo por tipo de nó. Mais "tipado", mas multiplica casos e
não agrega nada — a AST é homogênea (ver `data-model.md`).

---

## D6. Nó da AST: **um único tipo de struct, com 3 filhos**

**Decisão**: uma struct só, com `especie` (enum), `linha`, `lexema`, `tipo` e `filho1/2/3`.

**Por quê**: G-V1 precisa de no máximo 3 filhos — o `se ... entao ... senao ... fimse` tem
condição + então + senão. Uma struct só significa **uma** função construtora, **uma** função de
percurso e um `switch` — em vez de uma hierarquia de tipos. É a versão G-V1 do `Toperador` do
professor (que tinha 2 filhos porque SimpleLang não tinha `else`).

**Descartado**:
- Só 2 filhos, agrupando `entao`/`senao` num nó auxiliar. Economiza um ponteiro e custa um
  conceito extra ("nó que não corresponde a nada na linguagem"). Não compensa.
- Uma struct por espécie de nó, com *union* ou *tagged union*. É o desenho "certo" de livro,
  e é muito mais código para explicar.

---

## D7. Listas (comandos e declarações): nó de encadeamento à direita

**Decisão**: `ListaComando` e `ListaDeclVar` viram uma cadeia de nós `LISTA_CMD` / `LISTA_DECL`,
onde `filho1` = o item e `filho2` = o resto da lista (`NULL` no fim).

**Por quê**: é literalmente o `lstStmt` do exemplo do professor, e casa com a recursão à direita
da gramática dele. Percorrer a lista vira um `while (no) { visita(no->filho1); no = no->filho2; }`.

**Consequência importante e desejada**: no nó `BLOCO`, `filho1` = declarações e `filho2` =
comandos. Isso reproduz o que o enunciado descreve na Seção 6: *"as declarações de variáveis
aparecem na subárvore à esquerda e a lista de comandos na subárvore à direita"* — e é o que
torna o percurso semântico "esquerda→direita" natural (declara o escopo, depois usa).

---

## D8. Tabela de símbolos: pilha de listas encadeadas

**Decisão**: cada escopo é uma **lista encadeada** de entradas `{lexema, tipo, linha, deslocamento}`;
a pilha de escopos é outra lista encadeada (empilhar = inserir na cabeça).

**Por quê**: os blocos de G-V1 têm punhados de variáveis, não milhares. Busca linear é
instantânea nessa escala e cabe em 15 linhas de C explicáveis. As 4 operações pedidas pelo
enunciado viram 4 funções pequenas.

**Descartado**: tabela hash. É o que um compilador de verdade usa, e é o que o professor cita
como possibilidade — mas exige função de hash, tratamento de colisão e redimensionamento. Se a
disciplina exigir hash na apresentação, trocar só o arquivo `tabela.c` (a interface não muda).

---

## D9. Comportamento em caso de erro: reporta e **encerra**

**Decisão**: qualquer erro (léxico, sintático ou semântico) imprime a mensagem com a linha e
chama `exit(1)`.

**Por quê**: é o que o exemplo do professor faz (`yyerror` termina com `exit(1)`) e o enunciado
diz, para o semântico, *"reportar o erro, o número da linha e terminar a compilação"*.
Recuperação de erro (continuar e listar todos os erros) é um tema inteiro à parte.

**Descartado**: `error` recovery do Bison (regras com o token especial `error`). Poderoso e
difícil de explicar; não é pedido.

---

## D10. Ambiente de build: Bison 3.8.2 / Flex 2.6.4 / gcc 14

**Decisão**: o `Makefile` usa as variáveis `CC`, `BISON` e `FLEX`, com valores padrão que
funcionam no Linux do professor (`gcc`, `bison`, `flex`), e o aluno sobrescreve no macOS.

**Por quê / risco real detectado neste computador**:

| Ferramenta | Exigida pelo professor | Presente neste Mac | Situação |
|---|---|---|---|
| Bison | **3.8.2** | **2.3** (`/usr/bin/bison`) | ⚠️ **incompatível** |
| Flex | 2.6.4 | 2.6.4 (Apple) | ok |
| gcc | 14 | `/usr/bin/gcc` = clang | ok para C simples |

O Bison 2.3 da Apple é de 2008 e não aceita coisas do 3.x (`-H`, `%define parse.error verbose`,
`api.header.include`). O exemplo `ex1` do professor usa `bison -HsimpleLang.h`, que **não roda**
aqui. Ação necessária antes da Fase 1: `brew install bison` e colocar
`/opt/homebrew/opt/bison/bin` no `PATH` (ou apontar `BISON=` no Makefile).

Nota: o Makefile do professor fixa `CC = gcc-14`, que não existe neste Mac. Por isso `CC` fica
como variável — assim o mesmo Makefile roda nos dois lugares e o professor continua digitando
só `make`.

**Descartado**: usar o Bison 2.3 e evitar recursos do 3.x. Arriscado — o trabalho é avaliado no
3.8.2, e diferenças de comportamento apareceriam só na correção.

---

## D11. Geração de código: MIPS (SPIM/MARS)

**Decisão**: alvo MIPS, com as variáveis alocadas em pilha por *deslocamento* calculado na
tabela de símbolos; E/S via `syscall`.

**Por quê**: o material de apoio do enunciado é justamente *MIPS Assembly Language Programming*,
e MIPS tem simulador (MARS/SPIM) para testar sem hardware.

**Pendência aceitável**: os detalhes (registradores, convenção de pilha) só são decididos na
Fase 5, com o material de aula em mãos. Não bloqueia nada agora.

---

## D12. `INTCONST` sem sinal — desvio deliberado da Tabela 1 (verificado)

**Decisão**: no Flex, `INTCONST` é `[0-9]+`. O sinal fica por conta da gramática, que já tem
`UnExpr → '-' PrimExpr`.

**Por quê**: a Tabela 1 do enunciado descreve `INTCONST` como "podendo ou ter sinal (+ ou -)
ou não". Implementado literalmente (`[+-]?[0-9]+`), o casamento mais longo do Flex quebra
expressões comuns:

```
b = a-1;
```

O `-1` viraria um `INTCONST` de dois caracteres, e a gramática receberia
`IDENTIFICADOR INTCONST` sem operador. **Testado nos dois modos**:

| Regra no Flex | `b = a-1;` |
|---|---|
| `[+-]?{DIGITO}+` (literal do enunciado) | `ERRO: sintatico proximo a "-1" - linha 6` |
| `{DIGITO}+` (implementado) | aceito |

Com a versão sem sinal, `a = -5;` continua válido — é menos unário aplicado à constante.
É a solução padrão: o léxico não decide sinal, o sintático decide.

**Descartado**: usar contexto à direita (*trailing context*) no Flex para aceitar o sinal só
quando não houver operando à esquerda. Funcionaria, mas é um recurso obscuro e difícil de
explicar — e o resultado seria o mesmo.

---

## D13. Fim de arquivo: `0` ou `-1`, tanto faz (verificado)

**Decisão**: não escrever regra `<<EOF>>` na condição inicial; o padrão do Flex devolve `0`.

**Por quê**: a Seção 3.1 do enunciado diz que `yylex()` "deve retornar -1 (EOF) quando o fim
de arquivo é encontrado". Testados os dois, o resultado é idêntico: o código gerado pelo
Bison trata **qualquer valor ≤ 0** como fim de entrada. Uma regra a menos.

Isso não vale para o `<<EOF>>` dentro da *start condition* de comentário, que continua
sendo necessário — é ele que detecta o comentário não terminado.

---

## Nenhuma pendência bloqueante ("NEEDS CLARIFICATION")

Os dois pontos em aberto — formato exato do assembly (D11) e precisão da linha nos nós (D3) —
pertencem a fases posteriores e têm caminho de resolução registrado. As Fases 1 a 3 podem
começar imediatamente.
