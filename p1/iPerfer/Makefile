CXX = gcc
all: iPerfer

SOURCES = iPerfer.c
iPerfer: $(SOURCES)
	$(CXX) -g -O2 -std=c11 $(SOURCES) -o iPerfer

clean:
	rm -rf iPerfer *.dSYM

.PHONY: clean
