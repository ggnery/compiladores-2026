# Fase 5 — Geração de código MIPS

Roteiro para explicar esta entrega. Arquivos: `src/codigo.h`, `src/codigo.c`, a chamada no
`main` do `src/g-v1.y` e o teste `testes/Outros/ok_codigo.g`.

## 1. O problema

Até a Fase 4 o compilador só **conferia** o programa. Agora ele precisa **traduzi-lo**: escrever
um arquivo `.asm` com instruções MIPS que, rodando no simulador (SPIM ou MARS), fazem o que o
programa G-V1 descreve.

```bash
./g-v1 teste.g                     # grava teste.asm ao lado do teste.g
spim -quiet -file teste.asm        # executa
```

A Seção 7 do enunciado diz como: **outro percurso na AST**, de novo empilhando e desempilhando
escopos, mas agora a tabela de símbolos serve para dar a cada variável **um lugar na memória**.

## 2. O percurso é o mesmo do semântico

`codigo.c` tem o mesmo formato de `semantico.c`, função por função:

| semântico | geração de código |
|---|---|
| `visitaBloco`: empilha, insere, visita comandos, desempilha | `geraBloco`: empilha, insere **e calcula o deslocamento**, gera comandos, desempilha |
| `visitaComando`: um `case` por comando | `geraComando`: um `case` por comando, escrevendo instruções |
| `visitaExpressao`: pós-ordem, devolve o tipo | `geraExpressao`: pós-ordem, deixa o valor em `$a0` |

Como o programa já passou pelo semântico, não há mais nada a conferir: todo `busca` acha a
variável, todo tipo bate.

## 3. Onde as variáveis moram

Três registradores organizam a memória:

- **`$fp`** (*frame pointer*): recebe o valor de `$sp` na primeira instrução e **nunca mais
  muda**. Toda variável é achada a partir dele.
- **`$sp`** (*stack pointer*): o topo da pilha. Desce quando um bloco reserva variáveis ou
  quando uma expressão guarda um valor temporário.
- **`$a0`**: o "acumulador" — onde fica o resultado de cada expressão.

Cada variável recebe um **deslocamento** negativo em relação a `$fp`. A variável `ocupados`
conta quantos bytes já foram reservados; cada declaração soma 4 e pega o valor:

```
programa ok_codigo.g               memória (a pilha cresce para baixo)

{ x, i : int; c : car; }           $fp →  ┌─────────┐
                                          │   x     │  -4($fp)
                                          │   i     │  -8($fp)
                                          │   c     │ -12($fp)
  { x : int; }                            ├─────────┤ ← bloco interno
                                          │   x     │ -16($fp)  (o x de dentro)
                                   $sp →  └─────────┘
```

Na entrada de um bloco com declarações:

```asm
addiu $sp, $sp, -12     # reserva as variáveis do bloco
```

e na saída o contrário (`addiu $sp, $sp, 12`), além de `ocupados -= 12` e `desempilhaEscopo()`.
Por isso blocos irmãos reaproveitam o mesmo espaço — no `ok_codigo.g`, o `d` do `enquanto`
ganha o mesmo `-16` que o `x` interno usou antes —, e um bloco dentro de um `enquanto` reserva
e libera a cada volta, sem a pilha crescer.

O **sombreamento** sai de graça: dentro do bloco interno, `busca("x")` acha primeiro o `x` do
topo, com deslocamento `-16`; fora, acha o de baixo, com `-4`. Os dois `x` são posições
diferentes na memória, e o código gerado usa a certa em cada lugar.

## 4. Expressões: pós-ordem com pilha

`geraExpressao` deixa o valor da expressão em `$a0`. Para um operador binário:

1. gera o filho da esquerda → valor em `$a0`;
2. **empilha** `$a0` (o filho da direita vai sobrescrever `$a0`);
3. gera o filho da direita → valor em `$a0`;
4. **desempilha** o da esquerda para `$t1`;
5. faz a operação: `addu $a0, $t1, $a0`.

É a mesma pós-ordem do semântico: o operador só pode ser calculado quando os dois operandos
já foram. `soma + i` vira:

```asm
lw $a0, -4($fp)       # soma            (1)
addiu $sp, $sp, -4    #                 (2) empilha
sw $a0, 0($sp)
lw $a0, -8($fp)       # i               (3)
lw $t1, 0($sp)        #                 (4) desempilha
addiu $sp, $sp, 4
addu $a0, $t1, $a0    #                 (5)
```

A pilha é necessária porque expressões se aninham: em `a * (b + c)`, enquanto se calcula
`b + c`, o valor de `a` tem de estar guardado em algum lugar que a subexpressão não estrague.

| nó | instrução |
|---|---|
| `INT_CONST`, `CAR_CONST` | `li $a0, valor` (o `car` vira o código ASCII: `'a'` → 97) |
| `ID` | `lw $a0, desl($fp)` |
| `x = e` | gera `e`, depois `sw $a0, desl($fp)` |
| `+ - * /` | `addu`, `subu`, `mul`, `div` |
| `< > <= >= == !=` | `slt`, `sgt`, `sle`, `sge`, `seq`, `sne` — dão 1 ou 0 |
| `-a` | `subu $a0, $zero, $a0` |
| `!a` | `seq $a0, $a0, $zero` (1 se `a` era 0) |
| `a & b`, `a \|\| b` | curto-circuito (seção 6) |

## 5. Comandos

**`se` e `enquanto`** viram desvios. Cada um pede rótulos novos ao contador `rotulos`
(`L0`, `L1`, ...), para dois `se` nunca usarem o mesmo nome:

