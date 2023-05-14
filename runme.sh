
make
for ((i=1; i<=10; i++)) 
do ./t2 & sleep 1; done
sleep 1
killall -SIGINT t2
sleep 1

cat stat.txt