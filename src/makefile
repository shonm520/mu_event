src = $(wildcard ./*c)
obj = $(patsubst ./%.c, ./%.o, $(src))

target = svr
CC = gcc
CFLAGS = -g -lpthread

$(target):$(obj)
	$(CC) $^ -o $@ $(CFLAGS)
	rm -f $(obj) 


%.o:%.c
	$(CC) -c $< -o $@ 

.PHONY:clean
clean:
	rm -f $(obj) $(target) 
.PHONY:cleantemp
cleantemp:
	rm -f $(obj) 