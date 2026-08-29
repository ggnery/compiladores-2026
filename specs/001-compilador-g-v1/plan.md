# Plano de Implementação: Compilador G-V1

**Branch**: `001-compilador-g-v1` | **Data**: 2026-08-29 | **Spec**: [spec.md](./spec.md)

**Entrada**: `materials/trabalho1.pdf` + exemplos do professor em `materials/examples/`

## Resumo

Construir, **em cinco fases entregáveis**, um compilador didático em C para a linguagem G-V1,
usando Flex (léxico) e Bison (sintático). Cada fase compila, roda e é demonstrável sozinha:

1. **Léxico + sintático integrados** → reconhece se o programa é válido
2. **AST** → constrói e imprime a árvore
3. **Pilha de tabelas de símbolos** → 4 operações de escopo
4. **Analisador semântico** → percurso na AST checando escopo e tipos
5. **Geração de código** → percurso na AST emitindo assembly MIPS

Abordagem técnica: transcrever a gramática do enunciado **literalmente** (verificado: é
LALR(1) com **0 conflitos**), usar **uma única struct** de nó com 3 filhos para toda a AST, e
**listas encadeadas** para a tabela de símbolos. Nenhuma abstração além do necessário.

## Contexto Técnico

**Linguagem**: C (gcc), sem `-std` especial

**Dependências principais**: Flex 2.6.4, Bison 3.8.2 — nada além disso

**Armazenamento**: N/A (tudo em memória; entrada e saída são arquivos-texto)

**Testes**: arquivos `.g` de teste rodados manualmente contra saída esperada (ver `quickstart.md`)

**Plataforma-alvo**: Linux (ambiente de avaliação do professor); desenvolvimento em macOS

**Tipo de projeto**: compilador de linha de comando, arquivo único de entrada

**Metas de desempenho**: N/A — não há requisito de desempenho; clareza tem prioridade

**Restrições**:
- `make` sozinho tem que produzir o executável `g-v1`
- mensagens de erro com texto exato do enunciado
- ⚠️ **Bison local é 2.3, exigido 3.8.2** → `brew install bison` antes da Fase 1 (ver research.md D10)

**Escala/Escopo**: ~6 arquivos-fonte, na casa de 1000 linhas no total; 5 entregas até 15/10/2026

## Verificação da Constituição

*PORTÃO: precisa passar antes da Fase 0 e ser revisto após a Fase 1.*

| Princípio | Situação | Como o plano atende |
|---|---|---|
| I. Didática acima do código | ✅ | Cada fase entrega código + explicação curta; `data-model.md` documenta o significado de cada campo; comentários em português |
| II. Simplicidade radical | ✅ | 1 struct de nó (não hierarquia), lista encadeada (não hash), gramática literal (não reescrita), sem recuperação de erro |
| III. Entrega em fatias verticais | ✅ | 5 fases, cada uma com `make` + teste correto + teste com erro |
| IV. Fidelidade ao professor | ✅ | Gramática, nomes (`g-v1.l`, `g-v1.y`, `g-v1`), mensagens de erro e estilo de Makefile vindos do enunciado e do `ex2` |
| V. Roda no ambiente de avaliação | ⚠️→✅ | Makefile com `CC`/`BISON`/`FLEX` como variáveis; risco do Bison 2.3 identificado e com solução (`brew install bison`) |

**Resultado: PASSA.** Nenhuma violação a justificar. Revisto após a Fase 1 de design: continua passando.

## Estrutura do Projeto

### Documentação (esta feature)

```text
specs/001-compilador-g-v1/
├── spec.md              # o que precisa existir (requisitos + 5 fases)
├── plan.md              # este arquivo
├── research.md          # decisões técnicas, com o que foi descartado e por quê
├── data-model.md        # a struct do nó da AST e a pilha de tabelas de símbolos
├── quickstart.md        # como compilar, rodar e validar cada fase
├── contracts/
│   ├── tokens.md            # tabela de tokens (entrada do Flex)
│   ├── gramatica-bison.md   # gramática transcrita (entrada do Bison)
│   └── cli-e-erros.md       # uso do executável e texto exato dos erros
└── tasks.md             # gerado depois, por /speckit-tasks
```

### Código-fonte (raiz do repositório)

Layout **plano**, exatamente a lista de arquivos que o professor pede na Seção 8 do enunciado.
Nada de `src/`: o professor roda `make` na raiz da pasta entregue.

```text
Makefile              # gera léxico, gera sintático, compila, liga  -> ./g-v1
build/                # TUDO que é gerado (sintatico.c, lexico.c, g-v1.tab.h, *.o)
g-v1.l                # Flex: tokens, comentários, erros léxicos           [Fase 1]
g-v1.y                # Bison: gramática + ações que constroem a AST  [Fases 1-2]
ast.h / ast.c         # struct do nó, criaNo(), percurso de impressão      [Fase 2]
tabela.h / tabela.c   # pilha de tabelas de símbolos (4 operações)         [Fase 3]
semantico.h / .c      # percurso na AST: escopo + tipos                    [Fase 4]
codigo.h / codigo.c   # percurso na AST: assembly MIPS                     [Fase 5]
testes/
├── ok1.g                  # programa correto
├── erro_lexico1.g         # caractere inválido
├── erro_lexico2.g         # comentário não termina
├── erro_lexico3.g         # cadeia em duas linhas
├── erro_sintatico.g       # falta fimse
└── erro_semantico*.g      # não declarada / tipo errado          [a partir da Fase 4]
materials/            # PDFs e exemplos do professor (não entra na entrega)
```

**Decisão de estrutura**: arquivos na raiz, um par `.h`/`.c` por fase. Assim cada fase adiciona
**um par de arquivos novo** sem mexer nos anteriores, e a lista de entrega do enunciado é
conferida item a item. A `main()` fica no epílogo do `g-v1.y` (como no exemplo do professor) e
cresce uma chamada por fase: `yyparse()` → `imprimeArvore()` → `analisaSemantica()` → `geraCodigo()`.

## Sequência das fases

Cronograma do enunciado (Tabela 2). Hoje é **29/08/2026** — a Fase 1 vence em **2 dias**.

| Fase | Entrega | Prazo | Arquivos |
|---|---|---|---|
| 1 | Léxico + sintático integrados | **31/08/2026** | `g-v1.l`, `g-v1.y`, `Makefile`, `testes/` |
| 2 | Geração da AST | 08/09/2026 | `ast.h`, `ast.c`, ações no `g-v1.y` |
| 3 | Tabela de símbolos | 18/09/2026 | `tabela.h`, `tabela.c` |
| 4 | Analisador semântico | 28/09/2026 | `semantico.h`, `semantico.c` |
| 5 | Geração de código | 14/10/2026 | `codigo.h`, `codigo.c` |
| — | **Entrega na Turing** | **15/10/2026** | pasta compactada |
| — | Apresentação | 15, 19 e 22/10/2026 | saber explicar tudo |

**Dependências**: a ordem é estritamente sequencial. A Fase 3 pode ser escrita e testada em
paralelo à 2 (a tabela de símbolos não depende da AST), mas só é *usada* na Fase 4.

## Rastreamento de Complexidade

> Preencher apenas se a Verificação da Constituição tiver violações.

Nenhuma violação. As três tentações de complexidade foram identificadas e recusadas
explicitamente em `research.md`: hierarquia de tipos de nó (D6), tabela hash (D8) e
recuperação de erros do Bison (D9).
