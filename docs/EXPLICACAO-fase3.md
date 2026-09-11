# Fase 3 — Tabela de símbolos

Roteiro para explicar esta entrega. Arquivos: `tabela.h`, `tabela.c` e o teste
`testes/Tabela/teste_tabela.c`.

## 1. O problema

A análise semântica (Fase 4) precisa responder, para cada nome usado: **foi declarado? com que
tipo?** E a linguagem permite o mesmo nome em blocos aninhados:

```
principal
{ x, y : int; }
{
    { x : car; }
    {
        x = 'a';        /* o x car, de dentro */
        y = 1;          /* o y int, de fora   */
    }
    x = 2;              /* o x int de novo    */
}
```

Uma tabela só não resolve: ao entrar no bloco de dentro, o `x : car` precisa **esconder** o
`x : int`; ao sair dele, o `x : int` precisa **voltar**.

## 2. A ideia: uma pilha de tabelas

Cada bloco com declarações é um **escopo**, com a sua própria lista de variáveis. Os escopos
formam uma pilha que acompanha o aninhamento dos blocos:

- entrou num bloco → **empilha** um escopo;
- saiu do bloco → **desempilha**;
- procurou um nome → busca do **topo para a base**: o escopo mais de dentro é olhado primeiro.

É essa ordem de busca que faz a variável de dentro sobrepor a de fora — e isso não é erro.
Erro é declarar o mesmo nome duas vezes **no mesmo escopo**.

## 3. A estrutura (`tabela.h`)

Duas listas encadeadas. No meio do exemplo acima, a pilha fica assim:

```
topo → [escopo 2]  entradas: x → NULL
           │ abaixo
           ▼
       [escopo 1]  entradas: y → x → NULL
           │ abaixo
           ▼
         NULL
```

- `Entrada` é uma variável: `lexema`, `tipo`, `linha` da declaração e `deslocamento` (a posição
  na memória, que só a geração de código vai usar). O `prox` liga as variáveis do mesmo escopo.
- `Escopo` é um bloco. O `abaixo` aponta para o escopo de fora — é isso que forma a pilha.

O ponteiro `topo` é `static` dentro do `tabela.c`: o resto do programa só mexe na pilha pelas
funções.

## 4. As operações

| enunciado | função | o que faz |
|---|---|---|
| a) iniciar a pilha | `iniciaPilha()` | deixa `topo = NULL` |
| b) empilhar um escopo | `empilhaEscopo()` | cria um escopo vazio, que aponta `abaixo` para o topo atual e vira o topo |
| c) pesquisar um nome | `busca(lexema)` | percorre os escopos do topo para a base e devolve a primeira entrada com esse nome, ou `NULL` |
| d) remover o escopo atual | `desempilhaEscopo()` | libera as variáveis do topo e o próprio escopo; o topo volta a ser o de fora |
| — | `insere(lexema, tipo, linha)` | põe a variável no topo; devolve `NULL` se o nome já existe **nesse** escopo |
| — | `imprimePilha()` | mostra os escopos, do topo para a base, para a apresentação |

## 5. Decisões

**Listas encadeadas, não tabela hash.** Um bloco tem poucas variáveis, e a busca linear é
instantânea nessa escala. Se um dia for preciso trocar por hash, só o `tabela.c` muda: o
`tabela.h` continua o mesmo.

**A variável nova entra no começo da lista.** Ligar o nó é um passo só, sem andar até o fim;
antes disso o `insere` ainda percorre o escopo do topo, para checar redeclaração. O efeito
visível dessa escolha é a ordem: o `imprimePilha` mostra a mais recente primeiro (`y` antes de `x`).

**O lexema não é copiado nem liberado.** Ele pertence ao nó da árvore; a tabela só guarda o
ponteiro.

**`insere` sem escopo aberto aborta.** Isso só aconteceria por erro no próprio compilador, não no
programa G-V1.

## 6. Como demonstrar

```bash
make teste-tabela
```

O teste simula os blocos do exemplo da seção 1, confere cada busca e mostra a pilha a cada passo:

```
--- pilha de escopos (topo primeiro) ---
escopo 2:  x (car, linha 4)
escopo 1:  y (int, linha 2)  x (int, linha 2)
  ok    x nos dois blocos: a busca acha o interno
  ok    y só no externo: a busca acha o externo
  ok    nome inexistente: a busca devolve NULL
...
0 falha(s)
```

## 7. Como a Fase 4 vai usar

O analisador semântico percorre a árvore e conversa com a tabela a cada nó:

```
BLOCO                         → empilhaEscopo()
├── LISTA_DECL
│   └── DECL x : int          → insere("x", int, linha)   NULL = redeclaração
└── LISTA_CMD
    └── ... ID x ...          → busca("x")                NULL = não declarada
                              → fim do BLOCO: desempilhaEscopo()
```

A regra da árvore — declarações à esquerda do `BLOCO`, comandos à direita — é o que garante que
o `insere` de um nome acontece antes do `busca` dele.

## 8. Perguntas que podem cair

**Por que a busca vai do topo para a base?** Porque o topo é o bloco mais de dentro, e a
declaração mais próxima do uso é a que vale.

**Por que o `insere` só olha o escopo do topo?** Porque a regra de redeclaração é por escopo:
repetir o nome no mesmo bloco é erro, repetir num bloco de dentro é permitido.

**O que acontece com a memória ao desempilhar?** As entradas e o escopo são liberados. O lexema
fica, porque pertence à árvore.

**Por que o `topo` é `static`?** Para ninguém fora do `tabela.c` mexer na pilha sem passar pelas
funções — assim a pilha não fica inconsistente.
