# Constituição do Projeto — Compilador G-V1

**Versão**: 1.0.0 | **Ratificada em**: 2026-08-29 | **Última alteração**: 2026-08-29

Contexto: trabalho da disciplina *Compiladores e Compiladores 1* (UFG / INF, 2026-2),
Prof. Thierson Couto Rosa. O produto é um compilador didático para a linguagem **G-V1**.

## Princípios Centrais

### I. Didática acima do código (NÃO-NEGOCIÁVEL)

Toda entrega deve ser explicável em voz alta pelo aluno. O critério de aceitação de
qualquer arquivo não é "funciona", e sim "eu consigo explicar cada linha ao professor".
O enunciado é explícito: *"se o aluno não souber explicar o trabalho, recebe nota zero"*.

Na prática: cada arquivo entregue vem acompanhado de uma explicação curta (o que faz,
por que existe, como se liga na fase anterior). Comentários em português, no ponto onde
a dúvida aparece — não comentários decorativos.

### II. Simplicidade radical

Sempre a solução mais simples que atende ao enunciado. Lista encadeada antes de tabela
hash. Struct única antes de hierarquia de tipos. Função de 20 linhas antes de 5 funções
de 4 linhas. Nenhuma abstração é criada "porque pode ser útil depois".

Se duas soluções resolvem o problema, vence a que tem menos conceitos novos.

### III. Entrega em fatias verticais

O trabalho é implementado **em partes**, nunca de uma vez. Cada fase (léxico+sintático,
AST, tabela de símbolos, semântico, geração de código) precisa compilar, rodar e ser
demonstrável sozinha, com `make` e um arquivo de teste. Nunca começar a fase N+1 com a
fase N quebrada.

### IV. Fidelidade ao professor

Nomes de arquivos, formato de mensagens de erro, estrutura do Makefile e estilo de código
seguem o enunciado e os exemplos do professor (`materials/examples/ex1`, `ex2`). Onde o
enunciado define um texto exato de mensagem, o texto é reproduzido caractere por caractere.
Não inventamos formato "melhor".

### V. Roda no ambiente de avaliação

O código é avaliado em **Linux, gcc/g++ 14, Flex 2.6.4, Bison 3.8.2**, e o professor precisa
obter o executável digitando apenas `make`. Nada de dependência externa, nada de script
extra, nada que só funcione no macOS do aluno.

## Restrições Técnicas

- Linguagem: **C** (o enunciado permite C/C++; os exemplos do professor são em C).
- Ferramentas: Flex (analisador léxico) + Bison (analisador sintático), obrigatórias.
- Executável final: `g-v1`, invocado como `./g-v1 arquivo.g`.
- Sem bibliotecas de terceiros.

## Fluxo de Trabalho

1. Uma fase por vez, na ordem do cronograma do enunciado.
2. Cada fase termina com: código compilando + arquivo de teste correto + arquivo de teste
   com erro + explicação escrita curta.
3. Só então a próxima fase começa.

## Governança

Esta constituição prevalece sobre preferências de estilo. Qualquer violação (ex.: uma
estrutura de dados mais complexa que o necessário) deve ser justificada por escrito na
seção *Complexity Tracking* do plano.

Emendas: alterar este arquivo e subir a versão (MAJOR = princípio removido/redefinido,
MINOR = princípio novo, PATCH = redação).
