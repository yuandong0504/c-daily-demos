# ===== core/Makefile =====
CC      = gcc
CFLAGS  = -Wall -Wextra -Wconversion -g
OBJS    = list_base.o pred_filter.o
TESTS   = $(patsubst %.c,%,$(wildcard _check_*.c))

# ===== 默认帮助 =====
all:
	@echo "Usage:"
	@echo "  make cfa        # 编译并运行 _check_free_all.c"
	@echo "  make check      # 自动编译并运行所有 _check_*.c 测试"
	@echo "  make clean      # 清理目标文件"

# ===== 通用对象规则 =====
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ===== free_all 自测 =====
cfa: $(OBJS) _check_free_all.c
	$(CC) $(CFLAGS) $^ -o $@
	@echo "[RUN] ./cfa"
	@./$@

# ===== 自动测试入口 =====
check: $(OBJS) $(TESTS)
	@for t in $(TESTS); do \
		echo "[BUILD] $$t"; \
		$(CC) $(CFLAGS) $(OBJS) $$t.c -o $$t || exit 1; \
	done
	@echo ""
	@echo "===== Running all tests ====="
	@for t in $(TESTS); do \
		echo "[RUN] ./$$t"; \
		./$$t || exit 1; \
		echo ""; \
	done
	@echo "[OK] All tests passed."

# ===== 清理 =====
clean:
	rm -f $(OBJS) $(TESTS) *.o *.gch cfa
	@echo "[OK] cleaned"
