test_jwd : testMain.o jwd1797.o
	gcc -o test_jwd testMain.o jwd1797.o
testMain.o : testMain.c jwd1797.h 
	gcc -c testMain.c
jwd1797.o : jwd1797.c jwd1797.h
	gcc -c jwd1797.c
clean :
	rm test_jwd testMain.o jwd1797.o
