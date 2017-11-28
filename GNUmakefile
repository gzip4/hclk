CPPFLAGS += -I/usr/local/include
LDFLAGS  += -L/usr/local/lib -lX11 -lm

all: hclk

hclk: hclk.o
	$(CC) $(LDFLAGS) -o $@ $<

clean:
	$(RM) -f *.[oasid]

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
