# make file for SCC

MONGOOSE_SRC_ROOT=mongoose/mongoose
OUT_DIR=mongoose/build

mongoose-rebuild: rebuild-$(PROG_NAME) mongoose

mongoose:
	./scc $(MONGOOSE_SRC_ROOT)/main.c > $(OUT_DIR)/main.s || true
	./scc $(MONGOOSE_SRC_ROOT)/mongoose.c > $(OUT_DIR)/mongoose.s || true
	gcc -m32 -g $(OUT_DIR)/main.s $(OUT_DIR)/mongoose.s -ldl -lpthread
	./a.out

.PHONY: mongoose
