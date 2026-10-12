/* codigo.c - gerador de código assembly MIPS da linguagem G-V1
   Última etapa: fonte -> tokens -> árvore -> semântico -> MIPS (Seção 7 do enunciado).
   Entra a árvore já aprovada pelo semântico, com o tipo de cada expressão em no->tipo; sai o .asm
   que roda no SPIM/MARS. É um novo percurso na árvore, com a mesma forma do semântico.
   Registradores: $fp fixo (base das variáveis), $sp topo da pilha, $a0 valor da expressão. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codigo.h"
#include "tabela.h"

static FILE* saida;        /* arquivo .asm sendo escrito */
static int   rotulos;      /* contador que deixa cada rótulo único: L0, L1, ... (e os S das cadeias) */
static int   ocupados;     /* bytes de variáveis já reservados abaixo de $fp pelos blocos abertos */

/* Uma função por nível da árvore, como no semântico; static porque só geraCodigo sai no
   codigo.h. Declaradas antes porque se chamam umas às outras (um comando pode ser um bloco). */
static void geraBloco(No* bloco);
static void geraComando(No* no);
static void geraExpressao(No* no);

/* Espaço da variável, "indicado pelo seu tipo" (Seção 7).
   Cada variável ocupa uma palavra (4 bytes). O car caberia num byte, mas lw/sw exigem
   endereço múltiplo de 4; com tudo em palavras não há alinhamento a calcular. */
static int tamanho(Tipo tipo) {
    return 4;                                      /* int e car */
}

static int novoRotulo(void) {
    return rotulos++;
}

/* Guarda $a0 no topo da pilha / tira o topo da pilha para $t1: é onde o operando
   da esquerda espera enquanto o da direita é calculado. A pilha cresce para baixo,
   4 bytes por valor. */
static void empilha(void) {
    fprintf(saida, "\taddiu $sp, $sp, -4\n\tsw $a0, 0($sp)\n");
}

static void desempilha(void) {
    fprintf(saida, "\tlw $t1, 0($sp)\n\taddiu $sp, $sp, 4\n");
}

/* Código ASCII de um CARCONST, que vem do léxico com os apóstrofos: "'a'" -> 97, "'\n'" -> 10.
   O li precisa do número; os escapes valem o mesmo que em C ('\a' 7, '\b' 8, '\f' 12, '\v' 11). */
static int codigoCaractere(const char* lexema) {
    if (lexema[1] != '\\') return lexema[1];       /* lexema[0] é o apóstrofo */
    switch (lexema[2]) {
    case 'n': return '\n';
    case 't': return '\t';
    case 'r': return '\r';
    case '0': return '\0';
    case 'a': return '\a';
    case 'b': return '\b';
    case 'f': return '\f';
    case 'v': return '\v';
    default:  return lexema[2];                    /* \\  \'  \" (e escapes desconhecidos) viram o próprio caractere */
    }
}

/* Escreve a cadeia (com as aspas, como veio do léxico) na seção .data; escapes
   no meio, como \n, o próprio montador interpreta.
   Uma barra logo antes da aspa final faria o montador ler \" como aspa escapada (o SPIM
   escreveria C:" em vez de C:\): essas barras saem à parte, como bytes (92 = '\\'). */
static void escreveCadeia(const char* lexema) {
    int fim    = strlen(lexema) - 1;                 /* posição da aspa final */
    int barras = 0;

    /* conta as barras coladas na aspa final, sem chegar à aspa inicial (posição 0) */
    while (fim - barras - 1 > 0 && lexema[fim - barras - 1] == '\\') barras++;

    if (barras == 0) {                               /* o caso comum: a cadeia inteira */
        fprintf(saida, ".asciiz %s\n", lexema);
        return;
    }
    fprintf(saida, ".ascii %.*s\"\n", fim - barras, lexema);   /* "C:\" -> .ascii "C:" (sem terminador) */
    while (barras--) fprintf(saida, "\t.byte 92\n");
    fprintf(saida, "\t.byte 0\n");                  /* o terminador que o .asciiz poria */
}

/* Chamada pelo main só se o semântico passou. Abre o .asm, gera o bloco principal
   a partir do rótulo main e termina com a syscall 10, que encerra o programa. */
