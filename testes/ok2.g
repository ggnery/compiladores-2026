/* Segundo teste correto: cobre o que o ok1.g nao usa -
   leia, comando vazio, todos os operadores, bloco aninhado,
   constante caractere com escape e menos unario. */
principal
{ a, b : int;
  c : car;
}
{
  ;
  leia a;
  leia b;
  c = '\n';
  a = -5;
  b = a * 2 / 3;
  b = a-1;

  se (a >= b & b != 0) entao
    escreva "ok";
  fimse

  se (!(a == b) || a < b) entao
  { d : int;
  }
  {
    d = a + b;
    escreva d;
  }
  senao
    ;
  fimse

  enquanto (a > 0)
    a = a - 1;

  novalinha;
}
