/* codigo.c - gerador de código: percorre a árvore e escreve o assembly MIPS */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codigo.h"
#include "tabela.h"

static FILE* saida;        /* arquivo .asm */
static int   rotulos;      /* próximo número de rótulo (L e S) */
static int   ocupados;     /* bytes já reservados abaixo de $fp */

static void geraBloco(No* bloco);
static void geraComando(No* no);
static void geraExpressao(No* no);

static int tamanho(Tipo tipo) {
    return 4;                                      /* int e car: uma palavra cada */
}

static int novoRotulo(void) {
    return rotulos++;
}

/* Guarda $a0 no topo da pilha / tira o topo da pilha para $t1. */
static void empilha(void) {
    fprintf(saida, "\taddiu $sp, $sp, -4\n\tsw $a0, 0($sp)\n");
}

static void desempilha(void) {
    fprintf(saida, "\tlw $t1, 0($sp)\n\taddiu $sp, $sp, 4\n");
}

/* Código ASCII de um CARCONST: "'a'" -> 97, "'\n'" -> 10. */
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
    default:  return lexema[2];                    /* \\  \'  \" viram o próprio caractere */
    }
}

/* Escreve a cadeia em .data. Barras antes da aspa final saem como .byte
   (o montador leria \" como aspa). */
static void escreveCadeia(const char* lexema) {
    int fim    = strlen(lexema) - 1;                 /* posição da aspa final */
    int barras = 0;

    /* conta as barras coladas na aspa final */
    while (fim - barras - 1 > 0 && lexema[fim - barras - 1] == '\\') barras++;

    if (barras == 0) {                               /* caso comum */
        fprintf(saida, ".asciiz %s\n", lexema);
        return;
    }
    fprintf(saida, ".ascii %.*s\"\n", fim - barras, lexema);   /* "C:\" -> .ascii "C:" */
    while (barras--) fprintf(saida, "\t.byte 92\n");          /* 92 = '\\' */
    fprintf(saida, "\t.byte 0\n");                  /* terminador */
}

void geraCodigo(No* raiz, const char* nomeArquivo) {
    saida = fopen(nomeArquivo, "w");
    if (!saida) { printf("Não foi possível criar o arquivo %s\n", nomeArquivo); exit(1); }

    rotulos  = 0;
    ocupados = 0;
    iniciaPilha();                                   /* escopos refeitos neste percurso */

    fprintf(saida, "# Gerado pelo compilador G-V1\n");
    fprintf(saida, "\t.text\n\t.globl main\nmain:\n");   /* o SPIM começa em main */
    fprintf(saida, "\tmove $fp, $sp\t\t# $fp fica parado: as variáveis são achadas a partir dele\n");

    geraBloco(raiz->filho1);                         /* o bloco principal */

    fprintf(saida, "\tli $v0, 10\t\t# fim do programa\n\tsyscall\n");
    fclose(saida);
}

/* Como no semântico, mas dá a cada variável seu lugar: deslocamento($fp). */
static void geraBloco(No* bloco) {
    No*      p;
    Entrada* var;
    int      tamBloco = 0;

    if (bloco->filho1) empilhaEscopo();              /* só bloco com declarações abre escopo */

    for (p = bloco->filho1; p; p = p->filho2) {      /* cada declaração */
        var = insere(p->filho1->lexema, p->filho1->tipo, p->filho1->linha);
        ocupados += tamanho(var->tipo);
        tamBloco += tamanho(var->tipo);
        var->deslocamento = -ocupados;               /* abaixo de $fp: -4, -8, ... */
    }
    if (tamBloco)
        fprintf(saida, "\taddiu $sp, $sp, -%d\t\t# reserva as variáveis do bloco (linha %d)\n", tamBloco, bloco->linha);

    for (p = bloco->filho2; p; p = p->filho2)        /* cada comando */
        geraComando(p->filho1);

    if (tamBloco) {                                  /* só se houve declaração */
        fprintf(saida, "\taddiu $sp, $sp, %d\t\t# libera as variáveis do bloco\n", tamBloco);
        ocupados -= tamBloco;                        /* blocos irmãos reusam o espaço */
        desempilhaEscopo();
    }
}

