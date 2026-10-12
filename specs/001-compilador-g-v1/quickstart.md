# Quickstart — como compilar, rodar e validar

## 0. Pré-requisito neste Mac (fazer uma vez)

O Bison do macOS é a **versão 2.3** (de 2008). O trabalho é avaliado com a **3.8.2**, e o
próprio exemplo do professor (`bison -HsimpleLang.h`) não roda na 2.3.

```bash
brew install bison
# usar o bison novo nesta sessão do terminal:
export PATH="/opt/homebrew/opt/bison/bin:$PATH"
bison --version      # precisa dizer 3.8.x
flex --version       # 2.6.4 - o do sistema já serve
```

> Já feito (T001): a linha do `export PATH` está no `~/.zshrc`. Como o Bison do Homebrew é
> *keg-only*, ele não substitui o `/usr/bin/bison` — **abra um terminal novo** antes do
> primeiro `make`.

## 1. Compilar

```bash
make
```

O `Makefile` faz, nesta ordem:

| Passo | Comando | O que produz |
|---|---|---|
| 1 | `bison --defines=build/g-v1.tab.h -o build/sintatico.c src/g-v1.y` e `touch build/g-v1.tab.h` | `build/sintatico.c` e `build/g-v1.tab.h` (números dos tokens) |
| 2 | `flex -o build/lexico.c src/g-v1.l` | `build/lexico.c` |
| 3 | `gcc -Ibuild -Isrc -c build/sintatico.c ...` (idem `lexico.c`) e `gcc -c src/ast.c ...` (idem `tabela.c`, `semantico.c`, `codigo.c`) | os 6 `.o`, dentro de `build/` |
| 4 | `gcc build/sintatico.o build/lexico.o build/ast.o build/tabela.o build/semantico.o build/codigo.o -o g-v1` | o executável `g-v1`, na raiz |

### Onde cada coisa fica

```
raiz/
├── Makefile
├── src/              <- só o que é escrito à mão
│   ├── g-v1.l             analisador léxico (Flex)
│   ├── g-v1.y             analisador sintático + main (Bison)
│   ├── ast.h, ast.c       árvore sintática abstrata
│   ├── tabela.h, tabela.c pilha de tabelas de símbolos
│   ├── semantico.h, semantico.c
│   └── codigo.h, codigo.c gerador de código MIPS
├── testes/
├── docs/                  EXPLICACAO-faseN.md
├── g-v1              <- executável (na raiz: o enunciado pede ./g-v1 teste.g)
└── build/            <- só o que é gerado; pode apagar a qualquer momento
    ├── sintatico.c, g-v1.tab.h   gerados pelo Bison
    ├── lexico.c                  gerado pelo Flex
    ├── *.o                       os 6 objetos
    └── arvore.txt                gravada pelo ./g-v1
```

`make clean` apaga a pasta `build/` inteira e o executável.

> O `g-v1.tab.h` gerado pelo Bison é incluído **pelos dois** lados (`.y` e `.l`) — é ele que faz
> léxico e sintático concordarem sobre o valor inteiro de cada token. É por isso que, no
> Makefile, `build/lexico.c` depende do header, que nasce no passo do Bison.
>
> O `touch` do passo 1 existe porque o Bison grava o `.h` alguns milissegundos **antes** do
> `.c`. Como a regra do `.h` depende do `.c`, no `make` do Linux o `.h` parecia sempre
> desatualizado e o `make` nunca dizia "up to date". O `touch` deixa o `.h` mais novo.

## 2. Rodar

```bash
./g-v1 testes/Outros/ok1.g
```

## 3. Programa de teste (correto)

`testes/Outros/ok1.g`:

```
/* Programa de teste em G-V1: soma os numeros de 1 a 10
   e diz se o resultado e grande ou pequeno.
   Este comentario ocupa mais de uma linha - o que e permitido. */

principal
{ soma, i : int;
  c : car;
}
{
  soma = 0;
  i = 1;

  enquanto (i <= 10)
  {
    soma = soma + i;
    i = i + 1;
  }

  escreva "soma: ";
  escreva soma;
  novalinha;

  se (soma > 50) entao
    escreva "grande";
  senao
    escreva "pequeno";
  fimse
  novalinha;

  c = 'a';
  escreva c;
  novalinha;
}
```

