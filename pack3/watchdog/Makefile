MOD_NAME := xt_watchdog
MOD_FULL_NAME := $(MOD_NAME).ko
obj-m := $(MOD_NAME).o

PWD    := $(shell pwd)

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean || true

insert:
	insmod	$(MOD_FULL_NAME)

remove:
	rmmod	$(MOD_FULL_NAME) || true

install:
	install -D $(MOD_FULL_NAME) /lib/modules/$(shell uname -r)/updates/$(MOD_FULL_NAME)
	depmod -a
