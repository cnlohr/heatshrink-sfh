all : heatshrink_test

CFLAGS:=-O2 -g -I.

heatshrink_test : tests/heatshrink_test.c
	gcc -o $@ $^ $(CFLAGS)

heatshrink_test_static : tests/heatshrink_test_static.c
	gcc -o $@ $^ $(CFLAGS)

heatshrink_test_static2 : tests/heatshrink_test_static2.c
	gcc -o $@ $^ $(CFLAGS)

heatshrink_test_size : tests/heatshrink_test_size.c
	gcc -o $@ $^ -I. -Os -nostdlib -Wl,-e"start" -ffunction-sections -fdata-sections -flto -Wl,--gc-sections

heatshrink/heatshrink : heatshrink/heatshrink.c heatshrink/heatshrink_encoder.c heatshrink/heatshrink_decoder.c
	gcc -o $@ $^

heatshrink : 
	git submodule update --init --recursive

test : heatshrink/heatshrink heatshrink_test heatshrink_test_static heatshrink_test_static2 heatshrink_test_size
	heatshrink/heatshrink -e -w 9 -l 5 /usr/bin/gcc gcc.hs
	./heatshrink_test
	diff gcc.check /usr/bin/gcc
	./heatshrink_test_static < gcc.hs > gcc2.check
	diff gcc2.check /usr/bin/gcc
	./heatshrink_test_static2 < gcc.hs > gcc2.check
	diff gcc2.check /usr/bin/gcc
	./heatshrink_test_size
	size heatshrink_test_size

clean :
	rm -rf heatshrink_test gcc.hs gcc.check heatshrink_test_static heatshrink_test_static2 heatshrink_test_size gcc2.check

