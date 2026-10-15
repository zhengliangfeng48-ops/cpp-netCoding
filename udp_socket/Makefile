src=$(wildcard *.c)
target=$(patsubst %.c,%,$(src))

ALL:$(target)

%:%.c
	gcc $< -Wall -o $@ -g

clean:
	-rm -rf $(target)

.PHONY: clean ALL
