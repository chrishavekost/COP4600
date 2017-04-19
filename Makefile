obj-m+=ebbchar.o, ebbcharRead.o, ebbcharWrite.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules 
	$(CC) testebbchar.c -o test
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm test