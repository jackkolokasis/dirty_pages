##############################################
#
# Author: 			Iacovos G. Kolokasis
# Date:					2014-03-24
# Description:	Makefile for trace_dirty_pages
# 						 	kernel module
#
#############################################

TARGET = trace_dirty_pages

obj-m += $(TARGET).o

all:
	make -C /lib/modules/6.1.68/build M=$(PWD) modules

clean:
	make -C /lib/modules/6.1.68/build M=$(PWD) clean

load:
	sudo insmod $(TARGET).ko && \
		sudo chown -R kolokasis /dev/trace_dirty_pages

unload:
	sudo rmmod $(TARGET)
