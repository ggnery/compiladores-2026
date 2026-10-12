# Makefile do compilador G-V1
# Para gerar o executavel basta digitar:  make
#
# Ferramentas em variaveis: no Mac, use  make BISON=<caminho do bison 3.8>
CC    = gcc
BISON = bison
FLEX  = flex

# Pasta do que e' gerado automaticamente (da para apagar).
BUILD = build

# Pasta do codigo escrito a mao (.l, .y, .c, .h).
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

# Todos os pedacos do compilador (o main esta no g-v1.y).
OBJETOS = $(SINTATICO_O) $(LEXICO_O) $(AST_O) $(TABELA_O) $(SEMANTICO_O) $(CODIGO_O)

# Alvo principal (o primeiro, por isso basta "make"): liga tudo em ./g-v1
g-v1: $(OBJETOS)
	$(CC) $(OBJETOS) -o g-v1

# ------------------------------- arvore ---------------------------------
$(AST_O): $(SRC)/ast.c $(SRC)/ast.h
	mkdir -p $(BUILD)
	$(CC) -c $(SRC)/ast.c -o $(AST_O)

# --------------------------- tabela de simbolos --------------------------
$(TABELA_O): $(SRC)/tabela.c $(SRC)/tabela.h $(SRC)/ast.h
	mkdir -p $(BUILD)
	$(CC) -c $(SRC)/tabela.c -o $(TABELA_O)

# Teste da tabela (Fase 3):  make teste-tabela
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
# O bison gera o .c e o .h de uma vez.
# "touch": deixa o .h mais novo que o .c, senao o make nunca fica "up to date".
$(SINTATICO_C): $(SRC)/g-v1.y
	mkdir -p $(BUILD)
	$(BISON) --defines=$(SINTATICO_H) -o $(SINTATICO_C) $(SRC)/g-v1.y
	touch $(SINTATICO_H)

# O .h vem da regra de cima; o bison so' roda de novo se o .h sumir.
$(SINTATICO_H): $(SINTATICO_C)
	@test -f $@ || $(BISON) --defines=$(SINTATICO_H) -o $(SINTATICO_C) $(SRC)/g-v1.y

# "-I$(SRC)": sintatico.c fica em build/, mas inclui os .h de src/.
$(SINTATICO_O): $(SINTATICO_C) $(SINTATICO_H) $(SRC)/ast.h $(SRC)/semantico.h $(SRC)/codigo.h
	$(CC) -I$(BUILD) -I$(SRC) -c $(SINTATICO_C) -o $(SINTATICO_O)

# -------------------------- analisador lexico ---------------------------
# Depende do .h do bison, que tem o numero de cada token.
$(LEXICO_C): $(SRC)/g-v1.l $(SINTATICO_H)
	mkdir -p $(BUILD)
	$(FLEX) -o $(LEXICO_C) $(SRC)/g-v1.l

# "-I$(SRC)" tambem aqui: g-v1.tab.h inclui ast.h (por causa da %union).
$(LEXICO_O): $(LEXICO_C) $(SRC)/ast.h
	$(CC) -I$(BUILD) -I$(SRC) -c $(LEXICO_C) -o $(LEXICO_O)

# Apaga a pasta build/ e o executavel.
clean:
	rm -rf $(BUILD) g-v1

# Avisa ao make que "clean" e "teste-tabela" sao comandos, nao arquivos.
.PHONY: clean teste-tabela
