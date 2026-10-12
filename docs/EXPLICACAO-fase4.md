# Fase 4 — Análise semântica

Roteiro para explicar esta entrega. Arquivos: `semantico.h`, `semantico.c`, a chamada no `main`
do `g-v1.y` e os testes `testes/Outros/erro_semantico1..3.g` e `testes/Outros/ok_escopo.g`.

## 1. O problema

A Fase 1 confere a **forma** do programa; esta fase confere o **sentido**. Este trecho de um
teste do professor é sintaticamente perfeito:

```
{ fatorial, n : int; }
{
    ...
    i = n - 1;          /* linha 19: i nunca foi declarado */
```

A gramática não tem como recusá-lo: "declarar antes de usar" e "não misturar `int` com `car`"
dependem de **lembrar** o que apareceu antes, e uma gramática livre de contexto não lembra.
Quem lembra é a tabela de símbolos da Fase 3, consultada durante um percurso na árvore da Fase 2.

## 2. O percurso

`analisaSemantica(raiz)` inicia a pilha e visita o bloco principal. Três funções recursivas,
uma para cada tipo de subárvore:

| função | recebe | faz |
|---|---|---|
| `visitaBloco` | um `BLOCO` | abre escopo, insere as declarações, visita os comandos, fecha escopo |
| `visitaComando` | um comando | um `case` por espécie; bloco aninhado volta para `visitaBloco` |
| `visitaExpressao` | uma expressão | confere os tipos e devolve o tipo da expressão |

O `BLOCO` é onde a árvore e a pilha de escopos se encontram:

```
BLOCO                      → empilhaEscopo()             (só se o bloco tem declarações)
├── LISTA_DECL  (filho1)   → insere(nome, tipo, linha)   NULL = "ja declarada"
└── LISTA_CMD   (filho2)   → ... busca(nome) ...         NULL = "nao declarada"
                           → fim do bloco: desempilhaEscopo()
```

A ordem importa: `filho1` antes de `filho2`. Como as declarações estão sempre à esquerda (regra
que a Fase 2 garantiu), todo nome já foi inserido quando algum comando o usa.

E como um bloco aninhado é um comando, `visitaBloco` chama a si mesma (passando por
`visitaComando`): o bloco de dentro empilha o seu escopo **em cima** do de fora e desempilha ao
terminar. Os pares empilha/desempilha se aninham exatamente como as chaves do programa — é assim
que a pilha acompanha os blocos durante o percurso.

## 3. Escopo

- **Não declarada**: `busca` percorre a pilha do topo para a base e devolve `NULL` → erro, com
  a linha do **uso**.
- **Redeclarada**: `insere` só olha o escopo do topo e devolve `NULL` se o nome já está lá →
  erro, com a linha da **segunda** declaração (a primeira era legítima).
- **Sombreamento não é erro**: o mesmo nome num bloco de dentro vai para outra tabela, e a
  `busca` acha primeiro o de dentro. O `ok_escopo.g` demonstra: na árvore gravada, o `x` do
  bloco interno sai `car` e o de fora, `int`.

No teste `NotaEmConceito-VariavelNaoDeclaradaNoEsopoAtual.g`, `conceito` é declarado só dentro
dos blocos do `se`. Na linha 66, depois do `fimse`, esses escopos já foram desempilhados — por
isso `conceito` não existe ali.

## 4. Tipos

`visitaExpressao` percorre em **pós-ordem**: primeiro os operandos (`filho1`, `filho2`), depois
o próprio nó. Quando chega ao operador, já sabe o tipo de cada lado e aplica a regra:

| construção | regra | tipo do resultado |
|---|---|---|
| `INT_CONST`, `CAR_CONST` | — | `int`, `car` |
| `ID` | tem de estar declarado | o da declaração |
| `x = e` | tipo de `e` igual ao de `x` | o de `x` |
| `+ - * /` e o `-` unário | operandos `int` | `int` |
| `< > <= >= == !=` | operandos do **mesmo** tipo | `int` |
| `\|\|` `&` `!` | operandos `int` | `int` |
| condição de `se` / `enquanto` | tem de ser `int` | — |

Não existe tipo lógico em G-V1: comparações e operações lógicas dão `int` (1 é verdadeiro,
0 é falso), como no C. `escreva` aceita `int` e `car`; `leia` só exige a variável declarada.

O tipo calculado fica **anotado no nó** (`no->tipo`) — o campo existia desde a Fase 2 esperando
por isto. O `main` agora grava a árvore **depois** do semântico, então `build/arvore.txt` mostra
o tipo de cada expressão. No teste do professor da linha 26, `se(nota<'7')`, quando o percurso
chega ao `MENOR_OP` os dois filhos já estão anotados:

```
MENOR_OP  (linha 26)          ← int < car: "operacao relacional exige operandos do mesmo tipo"
├── ID nota : int
└── CAR_CONST '7' : car
```

A regra da condição fica numa função pequena, `visitaCondicao`, usada pelo `se` e pelo `enquanto`.

## 5. As mensagens

Todas saem por `erroSemantico`, no mesmo formato do erro sintático — `ERRO:`, a descrição,
` - linha N` —, e a compilação termina (`exit(1)`), como pede a Seção 6 do enunciado.

