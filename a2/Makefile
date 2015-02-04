all : simpletest

simpletest : simpletest.o smalloc.o testhelpers.o
	gcc -Wall -g -o simpletest simpletest.o smalloc.o testhelpers.o
	
%.o : %.c smalloc.h
	gcc -Wall -g -c $<
	
clean : 
	rm simpletest *.o
