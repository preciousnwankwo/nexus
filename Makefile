CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -std=c11 -g -O2
LDFLAGS ?=

SRC_DIR = src/bootstrap
RUNTIME_DIR = src/runtime
BUILD_DIR = build
TEST_DIR = tests

SRCS = $(SRC_DIR)/arena.c $(SRC_DIR)/string_table.c $(SRC_DIR)/lexer.c $(SRC_DIR)/ast.c $(SRC_DIR)/parser.c $(SRC_DIR)/symbol_table.c $(SRC_DIR)/typecheck.c $(SRC_DIR)/codegen.c
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

LIB_OBJS = $(BUILD_DIR)/arena.o $(BUILD_DIR)/string_table.o $(BUILD_DIR)/lexer.o $(BUILD_DIR)/ast.o $(BUILD_DIR)/parser.o $(BUILD_DIR)/symbol_table.o $(BUILD_DIR)/typecheck.o $(BUILD_DIR)/codegen.o

RUNTIME_SRC = $(RUNTIME_DIR)/runtime.c
RUNTIME_OBJ = $(BUILD_DIR)/runtime.o

LEXER_TEST_SRC = $(TEST_DIR)/lexer/test_lexer.c
LEXER_TEST_BIN = $(BUILD_DIR)/test_lexer

PARSER_TEST_SRC = $(TEST_DIR)/parser/test_parser.c
PARSER_TEST_BIN = $(BUILD_DIR)/test_parser

.PHONY: all clean test compile

all: $(BUILD_DIR)/nexus-bootstrap

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(RUNTIME_OBJ): $(RUNTIME_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(RUNTIME_DIR) -c $< -o $@

$(BUILD_DIR)/nexus-bootstrap: $(LIB_OBJS) $(SRC_DIR)/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LIB_OBJS) $(SRC_DIR)/main.c -o $@ $(LDFLAGS)

$(LEXER_TEST_BIN): $(LIB_OBJS) $(LEXER_TEST_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LIB_OBJS) $(LEXER_TEST_SRC) -o $@ $(LDFLAGS)

$(PARSER_TEST_BIN): $(LIB_OBJS) $(PARSER_TEST_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LIB_OBJS) $(PARSER_TEST_SRC) -o $@ $(LDFLAGS)

test: $(LEXER_TEST_BIN) $(PARSER_TEST_BIN)
	./$(LEXER_TEST_BIN)
	./$(PARSER_TEST_BIN)

compile: $(BUILD_DIR)/nexus-bootstrap $(RUNTIME_OBJ)
	@echo "Usage: make compile SOURCE=example.nx"
	@if [ -n "$(SOURCE)" ]; then \
		./$(BUILD_DIR)/nexus-bootstrap $(SOURCE) $(BUILD_DIR)/output.c && \
		$(CC) $(CFLAGS) -I$(RUNTIME_DIR) $(BUILD_DIR)/output.c $(RUNTIME_OBJ) -o $(BUILD_DIR)/output && \
		echo "Compiled to $(BUILD_DIR)/output"; \
	fi

clean:
	rm -rf $(BUILD_DIR)
