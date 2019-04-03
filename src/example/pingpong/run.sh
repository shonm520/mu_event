rm  pingpongsvr   
rm  pingpongcli   
echo "rm"
gcc -o pingpongsvr server.c ../../*.c -lpthread  -g -I ../../ 
sleep 1    
gcc -o pingpongcli client.c ../../*.c -lpthread  -g -I ../../ 