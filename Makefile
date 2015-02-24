SRC := $(wildcard src/*.c)
BIN := $(SRC:src/%.c=build/%)
COMPILE = gcc

build/%: src/%.c
	mkdir -p $(dir $@)
	$(COMPILE) $< -o $@

bin: $(BIN)

clean:
	rm -rf build

.PHONY: bin clean