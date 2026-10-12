/* Casos de borda corrigidos na revisao. Rodar com:
     ./g-v1 testes/Outros/ok_borda.g
     spim -quiet -file testes/Outros/ok_borda.asm
   Saida esperada (uma por linha):
     5 5       INTCONST com sinal + (Tabela 1): +5 e a - +2
     1         escape '\a' de CARCONST vira o codigo 7, e nao a letra a
     C:\       cadeia terminada em barra invertida */
principal
{ a, b : int;
  c : car;
}
{
  a = 7;
  b = +5;     escreva b; escreva " ";
  b = a - +2; escreva b; novalinha;
  c = '\a';   escreva c == '\a' & c != 'a'; novalinha;
  escreva "C:\"; novalinha;
}
