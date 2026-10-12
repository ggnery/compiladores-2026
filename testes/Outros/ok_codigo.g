/* Teste da geracao de codigo. Rodar com:
     ./g-v1 testes/Outros/ok_codigo.g
     printf "z\n" | spim -quiet -file testes/Outros/ok_codigo.asm
   Saida esperada (uma por linha):
     z         le e escreve um car
     7 3       x de fora; x de dentro esconde o de fora
     7         ao sair do bloco, x volta a ser o de fora
     -2 1      menos unario e divisao inteira (7 / 4 = 1)
     ok        & e || com curto-circuito
     senao     o ramo senao
     54321     enquanto com bloco que declara variavel a cada volta */
principal
{ x, i : int;
  c : car;
}
{
  leia c;
  escreva c; novalinha;

  x = 7;
  escreva x; escreva " ";
  { x : int; }
  { x = 3; escreva x; novalinha; }
  escreva x; novalinha;

  escreva -2; escreva " "; escreva x / 4; novalinha;

  se ((x > 100 & x / 0 == 1) || (x == 7 & c == 'z')) entao
    escreva "ok";
  fimse
  novalinha;

  se (!(x == 7)) entao escreva "entao"; senao escreva "senao"; fimse
  novalinha;

  i = 5;
  enquanto (i > 0)
  { d : int; }
  { d = i; escreva d; i = i - 1; }
  novalinha;
}
