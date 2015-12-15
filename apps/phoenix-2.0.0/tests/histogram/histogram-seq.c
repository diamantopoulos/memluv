/* Copyright (c) 2007-2009, Stanford University
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Stanford University nor the names of its 
*       contributors may be used to endorse or promote products derived from 
*       this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include "../../include/stddefines.h"

#define IMG_DATA_OFFSET_POS 10
#define BITS_PER_PIXEL_POS 28

#define BYPASS_IMG 0

int swap;      // to indicate if we need to swap byte order of header information


/* test_endianess
 *
 */
void test_endianess() {
   unsigned int num = 0x12345678;
   char *low = (char *)(&(num));
   if (*low ==  0x78) {
      dprintf("No need to swap\n");
      swap = 0;
   }
   else if (*low == 0x12) {
      dprintf("Need to swap\n");
      swap = 1;
   }
   else {
      printf("Error: Invalid value found in memory\n");
      exit(1);
   } 
}

/* swap_bytes
 *
 */
void swap_bytes(char *bytes, int num_bytes) {
   int i;
   char tmp;
   
   for (i = 0; i < num_bytes/2; i++) {
      dprintf("Swapping %d and %d\n", bytes[i], bytes[num_bytes - i - 1]);
      tmp = bytes[i];
      bytes[i] = bytes[num_bytes - i - 1];
      bytes[num_bytes - i - 1] = tmp;   
   }
}

void histogram(int start, int end, char* fdata, int* blue, int* green, int* red);

void histogram(int start, int end, char* fdata, int* blue, int* green, int* red) {
	int i;
	printf("DEBUG: start=%d end=%d \n", start, end);
	for (i=start; i < end; i+=3) {      
		unsigned char *val = (unsigned char *)&(fdata[i]);
		blue[*val]++;
	
		val = (unsigned char *)&(fdata[i+1]);
		green[*val]++;
      
		val = (unsigned char *)&(fdata[i+2]);
		red[*val]++;   
	}
}

int main(int argc, char *argv[]) {
      
   int i;
   int fd;
   struct stat finfo;
   char * fname;
   int red[256];
   int green[256];
   int blue[256];
   int data_size;
   // Make sure a filename is specified
#if BYPASS_IMG == 0
	char *fdata;
	if (argc == 2) {
		fname = argv[1];  
		//fname = "/home/diamantd/projects/memluv/apps/phoenix-2.0.0/tests/histogram/datafiles/beach.bmp";
		printf("INFO: Using image %s\n", argv[1]);
	}
	else {
		printf("USAGE: %s <bitmap filename>\n", argv[0]);
		exit(1);
	}
#else
	char fdata[1000];
	bypass_image = 1;
	printf("INFO: Bypassing image\n");
	fname = NULL;
#endif
	
#if BYPASS_IMG == 0 
	// Read in the file
	CHECK_ERROR((fd = open(fname, O_RDONLY)) < 0);
	// Get the file info (for file length)
	CHECK_ERROR(fstat(fd, &finfo) < 0);
	// Memory map the file
	CHECK_ERROR((fdata = mmap(0, finfo.st_size + 1, 
	PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == NULL);
	data_size = (int)finfo.st_size;
#else
	data_size = 300;
	for (i=0; i < data_size; i+=3) {
		fdata[i] = 43;
	}
	fdata[0] = 'B';
	fdata[1] = 'M';
	fdata[BITS_PER_PIXEL_POS]=24;
#endif
	   
   
   if ((fdata[0] != 'B') || (fdata[1] != 'M')) {
      printf("File is not a valid bitmap file. Exiting\n");
      exit(1);
   }
   
   test_endianess();    // will set the variable "swap"
   
   unsigned short *bitsperpixel = (unsigned short *)(&(fdata[BITS_PER_PIXEL_POS]));
   if (swap) {
      swap_bytes((char *)(bitsperpixel), sizeof(*bitsperpixel));
   }
   if (*bitsperpixel != 24) {    // ensure its 3 bytes per pixel
      printf("Error: Invalid bitmap format - ");
      printf("This application only accepts 24-bit pictures. Exiting\n");
      exit(1);
   }
   
   unsigned short *data_pos = (unsigned short *)(&(fdata[IMG_DATA_OFFSET_POS]));
   if (swap) {
      swap_bytes((char *)(data_pos), sizeof(*data_pos));
   }
   
   int imgdata_bytes = data_size - (int)(*(data_pos));
   printf("This file has %d bytes of image data, %d pixels\n", imgdata_bytes,
                                                            imgdata_bytes / 3);
                                                            
   printf("Starting sequential histogram\n");                                                            

   
   memset(&(red[0]), 0, sizeof(int) * 256);
   memset(&(green[0]), 0, sizeof(int) * 256);
   memset(&(blue[0]), 0, sizeof(int) * 256);
 
   histogram(*data_pos, data_size, fdata, blue, green, red);

/*   for (i=*data_pos; i < data_size; i+=3) {      
      unsigned char *val = (unsigned char *)&(fdata[i]);
      blue[*val]++;
      
      val = (unsigned char *)&(fdata[i+1]);
      green[*val]++;
      
      val = (unsigned char *)&(fdata[i+2]);
      red[*val]++;   
   }
*/ 
   dprintf("\n\nBlue Green Red\n");
   dprintf("----------\n\n");
   for (i = 0; i < 256; i++) {
      dprintf("%d - %d %d %d\n", i, blue[i], green[i], red[i]);        
   }

#if BYPASS_IMG == 0
	CHECK_ERROR(munmap(fdata, data_size + 1) < 0);
	CHECK_ERROR(close(fd) < 0);
#endif
   return 0;
}