Note a forma do bloco com declarações: **`{declarações} {comandos}`** — dois pares de chaves,
conforme `Bloco : VarSection '{' ListaComando '}'`.

## 4. Roteiro de validação por fase

Cada fase só é considerada pronta quando os testes abaixo passam.

### Fase 2 — fundação (esqueleto) ✅ concluída

| Teste | Comando | Esperado |
|---|---|---|
| build | `make clean && make` | gera `./g-v1` sem erro |
| só `principal` | `./g-v1 testes/Outros/esqueleto.g` | `ERRO: sintatico proximo ao fim do arquivo - linha 1` (o `OK: reconheci a palavra principal.` era da gramática provisória da fundação) |
| sem argumento | `./g-v1` | `Uso correto: ./g-v1 nome_do_arquivo` |
| arquivo inexistente | `./g-v1 nao_existe.g` | `Não foi possível abrir o arquivo nao_existe.g` |
| erro sintático | arquivo com `principal` duas vezes | `ERRO: sintatico proximo a "principal" - linha 2` |

### Fase 3 — léxico + sintático (US1)

| Teste | Comando | Esperado |
|---|---|---|
| correto | `./g-v1 testes/Outros/ok1.g` | sem mensagem de erro |
| caractere inválido | `./g-v1 testes/Outros/erro_lexico1.g` | `ERRO: CARACTERE INVÁLIDO 6` |
| comentário aberto | `./g-v1 testes/Outros/erro_lexico2.g` | `ERRO: COMENTÁRIO NAO TERMINA 4` |
| cadeia em 2 linhas | `./g-v1 testes/Outros/erro_lexico3.g` | `ERRO: CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA 3` |
| falta `fimse` | `./g-v1 testes/Outros/erro_sintatico.g` | `ERRO: sintatico proximo a "novalinha" - linha 8` |
| os 5 de `testes/ErroSintatico/` | `./g-v1 testes/ErroSintatico/<arquivo>.g` | 4 deles acusam a linha **seguinte** à do nome — de propósito; tabela e porquê em `docs/EXPLICACAO-fase1.md`, seção 11 |

Checagem extra desta fase: o `make` não deve imprimir **nenhum** aviso de conflito vindo do
Bison. Se imprimir, a gramática foi transcrita errado (a original tem 0 conflitos).

### Fase 4 — AST (US2)

| Teste | Esperado |
|---|---|
| `./g-v1 testes/Outros/ok1.g` | grava o percurso da árvore em `build/arvore.txt` |
| conferência manual | a árvore corresponde ao programa: `BLOCO` com declarações à esquerda e comandos à direita; `SE_CMD` com três filhos |

### Fase 5 — tabela de símbolos (US3)

| Teste | Esperado |
|---|---|
| `make teste-tabela` | as situações abaixo aparecem com `ok` e o total é `0 falha(s)` |
| blocos aninhados com `x` nos dois | a busca de dentro acha o **interno** |
| `x` só no bloco externo | a busca de dentro acha o **externo** |
| nome inexistente | busca devolve vazio |
| sair do bloco | o escopo do topo é removido |

### Fase 6 — semântico (US4)

