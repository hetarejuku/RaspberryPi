#Makefile
adc: main.o ABE_ADCDACPi.o
	cc -o adc main.o ABE_ADCDACPi.o

main.o: main.c
	cc -c main.c

ABE_ADCDACPi.o: ABE_ADCDACPi.c
	cc -c ABE_ADCDACPi.c
