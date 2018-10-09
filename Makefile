CC=gcc
TAR=assembler

all: assembler

%.o: %.c
	$(CC) -c $^

$(TAR): assembler.o
	$(CC) -o $@ $^

.PHONY: clean test help

clean:
	rm -rf *.o $(TAR)

help:
	@echo "The following options are provided with Make\n\t$$ make \t\t# build assembler\n\t$$ make clean \t# clean the build\n\t$$ make test \t# test cases"

test: assembler test_1 test_2 test_3 test_4 test_5

test_1:
	@echo "Testing example1"; \
	diff -Naur sample_input/example1.o sample_output/example1.o; \
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_2:
	@echo "Testing example2_mod"; \
	diff -Naur sample_input/example2_mod.o sample_output/example2_mod.o; \
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_3:
	@echo "Testing example3"; \
	diff -Naur sample_input/example3.o sample_output/example3.o; \
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_4:
	@echo "Testing example4"; \
	diff -Naur sample_input/example4.o sample_output/example4.o; \
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_5:
	@echo "Testing example5"; \
	diff -Naur sample_input/example5.o sample_output/example5.o; \
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output"; fi
