/* Programa de teste em G-V1: soma os numeros de 1 a 10
   e diz se o resultado e grande ou pequeno.
   Este comentario ocupa mais de uma linha - o que e permitido. */

principal
{ soma, i : int;
  c : car;
}
{
  soma = 0;
  i = 1;

  enquanto (i <= 10)
  {
    soma = soma + i;
    i = i + 1;
  }

  escreva "soma: ";
  escreva soma;
  novalinha;

  se (soma > 50) entao
    escreva "grande";
  senao
    escreva "pequeno";
  fimse
  novalinha;

  c = 'a';
  escreva c;
  novalinha;
}
