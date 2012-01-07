/**************Statement of Policy****************************/
// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Haojian Jin
/*************************************************************/

#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdio.h>   
#include <stdlib.h>   
#include <string.h>  
#include <pthread.h>

unsigned char *g_bitmap = NULL;  
static pthread_mutex_t bitmap_mtx= PTHREAD_MUTEX_INITIALIZER; 
   
int bitmap_init(int size)
{
	g_bitmap = (unsigned char *)malloc((size/8 + 1) * sizeof(unsigned char));
	memset(g_bitmap, 0, (size/8+1));
	return 1;
}

int bitmap_set(int index)
{
	int quo = index / 8;
	int remainder = (8- (index+1) % 8)%8;
	unsigned char x = (0x1<<remainder);
	pthread_mutex_lock(&bitmap_mtx);
	g_bitmap[quo]|= x;
	pthread_mutex_unlock(&bitmap_mtx);
	return 1;
}

int bitmap_get(int index)
{
	int quo = index / 8;
	int remainder =(8- (index+1) % 8)%8;
	unsigned char x = (0x1<<remainder);
	unsigned char res;
	pthread_mutex_lock(&bitmap_mtx);
	res = g_bitmap[quo] & x;
	pthread_mutex_unlock(&bitmap_mtx);
	return res>0 ? 1:0;
}

int bitmap_free()
{
	free(g_bitmap);
}

#endif
