/* teste_tabela.c - demonstra e confere a pilha de tabelas de símbolos (Fase 3)

   Simula o que o analisador semântico vai fazer com este programa:

       principal
       { x, y : int; }          <- bloco externo: linha 2
       {
           { x : car; }         <- bloco interno redeclara x: linha 4
           { ... }
       }

   Para rodar:  make teste-tabela */

#include <stdio.h>
#include "tabela.h"

static int falhas = 0;

static void confere(int ok, const char* situacao) {
    printf("%s  %s\n", ok ? "  ok  " : "FALHA ", situacao);
    if (!ok) falhas++;
}

int main(void) {
    Entrada* e;
    char nome1[] = "soma", nome2[] = "soma";   /* mesmo texto, endereços diferentes */

    iniciaPilha();
    imprimePilha();

    empilhaEscopo();                           /* entra no bloco externo */
    insere("x", TIPO_INT, 2);
    insere("y", TIPO_INT, 2);
    imprimePilha();
    confere(insere("x", TIPO_CAR, 3) == NULL, "redeclarar x no mesmo escopo devolve NULL");

    empilhaEscopo();                           /* entra no bloco interno */
    confere(insere("x", TIPO_CAR, 4) != NULL, "x de novo, em outro escopo, é permitido");
    imprimePilha();

    e = busca("x");
    confere(e && e->tipo == TIPO_CAR && e->linha == 4, "x nos dois blocos: a busca acha o interno");
    e = busca("y");
    confere(e && e->tipo == TIPO_INT && e->linha == 2, "y só no externo: a busca acha o externo");
    confere(busca("z") == NULL, "nome inexistente: a busca devolve NULL");

    desempilhaEscopo();                        /* sai do bloco interno */
    imprimePilha();
    e = busca("x");
    confere(e && e->tipo == TIPO_INT && e->linha == 2, "saiu do bloco: o topo foi removido e x volta a ser o externo");

    desempilhaEscopo();                        /* sai do bloco externo */
    imprimePilha();
    confere(busca("x") == NULL && busca("y") == NULL, "saiu de todos os blocos: a pilha fica vazia");

    empilhaEscopo();
    insere(nome1, TIPO_INT, 1);
    confere(busca(nome2) != NULL, "a busca compara o texto do nome, não o endereço");
    desempilhaEscopo();

    printf("\n%d falha(s)\n", falhas);
    return falhas != 0;
}
