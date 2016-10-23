#Makefile
pir: main.o grovepi.o
	cc -o pir main.o grovepi.o

main.o: main.c
	cc -c main.c

grovepi.o: grovepi.c grovepi.h
	cc -c grovepi.c
