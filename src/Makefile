

CC = gcc
CFLAGS = -O2 -Wall -ggdb -lm
default: clean test_memluv run

clean:
	rm -rf *.o test_memluv 

run:
	./test_memluv
	
test_memluv: clean util.o memluv.o test_memluv.o
	$(CC) $(CFLAGS) -o $@ util.o memluv.o test_memluv.o
	
vivado:
	vivado_hls -f hls/generic.tcl
	
vsim:
	vsim ./prj/solution1/sim/vhdl/yadmm.wlf &
	
waves:
	simvision ./prj/solution1/sim/vhdl/yadmm.vcd &