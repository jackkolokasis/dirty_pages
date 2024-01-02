obj-m += dirty_pages.o

all:
	make -C /lib/modules/6.5.0-14-generic/build M=$(PWD) modules

clean:
	make -C /lib/modules/6.5.0-14-generic/build M=$(PWD) clean
