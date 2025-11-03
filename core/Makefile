# ==== core/Makefile ====

CC = gcc
CFLAGS = -Wall -Wextra -Wconversion -g
TARGETS = list_base pred filter

# 默认目标：显示提示
all:
	@echo "Usage:"
	@echo "  make list_base   # build and run list_base.c"
	@echo "  make pred        # build and run pred.c (later)"
	@echo "  make filter      # build and run filter.c (later)"
	@echo "  make clean       # remove all executables"

# ---- 各模块 ----
list_base: list_base.c model.h list_base.h
	$(CC) $(CFLAGS) list_base.c -o $@ && ./list_base

pred: pred.c model.h list_base.h
	$(CC) $(CFLAGS) pred.c -o $@ && ./pred

filter: filter.c model.h list_base.h
	$(CC) $(CFLAGS) filter.c -o $@ && ./filter

# ---- 清理 ----
clean:
	rm -f $(TARGETS) *.o *.gch