void geraCodigo(No* raiz, const char* nomeArquivo) {
    saida = fopen(nomeArquivo, "w");
    if (!saida) { printf("Não foi possível criar o arquivo %s\n", nomeArquivo); exit(1); }

    rotulos  = 0;
    ocupados = 0;
    iniciaPilha();                                   /* a pilha de escopos é refeita neste percurso (Seção 7) */

    fprintf(saida, "# Gerado pelo compilador G-V1\n");
    fprintf(saida, "\t.text\n\t.globl main\nmain:\n");   /* o SPIM entra por jal main; .globl deixa o rótulo visível fora do arquivo */
    fprintf(saida, "\tmove $fp, $sp\t\t# $fp fica parado: as variáveis são achadas a partir dele\n");

    geraBloco(raiz->filho1);                         /* PROGRAMA tem um filho só: o bloco principal */

    fprintf(saida, "\tli $v0, 10\t\t# fim do programa\n\tsyscall\n");
    fclose(saida);
}

/* Mesmo percurso do semântico: declarações (filho1) e depois comandos (filho2).
   Agora a tabela serve para dar a cada variável o seu lugar na memória.
   Na saída o espaço é devolvido: blocos irmãos reaproveitam os mesmos deslocamentos. */
static void geraBloco(No* bloco) {
    No*      p;
    Entrada* var;
    int      tamBloco = 0;

    if (bloco->filho1) empilhaEscopo();              /* só bloco com declarações abre escopo (Seção 7) */

    for (p = bloco->filho1; p; p = p->filho2) {      /* LISTA_DECL: um DECL por nome */
        var = insere(p->filho1->lexema, p->filho1->tipo, p->filho1->linha);   /* nunca NULL: o semântico barrou nome repetido */
        ocupados += tamanho(var->tipo);              /* soma antes: 0($fp) já é o topo ocupado, a 1ª variável fica em -4 */
        tamBloco += tamanho(var->tipo);
        var->deslocamento = -ocupados;               /* negativo (a pilha cresce para baixo): a variável mora em deslocamento($fp) */
    }
    /* $sp desce junto, para os valores que as expressões empilham caírem abaixo das variáveis */
    if (tamBloco)
        fprintf(saida, "\taddiu $sp, $sp, -%d\t\t# reserva as variáveis do bloco (linha %d)\n", tamBloco, bloco->linha);

    for (p = bloco->filho2; p; p = p->filho2)        /* LISTA_CMD: um comando por nó */
        geraComando(p->filho1);

    if (tamBloco) {                                  /* tamBloco > 0 só se houve declaração, isto é, se empilhou */
        fprintf(saida, "\taddiu $sp, $sp, %d\t\t# libera as variáveis do bloco\n", tamBloco);
        ocupados -= tamBloco;
        desempilhaEscopo();
    }
}

/* Um case por comando, como visitaComando no semântico, agora escrevendo instruções.
   Entrada e saída usam syscall: número do serviço em $v0, argumento em $a0. */
static void geraComando(No* no) {
    int senao, fim, inicio;

    if (!no) return;                                   /* NULL: comando vazio (;) ou se sem senão */

    fprintf(saida, "\t\t\t\t# linha %d\n", no->linha);  /* só para achar no .asm o trecho de cada linha do fonte */
    switch (no->especie) {
    case BLOCO:
        geraBloco(no);
        break;
    case SE_CMD:                                       /* condição falsa (0) pula o "então" */
        senao = novoRotulo();
        fim   = novoRotulo();
        geraExpressao(no->filho1);
        fprintf(saida, "\tbeqz $a0, L%d\n", senao);
        geraComando(no->filho2);
        fprintf(saida, "\tj L%d\n", fim);              /* o "então" não cai no "senão" */
        fprintf(saida, "L%d:\n", senao);
        geraComando(no->filho3);                       /* NULL se não há senão: não gera nada */
        fprintf(saida, "L%d:\n", fim);
        break;
    case ENQUANTO_CMD:                                 /* testa no início; o corpo volta para o teste */
        inicio = novoRotulo();
        fim    = novoRotulo();
        fprintf(saida, "L%d:\n", inicio);
        geraExpressao(no->filho1);
        fprintf(saida, "\tbeqz $a0, L%d\n", fim);
        geraComando(no->filho2);
        fprintf(saida, "\tj L%d\n", inicio);
        fprintf(saida, "L%d:\n", fim);
        break;
    case LEIA_CMD:                                     /* syscall 5 lê int, 12 lê car; o valor volta em $v0 */
        fprintf(saida, "\tli $v0, %d\n\tsyscall\n", no->filho1->tipo == TIPO_INT ? 5 : 12);
        fprintf(saida, "\tsw $v0, %d($fp)\n", busca(no->filho1->lexema)->deslocamento);
        break;
    case ESCREVA_CMD:                                  /* syscall 1 escreve int, 11 escreve car; o tipo veio do semântico */
        geraExpressao(no->filho1);
        fprintf(saida, "\tli $v0, %d\n\tsyscall\n", no->filho1->tipo == TIPO_INT ? 1 : 11);
        break;
    case ESCREVA_STR:                                  /* o texto vai para .data; syscall 4 escreve */
        inicio = novoRotulo();
        fprintf(saida, "\t.data\nS%d:\t", inicio);
        escreveCadeia(no->lexema);
        fprintf(saida, "\t.text\n\tla $a0, S%d\n\tli $v0, 4\n\tsyscall\n", inicio);   /* o montador junta os pedaços de cada seção */
        break;
    case NOVALINHA_CMD:                                /* escreve o car 10 ('\n') com a syscall 11 */
        fprintf(saida, "\tli $a0, 10\n\tli $v0, 11\n\tsyscall\n");
        break;
    default:                                           /* expressão usada como comando: x = 1; */
        geraExpressao(no);
    }
}

