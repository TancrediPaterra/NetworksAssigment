CXX = gcc
all: iPerfer

SOURCES = iPerfer.c
iPerfer: $(SOURCES)
	$(CXX) -g -Wall -fsanitize=address $(SOURCES) -o iPerfer

clean:
	rm -rf iPerfer *.dSYM

.PHONY: clean
