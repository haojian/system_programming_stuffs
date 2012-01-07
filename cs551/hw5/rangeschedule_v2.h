/**************Statement of Policy****************************/
// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Haojian Jin
/*************************************************************/

#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdio.h>   
#include <stdlib.h>   
#include <string.h>  

typedef struct{
		long start_point;
		long end_point;
	}range_info;

range_info *tested_list = NULL;
int range_num_counter = 0;
int total_range_info = 0;
   
void range_list_init()
{
	tested_list = (range_info *)malloc(100 * sizeof(range_info));
	memset(tested_list, 0, 100 * sizeof(range_info));
}

long maxof4(long a, long b, long c, long d)
{
	long max =a;
	if(b>max)
		max = b;
	if(c>max)
		max = c;
	if(d>max)
		max =d;
	return max;
}

long minof4(long a, long b, long c, long d)
{
	long min = a;
	if(b<min)
		min = b;
	if(c<min)
		min = c;
	if(d<min)
		min = d;
	return min;
}

//merge and add new range
void add_tested_range(range_info _newrange)
{
	int j;
	if(range_num_counter == 0)
	{
		range_num_counter++;
		*tested_list = _newrange;
	}
	for(j =0; j<range_num_counter; j++)
	{
		if(_newrange.start_point>=(*(tested_list+j)).start_point && _newrange.start_point<=(*(tested_list+j)).end_point || _newrange.end_point>=(*(tested_list+j)).start_point && _newrange.end_point<=(*(tested_list+j)).end_point)
		{
			(*(tested_list+j)).start_point =minof4(_newrange.start_point, (*(tested_list+j)).start_point, _newrange.end_point, (*(tested_list+j)).end_point);
			(*(tested_list+j)).end_point =maxof4(_newrange.start_point, (*(tested_list+j)).start_point, _newrange.end_point, (*(tested_list+j)).end_point);
		}
		else
		{
			range_num_counter++;
			*(tested_list+j) = _newrange;
		}
			
	}
}

range_info newRange(long _start_point, long _rangesize)
{
	range_info result;
	long start =_start_point;
	long end = _start_point + _rangesize;
	int j;
	for(j=0; j< range_num_counter; j++)
	{
		
		if(start >= (*(tested_list+j)).start_point && end<=(*(tested_list+j)).end_point){
			start = (*(tested_list+j)).end_point;
			end = (*(tested_list+j)).end_point + _rangesize;
		}
		else if (end < (*(tested_list+j)).start_point || start  > (*(tested_list+j)).end_point)
		{
			//do nothing
		}
		else if(start <(*(tested_list+j)).start_point )
		{
			end = (*(tested_list+j)).start_point;
		}
		
	}
	result.start_point = start;
	result.end_point = end;
	add_tested_range(result);
	return result;
}



void range_list_free()
{
	free(tested_list );
}

void print_rangelist()
{
	int j;
	for(j=0; j<range_num_counter;j++)
	{
		printf("start point: %ld; end point: %ld\n", (*(tested_list+j)).start_point, (*(tested_list+j)).end_point);
	}
}

#endif
