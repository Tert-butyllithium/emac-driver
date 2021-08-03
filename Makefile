
CROSS_COMPILE := /sdk/lichee/brandy-2.0/tools/toolchain/riscv64-linux-x86_64-20200528/bin/riscv64-unknown-linux-gnu-
CC := $(CROSS_COMPILE)gcc

CFLAGS = -nostdlib -static -mcmodel=medany -g 

link_script = drv_net.lds

src := main.c
src += common.c

src += misc/vsnprintf.c
src += misc/string.c
src += misc/timer.c
src += misc/time.c
src += misc/nvedit.c

src += net/checksum.c
src += net/arp.c
src += net/eth_legacy.c 
src += net/eth_common.c
src += net/net.c
src += net/ping.c

src += net/miiphyutil.c
src += net/phy/phy.c

src += net/emac/sunxi_geth.c

objs := $(src:%.c=%.o)


# main: $(objs)

# all:
# 	(CROSS_COMPILE)gcc $(CFLAGS) $(src) -o main

all: drv_net

drv_net: drv_net.bin
	$(CROSS_COMPILE)objcopy -O binary --set-section-flags .bss=alloc,load,contents $< $@
	cp $@ $(target_dir)

drv_net.bin: $(objs)
	$(CROSS_COMPILE)gcc $(CFLAGS) $(objs) -T $(link_script) -o $@

clean:
	-@rm drv_net drv_net.bin