/* Sombreamento: um bloco interno pode declarar de novo um nome do bloco externo,
   ate com outro tipo. Tem de passar sem erro. Na arvore gravada, o x de dentro
   aparece como car e o de fora como int. */
principal
{ x : int;
  c : car;
}
{
  x = 1;
  c = 'a';
  { x : car;          /* este x esconde o x int de fora */
    y : int;
  }
  {
    x = 'b';
    y = x == c;       /* comparar dois car da um int */
    escreva c;        /* c continua sendo o de fora */
  }
  { y : car; }        /* bloco irmao: o y int do bloco anterior ja saiu */
  {
    y = 'z';
  }
  x = x + 1;          /* fora dos blocos, x volta a ser o int */
}
