all: fileCompressor

firstCompressor: fileCompressor.c
	gcc -g fileCompressor.c -o fileCompressor

clean: codebook 
	find . -name "*.hcz" -type f -delete

codebook:
	rm -f ./HuffmanCodebook

	