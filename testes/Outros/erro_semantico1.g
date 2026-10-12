/* Erro semantico esperado na linha 9: aritmetica com um car */
principal
{ total : int;
  c : car;
}
{
  c = 'a';
  total = 0;
  total = c + 1;
}
