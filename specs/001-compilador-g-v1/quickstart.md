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
| 1 | `bison --header=build/g-v1.tab.h -o build/sintatico.c g-v1.y` | `build/sintatico.c` e `build/g-v1.tab.h` (números dos tokens) |
| 2 | `flex -o build/lexico.c g-v1.l` | `build/lexico.c` |
| 3 | `gcc -Ibuild -c ...` | os `.o`, dentro de `build/` |
| 4 | `gcc ... -o g-v1` | o executável `g-v1`, na raiz |

### Onde cada coisa fica

```
raiz/                 <- só o que é escrito à mão
├── g-v1.l                 fonte do analisador léxico
├── g-v1.y                 fonte do analisador sintático
├── Makefile
├── testes/
├── g-v1                   <- executável (na raiz: o enunciado pede ./g-v1 teste.g)
└── build/            <- só o que é gerado; pode apagar a qualquer momento
    ├── sintatico.c        gerado pelo Bison
    ├── g-v1.tab.h         gerado pelo Bison
    ├── lexico.c           gerado pelo Flex
    ├── sintatico.o
    └── lexico.o
```

`make clean` apaga a pasta `build/` inteira e o executável.

> O `g-v1.tab.h` gerado pelo Bison é incluído **pelos dois** lados (`.y` e `.l`) — é ele que faz
> léxico e sintático concordarem sobre o valor inteiro de cada token. É por isso que, no
> Makefile, `build/lexico.c` depende de `build/sintatico.c`: o header nasce naquele passo.

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
| caminho feliz | `./g-v1 testes/Outros/esqueleto.g` | `OK: reconheci a palavra principal.` |
| sem argumento | `./g-v1` | mensagem de uso |
| arquivo inexistente | `./g-v1 nao_existe.g` | mensagem de arquivo não aberto |
| erro sintático | arquivo com `principal` duas vezes | `ERRO: syntax error ... - linha 2` |

### Fase 3 — léxico + sintático (US1)

| Teste | Comando | Esperado |
|---|---|---|
| correto | `./g-v1 testes/Outros/ok1.g` | sem mensagem de erro |
| caractere inválido | `./g-v1 testes/Outros/erro_lexico1.g` | `ERRO: CARACTERE INVÁLIDO <linha>` |
| comentário aberto | `./g-v1 testes/Outros/erro_lexico2.g` | `ERRO: COMENTÁRIO NAO TERMINA <linha>` |
| cadeia em 2 linhas | `./g-v1 testes/Outros/erro_lexico3.g` | `ERRO: CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA <linha>` |
| falta `fimse` | `./g-v1 testes/Outros/erro_sintatico.g` | `ERRO: ` e a linha |

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

## 5. Antes de entregar (15/10/2026)

```bash
make clean && make          # tem que funcionar do zero, só com "make"
```

- [ ] `g-v1.l`, `g-v1.y`, tabela de símbolos (+`.h`), semântico (+`.h`), gerador (+`.h`), `Makefile`
- [ ] Testado num Linux (ou com gcc/bison equivalentes aos do professor)
- [ ] Consigo explicar cada arquivo sem consultar
