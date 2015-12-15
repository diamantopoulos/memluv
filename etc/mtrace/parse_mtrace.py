"""
Parsing a mtrace log file and append to timeline-footprint format
"""
from numpy import *
import numpy as np
import glob
import os
import linecache
import csv

# 1038 bytes is the size of a heap log file with no heap activity (only heap info)

def ValidHeapFile(fpath):
	header_lines=1
	with open(fpath) as f:
		lines = len(list(f))
	return True if os.path.isfile(fpath) and lines > header_lines else False

print ("INFO: --------------------- \nINFO: Parsing mtrace logs \nINFO: ---------------------")

mtrace_files = glob.glob("/tmp/mtrace*.txt")
mtraces=len(mtrace_files)
print ("INFO: Total mtrace logs found:", mtraces)

colours=['b','g','r','c','m','y','k']

elapsed_time=208000
#with plt.xkcd():
total_bytes_allocated=0
index=0
fout = open("/tmp/mtrace.out",'w')
lines_parsed=0
event_time=0

#Heaps log parsing
for cur_mtrace in sorted(mtrace_files):
	if ValidHeapFile(cur_mtrace):
            fin = open(cur_mtrace,'r')
            total_lines = len(fin.readlines())
            tic=elapsed_time/(total_lines-3)
            print ("total_lines = ", total_lines, "tic = ", tic)
            fin.close()
            fin = open(cur_mtrace,'r')
            for line in fin:
                line = line.rstrip().split(' ')
                #print ("length(line) = ", len(line), "index=", index)
                if lines_parsed>=2 and lines_parsed<total_lines-1:
                    sign = line[2]
                    if sign == '+':
                        cur_bytes = line[4]
                        cur_bytes_dec = int(cur_bytes, 16)
                        total_bytes_allocated = total_bytes_allocated + cur_bytes_dec
                        #print ("INFO: Adding ", cur_bytes_dec, "bytes", "total_bytes_allocated=", total_bytes_allocated)
                    elif sign == '-':
                        total_bytes_allocated = total_bytes_allocated - cur_bytes_dec
                        #print ("INFO: Subtracting ", cur_bytes_dec, "bytes", "total_bytes_allocated=", total_bytes_allocated)
                    else:
                        print ("ERROR: Unknown sign", sign, "Aborting...")
                        __exit__
                    event_time=event_time+tic
                    fout.write(str(index)+" "+str(event_time)+" "+str(total_bytes_allocated)+"\n")
                    index=index+1
                else:
                     print ("WARNING: Ignoring this line", line)
                lines_parsed=lines_parsed+1
	else:
		print ("INFO: Current mtrace path :", cur_mtrace, "-> Skipping empty file")

fin.close()
fout.close()



