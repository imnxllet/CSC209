all : tests 

mytest : mytest.o smalloc.o testhelpers.o
	gcc -Wall -g -o mytest mytest.o smalloc.o testhelpers.o
	
tests : mytest.o simpletest.o smalloc.o testhelpers.o
	gcc -Wall -g -o mytest mytest.o smalloc.o testhelpers.o
	gcc -Wall -g -o simpletest simpletest.o smalloc.o testhelpers.o
	./simpletest
	./mytest
	
%.o : %.c smalloc.h
	gcc -Wall -g -c $<
	
clean : 
	rm simpletest *.o
	rm mytest *.o
