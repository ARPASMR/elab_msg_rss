# cumulata makefile
# 2018.04.19 Luca Paganotti

GCC=/usr/bin/gcc
SRC=./cumulata.c
OUT=./cumulata

all: cumulata.c
	@echo "Using gcc: $(GCC)"
	@echo "Source:    $(SRC)"
	@echo "Output:    $(OUT)"
	@echo "Building ..."
	$(GCC) $(SRC) -o $(OUT)
	@echo "Done."

.PHONY: clean

clean:
	@echo "Cleaning ..."
	@rm -f $(OUT)
	@echo "Done."


