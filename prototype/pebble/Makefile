# Makefile: to list out common commands for this pebble app
PHONE_IP=192.168.1.39 # this varies

build: src/pebble.c
	pebble build

run: build
	pebble install -v --phone $(PHONE_IP)

debug:
	pebble install -v --emulator chalk

logs:
	pebble logs --phone $(PHONE_IP)
