
CROSS_COMPILE := /sdk/lichee/brandy-2.0/tools/toolchain/riscv64-linux-x86_64-20200528/bin/riscv64-unknown-linux-gnu-
KDIR := /sdk/lichee/linux-5.4
PWD := $(shell pwd)
obj-m := emacdriver.o 

emacdriver-y := main.o
emacdriver-y += misc/nvedit.o
emacdriver-y += common.o
emacdriver-y += net/checksum.o
emacdriver-y += net/arp.o
emacdriver-y += net/eth_legacy.o 
emacdriver-y += net/eth_common.o
emacdriver-y += net/net.o
emacdriver-y += net/ping.o

emacdriver-y += net/miiphyutil.o
emacdriver-y += net/phy/phy.o

emacdriver-y += net/emac/sunxi_geth.o

all:
	make -C $(KDIR) M=$(PWD) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=riscv modules
	cp emacdriver.ko /sdk/out/d1-nezha/emacdriver.ko
clean:
	make -C $(KDIR) M=$(PWD) clean
