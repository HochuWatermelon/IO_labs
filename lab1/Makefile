NAME=driver
MAKE=make
KDIR?=/lib/modules/$(shell uname -r)/build

obj-m += $(NAME).o
$(NAME)-objs := src/_$(NAME).o

all:
	$(MAKE) -C $(KDIR) M=$(shell pwd) modules

load:
	insmod $(NAME).ko

unload:
	rmmod  $(NAME) -f

clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) clean