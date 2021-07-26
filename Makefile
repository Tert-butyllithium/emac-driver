
CROSS_COMPILE := /sdk/lichee/brandy-2.0/tools/toolchain/riscv64-linux-x86_64-20200528/bin/riscv64-unknown-linux-gnu-
KDIR := /sdk/lichee/linux-5.4
PWD := $(shell pwd)
ccflags-y := -I./include
obj-m := main.o 

main-y += common.o
main-y += net/checksum.o
main-y += net/arp.o
main-y += net/eth_legacy.o 
main-y += net/eth_common.o
main-y += net/net.o
main-y += net/ping.o

main-y += net/miiphyutil.o
main-y += net/phy/phy.o

main-y += net/emac/sunxi_geth.o

all:
	make -C $(KDIR) M=$(PWD) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=riscv modules
clean:
	make -C $(KDIR) M=$(PWD) clean
