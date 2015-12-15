/* Copyright (c) 2007-2009, Stanford University
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*        documentation and/or other materials provided with the distribution.
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
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <mcheck.h>

#include "stddefines.h"

#define DEF_NUM_POINTS 10000
#define DEF_NUM_MEANS 10
#define DEF_DIM 3
#define DEF_GRID_SIZE 1000

#define false 0
#define true 1

typedef struct {
	int num_points; // number of vectors
	int dim;        // Dimension of each vector
	int num_means; // number of clusters
	int grid_size; // size of each dimension of vector space
	int modified;
} SETUP_T;

/** parse_args()
 *  Parse the user arguments
 */
void parse_args(int argc, char **argv, SETUP_T *setup) {    
    int c;
    extern char *optarg;
    extern int optind;
    
    setup->num_points = DEF_NUM_POINTS;
    setup->num_means = DEF_NUM_MEANS;
    setup->dim = DEF_DIM;
    setup->grid_size = DEF_GRID_SIZE;
    
    while ((c = getopt(argc, argv, "d:c:p:s:")) != EOF) 
    {
        switch (c) {
            case 'd':
                setup->dim = atoi(optarg);
                break;
            case 'c':
                setup->num_means = atoi(optarg);
                break;
            case 'p':
                setup->num_points = atoi(optarg);
                break;
            case 's':
                setup->grid_size = atoi(optarg);
                break;
            case '?':
                printf("Usage: %s -d <vector dimension> -c <num clusters> -p <num points> -s <grid size>\n", argv[0]);
                exit(1);
        }
    }
    
    if (setup->dim <= 0 || setup->num_means <= 0 || setup->num_points <= 0 || setup->grid_size <= 0) {
        printf("Illegal argument value. All values must be numeric and greater than 0\n");
        exit(1);
    }
    
    printf("Dimension = %d\n", setup->dim);
    printf("Number of clusters = %d\n", setup->num_means);
    printf("Number of points = %d\n", setup->num_points);
    printf("Size of each dimension = %d\n", setup->grid_size);    
}

/** generate_points()
 *  Generate the points
 */
void generate_points(int *pts, int size, SETUP_T *setup) {    
    int i, j;
    //printf("size=%d\tdim=%d\tsizeof(pts)=%u\n",size, dim, sizeof(pts));
    for (i=0; i<size; i++) {
        for (j=0; j<setup->dim; j++) {
	    //printf("i=%d\tj=%d\t(i*dim)+j=%d\n", i, j, (i*dim)+j);
            pts[(i*setup->dim)+j] = rand() % setup->grid_size;
        }
    }
}
/** get_sq_dist()
 *  Get the squared distance between 2 points
 */
static inline unsigned int get_sq_dist(int *v1, int v1_row, int *v2, int v2_row, SETUP_T *setup) {
    int i;
    
    unsigned int sum = 0;
    for (i = 0; i < setup->dim; i++) {
        sum += ((v1[v1_row*setup->dim+i] - v2[v2_row*setup->dim+i]) * (v1[v1_row*setup->dim+i] - v2[v2_row*setup->dim+i])); 
    }
    return sum;
}

/** add_to_sum()
 *	Helper function to update the total distance sum
 */
void add_to_sum(int *sum, int *point, int point_row, SETUP_T *setup) {
    int i;
    
    for (i = 0; i < setup->dim; i++) {
        sum[i] += point[point_row*setup->dim+i];    
    }    
}

/** find_clusters()
 *  Find the cluster that is most suitable for a given set of points
 */
void find_clusters(int *points, int *means, int *clusters, SETUP_T *setup) {
    int i, j;
    unsigned int min_dist, cur_dist;
    int min_idx;

    for (i = 0; i < setup->num_points; i++) {
        min_dist = get_sq_dist(points, i, means, 0, setup);
        min_idx = 0; 
        for (j = 1; j < setup->num_means; j++) {
            cur_dist = get_sq_dist(points, i, means, j, setup);
            if (cur_dist < min_dist) {
                min_dist = cur_dist;
                min_idx = j;    
            }
        }
        
        if (clusters[i] != min_idx) {
            clusters[i] = min_idx;
            setup->modified = true;
        }
    }    
}

/** calc_means()
 *  Compute the means for the various clusters
 */
void calc_means(int *points, int *means, int *clusters, SETUP_T *setup) {
    int i, j, grp_size;
    int *sum;
    
    sum = (int *)malloc(setup->dim * sizeof(int));
    
    for (i = 0; i < setup->num_means; i++) {
        //memset(sum, 0, setup->dim * sizeof(int));
	for(j=0; j<setup->dim; j++) 
		sum[j] = 0;
        grp_size = 0;
        
        for (j = 0; j < setup->num_points; j++) {
            if (clusters[j] == i) {
                add_to_sum(sum, points, j, setup);
                grp_size++;
            }    
        }
        
        for (j = 0; j < setup->dim; j++) {
            //dprintf("div sum = %d, grp size = %d\n", sum[j], grp_size);
            if (grp_size != 0) { 
                means[i*setup->dim+j] = sum[j] / grp_size;
            }
        }         
    }
    free(sum);
}

/** dump_matrix()
 *  Helper function to print out the points
 */
void dump_matrix(int *vals, int rows, int cols) {
    int i, j;
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            dprintf("%5d ",vals[i*cols+j]);
        }
        dprintf("\n");
    }
}

/** 
* This application groups 'num_points' row-vectors (which are randomly
* generated) into 'num_means' clusters through an iterative algorithm - the 
* k-means algorith 
*/
int main(int argc, char **argv) {
    //mtrace();
    int *points;
    int *means;
    int *clusters;
    int i;
    SETUP_T *setup = (SETUP_T *)malloc(sizeof(SETUP_T));

    parse_args(argc, argv, setup);
    points = (int *)malloc(setup->num_points * setup->dim * sizeof(int));

    dprintf("Generating points\n");
    generate_points(points, setup->num_points, setup);    

    means = (int *)malloc(setup->num_means * setup->dim * sizeof(int));
    dprintf("Generating means\n");
    generate_points(means, setup->num_means, setup);
    
    clusters = (int *)malloc(sizeof(int) * setup->num_points);
    //memset(clusters, -1, sizeof(int) * setup->num_points);
    for(i=0; i<setup->num_points; i++) 
	clusters[i] = -1;
    setup->modified = true;
        
    dprintf("\n\nStarting iterative algorithm\n");
    
    while (setup->modified) {
        setup->modified = false;
        dprintf(".");
        
        find_clusters(points, means, clusters, setup);
        calc_means(points, means, clusters, setup);
    }
    
    dprintf("\n\nFinal Means:\n");
    dump_matrix(means, setup->num_means, setup->dim);
    
    dprintf("Cleaning up\n");
    free(means);
    free(points);
    free(setup);
    //muntrace();
    return 0;  
}
