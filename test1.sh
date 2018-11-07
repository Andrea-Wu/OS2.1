rmmod driver
make
insmod ./driver.ko

gcc cmd_line_test.c
./a.out create "hello"
./a.out create "world"
./a.out create "bellow"
./a.out create ""

./a.out delete 1
./a.out delete 2
./a.out delete 3
./a.out delete 4