/* Deixa o valor da expressão em $a0 (no semântico, visitaExpressao devolvia o tipo).
   Pós-ordem: calcula o operando da esquerda e guarda na pilha, calcula o da
   direita, tira o da esquerda para $t1 e só então faz a operação. */
static void geraExpressao(No* no) {
    int fim;

    switch (no->especie) {
    case INT_CONST:                                    /* lexema sem sinal: +5 chega como "5"; -5 é NEG_OP */
        fprintf(saida, "\tli $a0, %d\n", atoi(no->lexema));
        return;
    case CAR_CONST:
        fprintf(saida, "\tli $a0, %d\t\t# %s\n", codigoCaractere(no->lexema), no->lexema);
        return;
    case ID:                                           /* busca acha o x do escopo mais interno (sombreamento); nunca NULL: o semântico conferiu */
        fprintf(saida, "\tlw $a0, %d($fp)\t\t# %s\n", busca(no->lexema)->deslocamento, no->lexema);
        return;
    case ATRIB:                                        /* x = e: o valor de e continua em $a0 */
        geraExpressao(no->filho2);
        fprintf(saida, "\tsw $a0, %d($fp)\t\t# %s = ...\n", busca(no->filho1->lexema)->deslocamento, no->filho1->lexema);
        return;
    case NEG_OP:                                       /* -a = 0 - a */
        geraExpressao(no->filho1);
        fprintf(saida, "\tsubu $a0, $zero, $a0\n");
        return;
    case NAO_OP:                                       /* !a: 1 se a == 0, senão 0 */
        geraExpressao(no->filho1);
        fprintf(saida, "\tseq $a0, $a0, $zero\n");
        return;
    case OU_OP: case E_OP:                             /* curto-circuito, como em C */
        fim = novoRotulo();
        geraExpressao(no->filho1);
        fprintf(saida, "\tsne $a0, $a0, $zero\n");     /* qualquer valor != 0 vira 1 */
        /* || com 1 ou & com 0 já está decidido: pula o da direita com a resposta em $a0 */
        fprintf(saida, "\t%s $a0, L%d\n", no->especie == OU_OP ? "bnez" : "beqz", fim);
        geraExpressao(no->filho2);
        fprintf(saida, "\tsne $a0, $a0, $zero\n");
        fprintf(saida, "L%d:\n", fim);
        return;
    default:                                           /* operadores binários */
        break;
    }

    geraExpressao(no->filho1);
    empilha();                                         /* o filho da direita vai sobrescrever $a0 */
    geraExpressao(no->filho2);
    desempilha();                                      /* $t1 = esquerda, $a0 = direita */

    /* addu/subu: como add/sub, mas sem exceção de overflow. seq, sne, sgt, sle, sge e o
       div de três operandos são pseudo-instruções do montador; os relacionais dão 1 ou 0. */
    switch (no->especie) {
    case MAIS_OP:       fprintf(saida, "\taddu $a0, $t1, $a0\n"); break;
    case MENOS_OP:      fprintf(saida, "\tsubu $a0, $t1, $a0\n"); break;
    case MULT_OP:       fprintf(saida, "\tmul $a0, $t1, $a0\n");  break;
    case DIV_OP:        fprintf(saida, "\tdiv $a0, $t1, $a0\n");  break;
    case IGUAL_OP:      fprintf(saida, "\tseq $a0, $t1, $a0\n");  break;
    case DIFERENTE_OP:  fprintf(saida, "\tsne $a0, $t1, $a0\n");  break;
    case MENOR_OP:      fprintf(saida, "\tslt $a0, $t1, $a0\n");  break;
    case MAIOR_OP:      fprintf(saida, "\tsgt $a0, $t1, $a0\n");  break;
    case MENORIGUAL_OP: fprintf(saida, "\tsle $a0, $t1, $a0\n");  break;
    case MAIORIGUAL_OP: fprintf(saida, "\tsge $a0, $t1, $a0\n");  break;
    default:            break;
    }
}
