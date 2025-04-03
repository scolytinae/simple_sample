all: chunker regex_replacer

chunker:
	g++ chunker.cpp -o chunker.out

regex_replacer:
	g++ regex_replacer.cpp -o regex_replacer.out

clean:
	rm *.out