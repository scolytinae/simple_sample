all: chunker

chunker:
	g++ chunker.cpp -o chunker.out

clean:
	rm *.out