static void geraComando(No* no) {
    int senao, fim, inicio;

    if (!no) return;                                   /* comando vazio (;) ou senão ausente */

    fprintf(saida, "\t\t\t\t# linha %d\n", no->linha);  /* linha do fonte, para achar no .asm */
    switch (no->especie) {
    case BLOCO:
        geraBloco(no);
        break;
    case SE_CMD:                                       /* condição falsa (0) pula o então */
        senao = novoRotulo();
        fim   = novoRotulo();
        geraExpressao(no->filho1);
        fprintf(saida, "\tbeqz $a0, L%d\n", senao);
        geraComando(no->filho2);                       /* então */
        fprintf(saida, "\tj L%d\n", fim);              /* pula o senão */
        fprintf(saida, "L%d:\n", senao);
        geraComando(no->filho3);                       /* senão */
        fprintf(saida, "L%d:\n", fim);
        break;
    case ENQUANTO_CMD:                                 /* testa no início; o corpo volta ao teste */
        inicio = novoRotulo();
        fim    = novoRotulo();
        fprintf(saida, "L%d:\n", inicio);
        geraExpressao(no->filho1);
        fprintf(saida, "\tbeqz $a0, L%d\n", fim);
        geraComando(no->filho2);                       /* corpo */
        fprintf(saida, "\tj L%d\n", inicio);
        fprintf(saida, "L%d:\n", fim);
        break;
    case LEIA_CMD:                                     /* syscall 5 lê int, 12 lê car; valor em $v0 */
        fprintf(saida, "\tli $v0, %d\n\tsyscall\n", no->filho1->tipo == TIPO_INT ? 5 : 12);
        fprintf(saida, "\tsw $v0, %d($fp)\n", busca(no->filho1->lexema)->deslocamento);
        break;
    case ESCREVA_CMD:                                  /* syscall 1 escreve int, 11 escreve car */
        geraExpressao(no->filho1);
        fprintf(saida, "\tli $v0, %d\n\tsyscall\n", no->filho1->tipo == TIPO_INT ? 1 : 11);
        break;
    case ESCREVA_STR:                                  /* texto em .data; syscall 4 escreve */
        inicio = novoRotulo();
        fprintf(saida, "\t.data\nS%d:\t", inicio);
        escreveCadeia(no->lexema);
        fprintf(saida, "\t.text\n\tla $a0, S%d\n\tli $v0, 4\n\tsyscall\n", inicio);
        break;
    case NOVALINHA_CMD:                                /* syscall 11 escreve o car 10 ('\n') */
        fprintf(saida, "\tli $a0, 10\n\tli $v0, 11\n\tsyscall\n");
        break;
    default:                                           /* expressão como comando: x = 1; */
        geraExpressao(no);
    }
}

/* Deixa o valor da expressão em $a0. */
static void geraExpressao(No* no) {
    int fim;

    switch (no->especie) {
    case INT_CONST:                                    /* sem sinal: -42 é NEG_OP */
        fprintf(saida, "\tli $a0, %d\n", atoi(no->lexema));
        return;
    case CAR_CONST:
        fprintf(saida, "\tli $a0, %d\t\t# %s\n", codigoCaractere(no->lexema), no->lexema);
        return;
    case ID:                                           /* vale a declaração mais interna */
        fprintf(saida, "\tlw $a0, %d($fp)\t\t# %s\n", busca(no->lexema)->deslocamento, no->lexema);
        return;
    case ATRIB:                                        /* x = e: o valor de e fica em $a0 */
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
        /* || com 1 ou & com 0: pula a direita */
        fprintf(saida, "\t%s $a0, L%d\n", no->especie == OU_OP ? "bnez" : "beqz", fim);
        geraExpressao(no->filho2);
        fprintf(saida, "\tsne $a0, $a0, $zero\n");
        fprintf(saida, "L%d:\n", fim);
        return;
    default:                                           /* operadores binários */
        break;
    }

    geraExpressao(no->filho1);
    empilha();                                         /* guarda a esquerda */
    geraExpressao(no->filho2);
    desempilha();                                      /* $t1 = esquerda, $a0 = direita */

    /* addu/subu ignoram overflow; relacionais dão 1 ou 0 */
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
