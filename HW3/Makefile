obj-m+=ebbcharRead.o
obj-m+=ebbcharWrite.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules 
	$(CC) testebbchar.c -o test
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm test
