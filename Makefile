# ===== Makefile =====
CC      := gcc
CFLAGS  := -Wall -Wextra -Wconversion -g -Iinclude
SRC_DIR := src
TST_DIR := tests

# 所有源码对象
SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(SRCS:%.c=%.o)

# 所有测试文件：tests/test_*.c
TEST_SRCS := $(wildcard $(TST_DIR)/test_*.c)
TEST_BINS := $(TEST_SRCS:.c=)

.PHONY: all test clean run-%

# 默认执行 test
all: test

# 源文件 -> .o
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 测试源文件 -> 可执行文件
$(TST_DIR)/%: $(TST_DIR)/%.c $(OBJS)
	@echo "[BUILD] $@"
	$(CC) $(CFLAGS) $^ -o $@

# 一键运行所有 tests/test_*.c
test: $(OBJS) $(TEST_BINS)
	@echo "===== Running all tests ====="
	@set -e; for t in $(TEST_BINS); do \
		echo "[RUN] $$t"; ./$$t; echo ""; \
	done
	@echo "[OK] all tests passed."

# 单独运行某个测试
run-%: $(OBJS) $(TST_DIR)/%
	@echo "[RUN] $(TST_DIR)/$*"
	@./$(TST_DIR)/$*

# 清理所有生成文件
clean:
	rm -f $(OBJS) $(TEST_BINS) *.o *.gch
	@echo "[OK] cleaned"
