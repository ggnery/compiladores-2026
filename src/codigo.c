/* codigo.c - gerador de código assembly MIPS da linguagem G-V1 */

#include <stdio.h>
#include <stdlib.h>
#include "codigo.h"
#include "tabela.h"

static FILE* saida;        /* arquivo .asm sendo escrito */
static int   rotulos;      /* contador que deixa cada rótulo único: L0, L1, ... */
static int   ocupados;     /* bytes de variáveis já reservados abaixo de $fp */

static void geraBloco(No* bloco);
static void geraComando(No* no);
static void geraExpressao(No* no);

/* Cada variável ocupa uma palavra (4 bytes). O car caberia num byte, mas
   com tudo em palavras o lw/sw sempre cai em endereço múltiplo de 4. */
static int tamanho(Tipo tipo) {
    return 4;                                      /* int e car */
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
    if (lexema[1] != '\\') return lexema[1];
    switch (lexema[2]) {
    case 'n': return '\n';
    case 't': return '\t';
    case 'r': return '\r';
    case '0': return '\0';
    default:  return lexema[2];                    /* \\  \'  \" viram o próprio caractere */
    }
}

void geraCodigo(No* raiz, const char* nomeArquivo) {
    saida = fopen(nomeArquivo, "w");
    if (!saida) { printf("Não foi possível criar o arquivo %s\n", nomeArquivo); exit(1); }

    rotulos  = 0;
    ocupados = 0;
    iniciaPilha();

    fprintf(saida, "# Gerado pelo compilador G-V1\n");
    fprintf(saida, "\t.text\n\t.globl main\nmain:\n");
    fprintf(saida, "\tmove $fp, $sp\t\t# $fp fica parado: as variáveis são achadas a partir dele\n");

    geraBloco(raiz->filho1);                         /* PROGRAMA tem um filho só: o bloco principal */

    fprintf(saida, "\tli $v0, 10\t\t# fim do programa\n\tsyscall\n");
    fclose(saida);
}

/* Mesmo percurso do semântico: declarações (filho1) e depois comandos (filho2).
   Agora a tabela serve para dar a cada variável o seu lugar na memória. */
static void geraBloco(No* bloco) {
    No*      p;
    Entrada* var;
    int      tamBloco = 0;

    if (bloco->filho1) empilhaEscopo();

    for (p = bloco->filho1; p; p = p->filho2) {      /* LISTA_DECL: um DECL por nome */
        var = insere(p->filho1->lexema, p->filho1->tipo, p->filho1->linha);
        ocupados += tamanho(var->tipo);
        tamBloco += tamanho(var->tipo);
        var->deslocamento = -ocupados;               /* a variável mora em deslocamento($fp) */
    }
    if (tamBloco)
        fprintf(saida, "\taddiu $sp, $sp, -%d\t\t# reserva as variáveis do bloco (linha %d)\n", tamBloco, bloco->linha);

    for (p = bloco->filho2; p; p = p->filho2)        /* LISTA_CMD: um comando por nó */
        geraComando(p->filho1);

    if (tamBloco) {
        fprintf(saida, "\taddiu $sp, $sp, %d\t\t# libera as variáveis do bloco\n", tamBloco);
        ocupados -= tamBloco;
        desempilhaEscopo();
    }
}

static void geraComando(No* no) {
    int senao, fim, inicio;

    if (!no) return;                                   /* comando vazio (;) não gera nó */

    fprintf(saida, "\t\t\t\t# linha %d\n", no->linha);
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
        fprintf(saida, "\tj L%d\n", fim);
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
    case ESCREVA_CMD:                                  /* syscall 1 escreve int, 11 escreve car */
        geraExpressao(no->filho1);
        fprintf(saida, "\tli $v0, %d\n\tsyscall\n", no->filho1->tipo == TIPO_INT ? 1 : 11);
        break;
    case ESCREVA_STR:                                  /* o texto vai para .data; syscall 4 escreve */
        inicio = novoRotulo();
        fprintf(saida, "\t.data\nS%d:\t.asciiz %s\n\t.text\n", inicio, no->lexema);
        fprintf(saida, "\tla $a0, S%d\n\tli $v0, 4\n\tsyscall\n", inicio);
        break;
    case NOVALINHA_CMD:
        fprintf(saida, "\tli $a0, 10\n\tli $v0, 11\n\tsyscall\n");
        break;
    default:                                           /* expressão usada como comando: x = 1; */
        geraExpressao(no);
    }
}

/* Deixa o valor da expressão em $a0.
   Pós-ordem: calcula o operando da esquerda e guarda na pilha, calcula o da
   direita, tira o da esquerda para $t1 e só então faz a operação. */
static void geraExpressao(No* no) {
    int fim;

    switch (no->especie) {
    case INT_CONST:
        fprintf(saida, "\tli $a0, %d\n", atoi(no->lexema));
        return;
    case CAR_CONST:
        fprintf(saida, "\tli $a0, %d\t\t# %s\n", codigoCaractere(no->lexema), no->lexema);
        return;
    case ID:
        fprintf(saida, "\tlw $a0, %d($fp)\t\t# %s\n", busca(no->lexema)->deslocamento, no->lexema);
        return;
    case ATRIB:                                        /* x = e: o valor de e continua em $a0 */
        geraExpressao(no->filho2);
        fprintf(saida, "\tsw $a0, %d($fp)\t\t# %s = ...\n", busca(no->filho1->lexema)->deslocamento, no->filho1->lexema);
        return;
    case NEG_OP:
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
        fprintf(saida, "\t%s $a0, L%d\n", no->especie == OU_OP ? "bnez" : "beqz", fim);
        geraExpressao(no->filho2);
        fprintf(saida, "\tsne $a0, $a0, $zero\n");
        fprintf(saida, "L%d:\n", fim);
        return;
    default:                                           /* operadores binários */
        break;
    }

    geraExpressao(no->filho1);
    empilha();
    geraExpressao(no->filho2);
    desempilha();                                      /* $t1 = esquerda, $a0 = direita */

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
