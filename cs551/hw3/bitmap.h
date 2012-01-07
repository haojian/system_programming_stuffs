/**************Statement of Policy****************************/
// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Haojian Jin
/*************************************************************/

#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdio.h>   
#include <stdlib.h>   
#include <string.h>   
   
int bitmap_init(int size, unsigned char *g_bitmap )
{
	g_bitmap = (unsigned char *)malloc((size/8 + 1) * sizeof(unsigned char));
	memset(g_bitmap, 0, (size/8+1));
	return 1;
}

int bitmap_set(int index, unsigned char *g_bitmap)
{
	int quo = index / 8;
	int remainder = (8- (index+1) % 8)%8;
	unsigned char x = (0x1<<remainder);
	g_bitmap[quo]|= x;
	return 1;
}

int bitmap_get(int index, unsigned char *g_bitmap)
{
	int quo = index / 8;
	int remainder =(8- (index+1) % 8)%8;
	unsigned char x = (0x1<<remainder);
	unsigned char res;
	res = g_bitmap[quo] & x;
	return res>0 ? 1:0;
}

int bitmap_free(unsigned char *g_bitmap)
{
	free(g_bitmap);
}

#endif
