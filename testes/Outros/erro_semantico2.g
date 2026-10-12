/* Erro semantico esperado na linha 8: operacao logica com um car */
principal
{ n : int;
  c : car;
}
{
  leia n;
  se (n > 0 & c) entao
    escreva "positivo";
  fimse
}
