# You can compile with either gcc or g++
# CC = g++
CC = gcc
CFLAGS = -I. -Wall -lm -DNDEBUG -g
# disable the -DNDEBUG flag for the printing the freelist
OPTFLAG = -O2
DEBUGFLAG = -g

all: dmm.o basicdmmtest

dmm.o: dmm.c
	$(CC) $(CFLAGS) -c dmm.c 

basicdmmtest: basicdmmtest.c dmm.o
	$(CC) $(CFLAGS) -o basicdmmtest basicdmmtest.c dmm.o

clean:
	rm -f *.o a.out
