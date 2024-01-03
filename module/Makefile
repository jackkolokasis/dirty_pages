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
	make -C /lib/modules/6.5.0-14-generic/build M=$(PWD) modules

clean:
	make -C /lib/modules/6.5.0-14-generic/build M=$(PWD) clean

load:
	sudo insmod $(TARGET).ko

unload:
	sudo rmmod $(TARGET)
