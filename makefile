stree: stree.c

stree.c:
	gcc -c stree.c -o stree

clean:
	-rm -f stree
	
.PHONY: clean