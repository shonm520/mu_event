src = $(wildcard ./*c)
obj = $(patsubst ./%.c, ./%.o, $(src))

LIB = libmuevent.a
CC = gcc
CFLAGS := -g -lpthread 



%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

$(LIB):$(obj)
	rm -f $@
	ar cr $@ $(obj)
	rm -f $(obj) 



.PHONY:clean
clean:
	rm -f $(obj) $(LIB) 
.PHONY:cleantemp
cleantemp:
	rm -f $(obj) 