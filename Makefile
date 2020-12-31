test_jwd : testMain.o jwd1797.o utility_functions.o
	gcc -o test_jwd testMain.o jwd1797.o utility_functions.o
testMain.o : testMain.c jwd1797.h
	gcc -c testMain.c
jwd1797.o : jwd1797.c jwd1797.h utility_functions.h
	gcc -c jwd1797.c
utility_functions.o : utility_functions.c utility_functions.h
		gcc -c utility_functions.c
clean :
	rm test_jwd testMain.o jwd1797.o utility_functions.o
