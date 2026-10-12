# Makefile do compilador G-V1
# Para gerar o executavel basta digitar:  make
#
# Nao e' uma fase (fonte -> tokens -> arvore -> semantico -> MIPS): monta o
# executavel que faz todas elas.
#   src/g-v1.y --bison--> build/sintatico.c + build/g-v1.tab.h
#   src/g-v1.l --flex---> build/lexico.c
#   cada .c --gcc -c--> build/*.o --gcc--> ./g-v1
# Secao 3 (analisadores ligados num executavel g-v1) e Secao 8 (gerar
# lexico e sintatico, compilar e ligar digitando apenas "make").
#
# As ferramentas ficam em variaveis para o mesmo Makefile funcionar no
# Linux do professor (gcc 14, flex 2.6.4, bison 3.8.2 - Secao 8) e no Mac,
# onde /usr/bin/bison e' o 2.3 e o 3.8 fica em outro lugar (make BISON=<caminho>).
CC    = gcc
BISON = bison
FLEX  = flex

# Pasta do que e' gerado automaticamente: saidas do bison e do flex, os .o e a
# arvore.txt que o ./g-v1 grava. Nada aqui e' escrito a mao - da para apagar.
BUILD = build

# Pasta com o codigo do compilador escrito a mao (.l, .y, .c, .h).
SRC = src

# ---- produtos do ANALISADOR SINTATICO (o Bison gera a partir de src/g-v1.y) ----
SINTATICO_C = $(BUILD)/sintatico.c
SINTATICO_H = $(BUILD)/g-v1.tab.h
SINTATICO_O = $(BUILD)/sintatico.o

# ---- produtos do ANALISADOR LEXICO (o Flex gera a partir de src/g-v1.l) ----
LEXICO_C = $(BUILD)/lexico.c
LEXICO_O = $(BUILD)/lexico.o

# ---- arvore sintatica abstrata (escrita a mao, nao e' gerada) ----
AST_O = $(BUILD)/ast.o

# ---- tabela de simbolos (escrita a mao) ----
TABELA_O = $(BUILD)/tabela.o

# ---- analisador semantico (escrito a mao) ----
SEMANTICO_O = $(BUILD)/semantico.o

# ---- gerador de codigo MIPS (escrito a mao) ----
CODIGO_O = $(BUILD)/codigo.o

# Todos os pedacos que formam o compilador (o main() esta' no g-v1.y,
# portanto em sintatico.o).
OBJETOS = $(SINTATICO_O) $(LEXICO_O) $(AST_O) $(TABELA_O) $(SEMANTICO_O) $(CODIGO_O)

# Alvo principal: liga os objetos e produz o executavel.
# E' o PRIMEIRO alvo do arquivo, por isso "make" sem argumentos o constroi.
# O executavel fica na RAIZ porque o enunciado pede:  ./g-v1 teste.g
g-v1: $(OBJETOS)
	$(CC) $(OBJETOS) -o g-v1

# Cada .o depende do seu .c e dos .h que ele inclui (direta ou indiretamente):
# se um header muda, o make recompila so' quem o usa.
# "mkdir -p": build/ some depois de "make clean"; o -p nao reclama se ela ja' existe.

# ------------------------------- arvore ---------------------------------
$(AST_O): $(SRC)/ast.c $(SRC)/ast.h
	mkdir -p $(BUILD)
	$(CC) -c $(SRC)/ast.c -o $(AST_O)

# --------------------------- tabela de simbolos --------------------------
$(TABELA_O): $(SRC)/tabela.c $(SRC)/tabela.h $(SRC)/ast.h
	mkdir -p $(BUILD)
	$(CC) -c $(SRC)/tabela.c -o $(TABELA_O)

# Teste da tabela (Fase 3):  make teste-tabela
# Nao faz parte do g-v1: liga so' tabela.o com o main de testes/Tabela/ e roda.
teste-tabela: $(TABELA_O)
	$(CC) -I$(SRC) testes/Tabela/teste_tabela.c $(TABELA_O) -o $(BUILD)/teste_tabela
	./$(BUILD)/teste_tabela

# ------------------------- analisador semantico -------------------------
$(SEMANTICO_O): $(SRC)/semantico.c $(SRC)/semantico.h $(SRC)/tabela.h $(SRC)/ast.h
	mkdir -p $(BUILD)
	$(CC) -c $(SRC)/semantico.c -o $(SEMANTICO_O)

# --------------------------- gerador de codigo ---------------------------
$(CODIGO_O): $(SRC)/codigo.c $(SRC)/codigo.h $(SRC)/tabela.h $(SRC)/ast.h
	mkdir -p $(BUILD)
	$(CC) -c $(SRC)/codigo.c -o $(CODIGO_O)

# ------------------------- analisador sintatico -------------------------
# Uma so' execucao do bison gera o parser (-o, .c) e o header (--defines, .h)
# com os numeros dos tokens, yylval e yylloc, que o lexico tambem inclui.
# "touch": o bison grava o .h ~20 ms antes do .c. O make do Linux compara
# fracoes de segundo, acharia o .h sempre mais velho que o .c (regra abaixo)
# e nunca diria "up to date". O touch deixa o .h mais novo que o .c.
$(SINTATICO_C): $(SRC)/g-v1.y
	mkdir -p $(BUILD)
	$(BISON) --defines=$(SINTATICO_H) -o $(SINTATICO_C) $(SRC)/g-v1.y
	touch $(SINTATICO_H)

# O .h ja' nasce na regra de cima; esta regra so' diz ao make de onde ele vem
# e roda o bison de novo apenas se o .h sumir ($@ = o alvo, build/g-v1.tab.h;
# o @ no inicio da linha so' impede o make de ecoar o comando).
$(SINTATICO_H): $(SINTATICO_C)
	@test -f $@ || $(BISON) --defines=$(SINTATICO_H) -o $(SINTATICO_C) $(SRC)/g-v1.y

# "-I$(SRC)": sintatico.c mora em build/, mas inclui ast.h, semantico.h e codigo.h de src/.
$(SINTATICO_O): $(SINTATICO_C) $(SINTATICO_H) $(SRC)/ast.h $(SRC)/semantico.h $(SRC)/codigo.h
	$(CC) -I$(BUILD) -I$(SRC) -c $(SINTATICO_C) -o $(SINTATICO_O)

# -------------------------- analisador lexico ---------------------------
# Depende do header do bison: lexico.c inclui g-v1.tab.h para saber o numero
# de cada token. Por isso o bison sempre roda antes do flex, e o lexico e'
# refeito quando o g-v1.y muda (os numeros dos tokens podem ter mudado).
$(LEXICO_C): $(SRC)/g-v1.l $(SINTATICO_H)
	mkdir -p $(BUILD)
	$(FLEX) -o $(LEXICO_C) $(SRC)/g-v1.l

# "-I$(SRC)" tambem aqui: g-v1.tab.h inclui ast.h (a %union usa No* e Tipo).
$(LEXICO_O): $(LEXICO_C) $(SRC)/ast.h
	$(CC) -I$(BUILD) -I$(SRC) -c $(LEXICO_C) -o $(LEXICO_O)

# Apaga a pasta build/ inteira (por isso o "-r") e o executavel.
clean:
	rm -rf $(BUILD) g-v1

# Avisa ao make que "clean" e "teste-tabela" sao comandos, nao arquivos a criar:
# sem isso, um arquivo chamado "clean" seria dado como "up to date" e o rm nao rodaria.
.PHONY: clean teste-tabela
