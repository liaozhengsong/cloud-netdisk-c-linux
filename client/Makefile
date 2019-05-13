CFLAGS = -g -lssl -lcrypt -lcrypto
source:= $(wildcard *.c)
obj:= $(patsubst %.c,%.o,$(source))
main: $(obj)
	gcc -o main $(obj) $(CFLAGS)
.PHONY:clean
clean:
	rm -rf $(obj) main
