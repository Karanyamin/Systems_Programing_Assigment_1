all: fileCompressor

fileCompressor: fileCompressor.c
	gcc -g fileCompressor.c -o fileCompressor
clean: 
	rm -f ./fileCompressor
cleanA: hcz
	rm -f ./HuffmanCodebook
hcz:
	find . -name "*.hcz" -type f -delete

	