.PHONY: build qemu phone clean

PHONE ?= 127.0.0.1
EMULATOR ?= basalt

build:
	pebble build

qemu: build
	pebble install --emulator $(EMULATOR) --logs

phone: build
	pebble install --phone $(PHONE) --logs

clean:
	pebble clean
	pebble kill
