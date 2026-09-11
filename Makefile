# Makefile do compilador G-V1
# Para gerar o executavel basta digitar:  make
#
# As ferramentas ficam em variaveis para o mesmo Makefile funcionar no
# Linux do professor e no Mac (onde o bison novo esta em outro lugar).
CC    = gcc
BISON = bison
FLEX  = flex

# Pasta com TUDO que e' gerado automaticamente.
# Nada dentro de build/ e' escrito a mao - da para apagar a qualquer momento.
BUILD = build

# ---- produtos do ANALISADOR SINTATICO (o Bison gera a partir de g-v1.y) ----
SINTATICO_C = $(BUILD)/sintatico.c
SINTATICO_H = $(BUILD)/g-v1.tab.h
SINTATICO_O = $(BUILD)/sintatico.o

# ---- produtos do ANALISADOR LEXICO (o Flex gera a partir de g-v1.l) ----
LEXICO_C = $(BUILD)/lexico.c
LEXICO_O = $(BUILD)/lexico.o

# ---- arvore sintatica abstrata (escrita a mao, nao e' gerada) ----
AST_O = $(BUILD)/ast.o

# Alvo principal: liga os dois objetos e produz o executavel.
# O executavel fica na RAIZ porque o enunciado pede:  ./g-v1 teste.g
g-v1: $(SINTATICO_O) $(LEXICO_O) $(AST_O)
	$(CC) $(SINTATICO_O) $(LEXICO_O) $(AST_O) -o g-v1

# ------------------------------- arvore ---------------------------------
$(AST_O): ast.c ast.h
	mkdir -p $(BUILD)
	$(CC) -c ast.c -o $(AST_O)

# ------------------------- analisador sintatico -------------------------
$(SINTATICO_C): g-v1.y
	mkdir -p $(BUILD)
	$(BISON) --defines=$(SINTATICO_H) -o $(SINTATICO_C) g-v1.y

# "-I." para achar ast.h na raiz, ja' que sintatico.c mora em build/.
# O bison gera .c e .h de uma vez; esta regra so' repara o .h se ele sumir.
$(SINTATICO_H): $(SINTATICO_C)
	@test -f $@ || $(BISON) --defines=$(SINTATICO_H) -o $(SINTATICO_C) g-v1.y

$(SINTATICO_O): $(SINTATICO_C) $(SINTATICO_H) ast.h
	$(CC) -I$(BUILD) -I. -c $(SINTATICO_C) -o $(SINTATICO_O)

# -------------------------- analisador lexico ---------------------------
# Depende de $(SINTATICO_C) porque e' naquele passo que o header nasce,
# e o lexico precisa dele para saber o numero de cada token.
$(LEXICO_C): g-v1.l $(SINTATICO_H)
	mkdir -p $(BUILD)
	$(FLEX) -o $(LEXICO_C) g-v1.l

# "-I." tambem aqui: g-v1.tab.h passou a incluir ast.h (por causa da %union).
$(LEXICO_O): $(LEXICO_C) ast.h
	$(CC) -I$(BUILD) -I. -c $(LEXICO_C) -o $(LEXICO_O)

# Apaga tudo que foi gerado. "-r" porque agora e' uma pasta inteira.
clean:
	rm -rf $(BUILD) g-v1

# Avisa ao make que "clean" e' um comando, nao um arquivo a ser criado.
.PHONY: clean