| Teste | Esperado |
|---|---|
| `testes/Outros/ok1.g` (e os 5 de `testes/Corretos/`) | `Programa semanticamente correto.`; a árvore gravada traz o tipo de cada expressão |
| `testes/Outros/ok_escopo.g` (sombreamento) | **sem** erro; na árvore, o `x` de dentro é `car` e o de fora, `int` |
| `testes/ErroSemantico/fatorial-VariavelNaoDeclaradaLin19.g` | `ERRO: variavel "i" nao declarada - linha 19` |
| `testes/ErroSemantico/NotaEmConceito-VariavelNaoDeclaradaNoEsopoAtual.g` | `ERRO: variavel "conceito" nao declarada - linha 66` |
| `testes/ErroSemantico/fatorial-VariavelRedeclaradaNoMesmoEscopoLin9.g` | `ERRO: variavel "fatorial" ja declarada neste escopo - linha 9` |
| `testes/ErroSemantico/NotaEmConceito-VariavelDeTipoIncopativelComExpressaoLin19.g` | `ERRO: tipos incompativeis na atribuicao - linha 19` |
| `testes/ErroSemantico/NotaEmConceito-Sub-expressoesDeTiposIncompativeisLin26.g` | `ERRO: operacao relacional exige operandos do mesmo tipo - linha 26` |
| `testes/Outros/erro_semantico1.g` (`c + 1`) | `ERRO: operacao aritmetica exige operandos do tipo int - linha 9` |
| `testes/Outros/erro_semantico2.g` (`&` com `car`) | `ERRO: operacao logica exige operandos do tipo int - linha 8` |
| `testes/Outros/erro_semantico3.g` (condição `car`) | `ERRO: condicao deve ser do tipo int - linha 7` |

### Fase 7 — geração de código (US5)

`./g-v1 teste.g` grava `teste.asm` ao lado do `.g`. Para rodar: `spim -quiet -file teste.asm`
(o SPIM lê um número por linha).

| Teste | Entrada | Saída esperada |
|---|---|---|
| `testes/Outros/ok_codigo.g` | `z` | `z` / `7 3` / `7` / `-2 1` / `ok` / `senao` / `54321` |
| `testes/Outros/ok1.g` | — | `soma: 55` / `grande` / `a` |
| `testes/Outros/ok2.g` | `7`, `3` | `ok-11` |
| `testes/Corretos/fatorial.g` | `5` | `O fatorial de 5 e: 120` |
| `testes/Corretos/PA.g` | `2`, `4`, `3` | `O valor do elemento 4 da progressao e: 11` |
| `testes/Corretos/Soma.g` | `2`, `4`, `3` | `O valor da soma da progressao aritmetica e: 26` |
| `testes/Corretos/NotaEmConceito.g` | `5` / `6` / `8` / `10` | `Conceito: D` / `C` / `B` / `A` |
| `testes/Corretos/SeqOrdenada.g` | `3`, `1`, `2`, `3`, `0` | `ORDENADA` |
| `testes/Outros/ok_borda.g` | — | `5 5` / `1` / `C:\` |

## 5. Antes de entregar (15/10/2026)

```bash
make clean && make          # tem que funcionar do zero, só com "make"
```

- [x] `make clean && make` do zero (T077)
- [x] Lista da Seção 8, agora em `src/`: `g-v1.l`, `g-v1.y`, tabela de símbolos (+`.h`), semântico (+`.h`), gerador (+`.h`), e o `Makefile` na raiz (T080)
- [x] Testado em Linux: Debian trixie, gcc 14.2, flex 2.6.4, bison 3.8.2 (T078)
- [ ] Consigo explicar cada arquivo sem consultar

### Gerar o zip

**Não** compactar a pasta como está: ela tem `build/` e o `g-v1` compilado no Mac. Com eles
no zip, o `make` do professor pode dizer "up to date" e deixar um executável de Mac, que não
roda no Linux. Duas formas certas:

```bash
make clean                                         # 1) apaga build/ e g-v1; depois compactar

git archive --format=zip -o entrega.zip HEAD       # 2) só o que está no git: o .gitignore
                                                   #    já deixa de fora build/, g-v1, *.o, *.asm
unzip -l entrega.zip                               # conferir: sem build/, sem g-v1, sem .o
```

Cuidados com o `git archive`: ele empacota o último **commit** (o que não foi commitado fica de
fora), e leva também `materiais/`, `.claude/` e `.specify/`. Para levar só o trabalho:
`git archive --format=zip -o entrega.zip HEAD Makefile src testes docs`. Para testar a extração
no Mac, use `tar -xf entrega.zip`: o `unzip` do macOS se perde no `ç` de um nome em
`testes/ErroSintatico/`.
