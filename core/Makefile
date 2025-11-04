# ==== core/Makefile ====
CC      = gcc
CFLAGS  = -Wall -Wextra -Wconversion -g -MMD -MP

# 对象文件与测试目标
OBJS    = list_base.o
TESTS   = _check_list _check_pred_filter

.PHONY: all list pred hdrcheck clean

# --- 默认帮助信息 ---
all:
	@echo "Usage:"
	@echo "  make list        # 编译并运行 _check_list.c"
	@echo "  make pred        # 编译并运行 _check_pred_filter.c"
	@echo "  make hdrcheck    # 只验证头文件是否可编译"
	@echo "  make clean       # 清理目标文件"

# --- 基础对象（自动追踪头文件依赖） ---
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- 链表模块自测 ---
_check_list: _check_list.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@

list: _check_list
	./_check_list

# --- 谓词过滤模块自测 ---
_check_pred_filter: _check_pred_filter.c pred_filter.c
	$(CC) $(CFLAGS) _check_pred_filter.c pred_filter.c -o $@

pred: _check_pred_filter
	./_check_pred_filter

# --- 头文件快速验证（仅语法检查） ---
hdrcheck:
	$(CC) $(CFLAGS) -fsyntax-only _check_pred_filter.c

# --- 清理 ---
clean:
	rm -f $(OBJS) $(TESTS) *.o *.gch *.d list pred
	@echo "[OK] cleaned"

# 自动包含依赖文件 (.d)
-include $(OBJS:.o=.d)
