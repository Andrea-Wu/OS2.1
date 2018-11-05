rmmod driver
make
insmod ./driver.ko

gcc cmd_line_test.c
./a.out create "hello"
./a.out create "world"

./a.out get_key 1
./a.out get_key 2
