CFLAGS = -g -pthread -I /usr/include/mysql/ -lmysqlclient -lcrypto -lcrypt -lssl
DIRS := ./src/
DIRO := ./bin/
source:= $(wildcard $(DIRS)*.c)
obj:= $(patsubst $(DIRS)%.c, $(DIRO)%.o,$(source))
main: $(obj)
	gcc -o $@ $^ $(CFLAGS)
$(obj): $(source)
	@$(foreach i,$(source),\
		gcc -o $(patsubst $(DIRS)%.c,$(DIRO)%.o,$(i)) -c $(i) $(CFLAGS);\
	)
.PHONY:clean
clean:
	rm -rf $(obj) main
