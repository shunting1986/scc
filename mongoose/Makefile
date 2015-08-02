# make file for SCC

MONGOOSE_SRC_ROOT=mongoose/mongoose

mongoose-rebuild: rebuild-$(PROG_NAME) mongoose

mongoose:
	@#./scc $(MONGOOSE_SRC_ROOT)/main.c
	./scc $(MONGOOSE_SRC_ROOT)/mongoose.c

.PHONY: mongoose
