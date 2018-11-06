rmmod driver
make
insmod ./driver.ko

gcc cmd_line_test.c
./a.out create "HOUGHTON"
./a.out create "world"

./a.out get_key 1
./a.out get_key 2

./a.out encrypt 1 "MICHIGAN TECHNOLOGICAL UNIVERSITY"
./a.out encrypt 2 "YON YONSEN FROM WISCONSIN"