| mensagem | teste que dispara |
|---|---|
| `ERRO: variavel "i" nao declarada - linha 19` | `ErroSemantico/fatorial-VariavelNaoDeclaradaLin19.g` |
| `ERRO: variavel "conceito" nao declarada - linha 66` | `ErroSemantico/NotaEmConceito-VariavelNaoDeclaradaNoEsopoAtual.g` |
| `ERRO: variavel "fatorial" ja declarada neste escopo - linha 9` | `ErroSemantico/fatorial-VariavelRedeclaradaNoMesmoEscopoLin9.g` |
| `ERRO: tipos incompativeis na atribuicao - linha 19` | `ErroSemantico/NotaEmConceito-VariavelDeTipoIncopativelComExpressaoLin19.g` |
| `ERRO: operacao relacional exige operandos do mesmo tipo - linha 26` | `ErroSemantico/NotaEmConceito-Sub-expressoesDeTiposIncompativeisLin26.g` |
| `ERRO: operacao aritmetica exige operandos do tipo int - linha 9` | `Outros/erro_semantico1.g` |
| `ERRO: operacao logica exige operandos do tipo int - linha 8` | `Outros/erro_semantico2.g` |
| `ERRO: condicao deve ser do tipo int - linha 7` | `Outros/erro_semantico3.g` |

Os quatro testes do professor que trazem a linha no nome do arquivo acusam exatamente essa
linha. A linha vem do nó onde o erro foi detectado — do uso, da declaração ou do operador —, a
mesma que a Fase 2 guardou em cada nó.

## 6. Decisões

**Pós-ordem nas expressões.** Não há como conferir um operador sem saber o tipo dos dois lados.
Consequência: em `c = c + z`, com `z` não declarado, o erro acusado é o do `z`, encontrado na
descida, e não o do `+`.

**O tipo fica anotado no nó.** Conferir e esquecer bastaria para achar erros. Anotar custa uma
linha por caso e deixa a árvore mostrando o resultado da análise — e a geração de código vai
precisar dele, por exemplo para saber se `escreva` imprime um número ou um caractere.

**Condição de `se`/`enquanto` tem de ser `int`.** O enunciado diz que expressão lógica ou
relacional recebe tipo `int` e que a condição é testada contra zero; um `car` como condição
seria usar caractere como valor lógico. É uma interpretação (já registrada na spec, História 4):
se o professor aceitar `car` ali, basta tirar o `if` do `visitaCondicao`.

**Só bloco com declarações abre escopo.** É o que o enunciado descreve nas Seções 5 e 7 ("cada
bloco que possui uma declaração de variáveis locais inicia um escopo novo"). Empilhar um escopo
vazio não mudaria nenhuma busca, mas seria uma operação a mais para explicar.

**A pilha é iniciada dentro de `analisaSemantica`.** O percurso começa sempre com a pilha
vazia, sem depender de quem o chama.

**No primeiro erro, a compilação termina** — igual aos erros léxicos e sintáticos. Como o
`main` só grava a árvore depois do semântico, um programa com erro semântico não gera
`build/arvore.txt`.

## 7. Como demonstrar

```bash
make
./g-v1 testes/Outros/ok1.g
#   Programa sintaticamente correto.
#   Programa semanticamente correto.
#   Árvore gravada em build/arvore.txt       <- agora com o tipo de cada expressão

./g-v1 testes/ErroSemantico/fatorial-VariavelNaoDeclaradaLin19.g
#   Programa sintaticamente correto.
#   ERRO: variavel "i" nao declarada - linha 19

./g-v1 testes/Outros/ok_escopo.g            # sombreamento: passa sem erro
grep "ID x" build/arvore.txt                # o x de dentro é car, o de fora é int
```

## 8. Perguntas que podem cair

**Por que isso não é feito pela gramática?** Porque depende de contexto: saber se `i` foi
declarado exige lembrar das declarações anteriores, e o Bison só reconhece a forma.

**Por que não conferir durante o parse, nas ações do Bison?** O enunciado pede um percurso na
AST, e separar as fases deixa cada uma simples: o parser só monta a árvore; o semântico só a lê.

**Como a pilha fica sincronizada com os blocos?** Cada `visitaBloco` empilha ao entrar e
desempilha ao sair. Um bloco aninhado é visitado por uma chamada recursiva, que termina — e
desempilha — antes de a de fora continuar. Quando ela volta, o topo é de novo o bloco de fora.

**Se o programa tem dois erros, qual aparece?** O primeiro que o percurso encontra: de cima
para baixo e, dentro de cada construção, da esquerda para a direita. A compilação para nele.

**Por que `x = y = 1;` é válido?** A atribuição é uma expressão (`Expr : IDENTIFICADOR '=' Expr`)
e tem tipo — o da variável. Então pode ser o lado direito de outra atribuição.

**O que o `default` do `visitaComando` recebe?** Uma expressão usada como comando, como
`x = 1;`: a regra `Comando : Expr ';'` não cria nó de comando, o `ATRIB` entra direto na lista.

**E o `default` do `visitaExpressao`?** Os operadores relacionais. Todas as outras espécies de
expressão têm `case` próprio.

**Por que `c == 'a'` pode ser escrito, se o resultado de comparar dois `car` não é `car`?**
Porque o resultado é `int` (1 ou 0), e `escreva` aceita `int`.
