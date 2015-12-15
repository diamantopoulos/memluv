#! /usr/bin/gnuplot

set multiplot layout 2,2


config_file='../../src/memluv.h'


if (!exists("filename1")) filename1='../../build/memluv_stats_heap-0.txt' {
	maxm=system("grep 'Total Size' '../../build/memluv_stats_heap-0.txt' | awk '{print $5}'")
	print("maxm=%d", maxm)
	print("Printing %s", filename1)
	set size 0.5,0.5
	set xlabel "Time (us)"
	set ylabel "Memory (Bytes)"
	set title "Runtime memory footprint"
	set tmargin 0
	set bmargin 0
	plot filename1 every ::1 using 2:6 with lines title 'Bytes Allocated', \
	filename1 every ::1 using 2:7 title 'Bytes Requested'  pt 6 lt -1
}

if (!exists("filename2")) filename2='../../build/memluv_stats_heap-1.txt' {
	maxm=system("grep 'Total Size' '../../build/memluv_stats_heap-1.txt' | awk '{print $5}'")
	print("maxm=%d", maxm)
	print("Printing %s", filename2)
	set size 1.0,0.5
	set xlabel "Time (us)"
	set ylabel "Memory (Bytes)"
	set title "Runtime memory footprint"
	plot filename2 every ::1 using 2:6 with lines title 'Bytes Allocated', \
	filename2 every ::1 using 2:7 title 'Bytes Requested'  pt 6 lt -1
}

set arrow from 10,maxm to 4500,maxm nohead lt 3 lc 3

pause -1 "Hit return to continue"
unset multiplot