CFLAGS = -Wall -Werror -g -O0

TARGETS = jiovec
all: $(TARGETS)

jiovec: jiovec.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
