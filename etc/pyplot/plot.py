"""
Plotting Allocated vs Requested bytes for all available HW heap logs
"""
from numpy import *
import numpy as np
import matplotlib.pyplot as plt
import glob
import os
from matplotlib.patches import ConnectionPatch
import linecache

# 1038 bytes is the size of a heap log file with no heap activity (only heap info)

def ValidHeapFile(fpath):
	header_lines=19
	with open(fpath) as f:
		lines = len(list(f))
	return True if os.path.isfile(fpath) and lines > header_lines else False

print ("INFO: --------------------- \nINFO: Plotting HW heap logs \nINFO: ---------------------")

heap_files = glob.glob("../../build/memluv_stats_heap-*.txt")
heaps=len(heap_files)
print ("INFO: Total heap logs found:", heaps)

colours=['b','g','r','c','m','y','k']

#with plt.xkcd():
time_arr = [ ]
allocated_arr = [ ]
requested_arr = [ ]

#Heaps log parsing
for cur_heap in sorted(heap_files):
	if ValidHeapFile(cur_heap):
		plt1=plt.subplot(2, 1, 1)
		time, allocated, requested = loadtxt(cur_heap, skiprows = 19, usecols = (1,5,6), unpack=True )
		print ('INFO: Current heap path :', cur_heap, '-> total actions: ', len(allocated))
		time_arr.append(time)
		allocated_arr.append(allocated)
		requested_arr.append(requested)
		line=linecache.getline(cur_heap, 9)
		max_mem = float(line.split()[4])
	else:
		print ("INFO: Current heap path :", cur_heap, "-> Skipping empty file")
plt.grid()

max_time=max([max(l) for l in time_arr])
w=int(max_time/100)

# Plotting
for i in range (0, len(time_arr)):
	plt1=plt.subplot(2, 1, 1)
	newlabel="Allocated HW Heap #"+str(i)
	plt.plot(time_arr[i], allocated_arr[i], colours[i%len(colours)] + '-', label=newlabel)
	plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3, ncol=4, mode="expand", borderaxespad=0.)
	newlabel="Requested HW Heap #"+str(i)
	plt.plot(time_arr[i], requested_arr[i], colours[i%len(colours)] + 'o', label=newlabel)
	plt.xlabel('Time (us)')
	plt.ylabel('Memory (Bytes)')
	plt2=plt.subplot(2, 1, 2)
	plt.bar(time_arr[i], allocated_arr[i], color=colours[i%len(colours)], width=w, align='center', label=newlabel ,linewidth=0.3)
	plt.xlabel('Time (us)')
	plt.ylabel('Memory (Bytes)')

plt1.axhline(y=max_mem,color='r',ls='dashed')
plt1.set_ylim([0,max_mem*1.1])
plt1.set_xlim([0,max_time*1.1])
plt1.xaxis.grid(True,'major',linewidth=1)
plt1.yaxis.grid(True,'major',linewidth=1)

plt2.axhline(y=max_mem,color='r',ls='dashed')
plt2.set_ylim([0,max_mem*1.1])
plt2.set_xlim([0,max_time*1.1])
plt2.xaxis.grid(True,'major',linewidth=1)
plt2.yaxis.grid(True,'major',linewidth=1)

plt.show()