```
se (cond) entao A senao B fimse         enquanto (cond) C

    <cond em $a0>                       L0: <cond em $a0>
    beqz $a0, L1      # falso: senão        beqz $a0, L1      # falso: sai
    <A>                                     <C>
    j L2                                    j L0              # volta ao teste
L1: <B>                                 L1:
L2:
```

`beqz` = "desvia se igual a zero": é aqui que o "0 é falso" do enunciado vira instrução.

**Entrada e saída** usam `syscall`: põe-se o número do serviço em `$v0` e o argumento em `$a0`.

| comando | serviço |
|---|---|
| `escreva` de `int` | 1 (escreve inteiro) |
| `escreva` de `car` | 11 (escreve caractere) |
| `escreva "texto"` | 4 (escreve cadeia; o texto vai para `.data`) |
| `leia` de `int` | 5 (lê inteiro; volta em `$v0`, depois `sw`) |
| `leia` de `car` | 12 (lê caractere) |
| `novalinha` | 11 com `$a0 = 10` (o `'\n'`) |
| fim do programa | 10 |

Como o `escreva` sabe se imprime número ou caractere? Pelo **tipo anotado no nó** pela Fase 4.
O `x` de `escreva x` é só um `ID`; quem sabe que ele é `car` é o `no->tipo` preenchido pelo
semântico.

## 6. Decisões

**Toda variável ocupa 4 bytes, inclusive `car`.** O enunciado diz que o tamanho é indicado pelo
tipo, e a função `tamanho()` existe para isso. Mas um `car` de 1 byte seguido de um `int`
deixaria o `int` num endereço que não é múltiplo de 4, e `lw`/`sw` exigem isso. Com tudo em
palavras não há alinhamento a calcular. Para mudar, basta `tamanho()` devolver 1 para `car` e
usar `lb`/`sb`.

**Endereços a partir de `$fp`, não de `$sp`.** O `$sp` se move durante as expressões; se as
variáveis fossem achadas a partir dele, o deslocamento de `x` mudaria conforme houvesse
valores empilhados. O `$fp` fica parado.

**`&` e `||` com curto-circuito, como em C.** Em `a & b`, se `a` é 0 o resultado já é 0, e `b`
nem é calculado (desvia direto para o fim). O teste `ok_codigo.g` usa
`x > 100 & x / 0 == 1`: sem curto-circuito, a divisão por zero pararia o programa.

**As cadeias de `escreva` vão para `.data` na hora em que aparecem.** O gerador troca para
`.data`, escreve a cadeia com um rótulo (`S2: .asciiz "soma: "`), volta para `.text` e usa o
rótulo. O montador junta as partes de cada seção — não é preciso uma segunda passada só para
coletar as cadeias.

**O nome do `.asm` sai do nome do `.g`.** `testes/Outros/ok1.g` → `testes/Outros/ok1.asm`. A
geração só acontece se o semântico passou; com erro, nenhum `.asm` é criado.

**Cada comando leva um comentário `# linha N`.** Não muda nada na execução, mas deixa achar no
`.asm` o trecho de cada linha do fonte.

## 7. Como demonstrar

```bash
make
./g-v1 testes/Outros/ok_codigo.g
printf "z\n" | spim -quiet -file testes/Outros/ok_codigo.asm
#   z
#   7 3        <- x de fora, x de dentro
#   7          <- de volta ao x de fora
#   -2 1
#   ok         <- curto-circuito: o x / 0 não foi calculado
#   senao
#   54321

./g-v1 testes/Corretos/fatorial.g
printf "5\n" | spim -quiet -file testes/Corretos/fatorial.asm
#   O fatorial de 5 e: 120
```

Os resultados de todos os programas corretos estão na tabela da Fase 7 de
`specs/001-compilador-g-v1/quickstart.md`.

## 8. Perguntas que podem cair

**Por que refazer a pilha de escopos, se o semântico já fez?** O semântico desempilha cada
escopo ao sair do bloco; quando ele termina, a pilha está vazia. E é preciso saber, em cada
ponto do programa, qual `x` está valendo — exatamente o que o empilha/desempilha durante o
percurso dá. O enunciado pede isso na Seção 7.

**O que é o deslocamento?** A distância, em bytes, entre `$fp` e a variável. `lw $a0, -8($fp)`
lê a palavra que está 8 bytes abaixo de `$fp`.

**Por que o deslocamento é negativo?** A pilha do MIPS cresce para endereços menores. As
variáveis ficam abaixo de onde `$fp` aponta.

**Por que a expressão precisa de pilha, se há vários registradores?** Com um número fixo de
registradores, uma expressão aninhada o bastante esgotaria todos. A pilha não tem esse limite,
e o algoritmo fica o mesmo para qualquer expressão.

**Por que a pós-ordem?** Não dá para somar antes de ter os dois números. Os filhos são gerados
antes do operador — a mesma razão pela qual o semântico confere os tipos dos filhos antes.

**O que acontece com `x = y = 1`?** O `ATRIB` de dentro guarda 1 em `y` e deixa 1 em `$a0`; o
de fora guarda esse `$a0` em `x`. A atribuição é uma expressão e tem valor.

**Por que a pilha não estoura num `enquanto` com bloco que declara variáveis?** O bloco reserva
na entrada e libera na saída, e o `j` de volta ao teste acontece depois da liberação. Cada volta
termina com `$sp` onde começou.

**`seq`, `sgt`, `mul`... são instruções do MIPS?** Algumas são *pseudo-instruções*: o montador
do SPIM/MARS as traduz em duas ou três instruções reais. Deixam o código gerado mais curto de
ler.
