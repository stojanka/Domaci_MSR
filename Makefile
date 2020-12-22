
ifneq ($(KERNELRELEASE),)
	obj-m := stred.o
else
# Ako KERNELDIR nije definisan postavi ga na ovu vrednost
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
# Podesi promenljivu PWD na vrednost trenutnog direktorijuma
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	gcc stred_app.c -o stred_app.out
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f *~ *.out

endif
