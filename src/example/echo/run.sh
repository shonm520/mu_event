rm -f echosvr

gcc -o echosvr *.c ../../*.c -lpthread -g -I ../../ 