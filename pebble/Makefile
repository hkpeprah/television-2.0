.PHONY: build qemu phone clean

PHONE ?= 127.0.0.1

build:
	pebble build

qemu: build
	pebble install --logs

phone: build
	pebble install --phone $(PHONE) --logs

clean:
	pebble clean
