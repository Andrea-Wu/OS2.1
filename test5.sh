rmmod driver
make
insmod ./driver.ko

gcc cmd_line_test.c
./a.out create "HOUGHTON"
./a.out create "world"

./a.out get_key 1

./a.out encrypt 1 "MICHIGAN TECHNOLOGICAL UNIVERSITY"
./a.out decrypt 1 "TWWNPZOAASWNUHZBNWWGSNBVCSLYPMM"
