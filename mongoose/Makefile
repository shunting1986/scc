# make file for SCC

MONGOOSE_SRC_ROOT=mongoose/mongoose

mongoose-rebuild: rebuild-$(PROG_NAME) mongoose

mongoose:
	@#./scc $(MONGOOSE_SRC_ROOT)/main.c > mongoose/build/main.s
	./scc $(MONGOOSE_SRC_ROOT)/mongoose.c > mongoose/build/mongoose.s

.PHONY: mongoose
