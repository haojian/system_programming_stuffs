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
	}range;

range *tested_list = NULL;
int range_num_counter = 0;
char result[100];
   
void range_list_init()
{
	tested_list = (range *)malloc(100 * sizeof(range));
	memset(tested_list, 0, 100 * sizeof(range));
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
void add_tested_range(range _newrange)
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

int istested(long _testnum)
{
	int j;
	for(j=0; j<range_num_counter;j++)
	{
		//printf("start point: %ld; end point: %ld\n", (*(tested_list+j)).start_point, (*(tested_list+j)).end_point);
		if(_testnum>=(*(tested_list+j)).start_point && _testnum<=(*(tested_list+j)).end_point)
			return 1;
	}
	return 0;
}

range *get_range_list(long _start_point, long _number_tobetested)
{
	range *return_list;
	int returnRangeCounter = 0;
	long cur_start_point = _start_point;
	long cur_end_point = _number_tobetested;
	long j=0;
	int newrange = 1;
	if(range_num_counter != 0)
	{
	printf("eeor1");
	while(j<=_number_tobetested)
	{
		int index=0;
		if(!istested(cur_start_point+index))
		{
			if(newrange)
			{
				(*(return_list+returnRangeCounter)).start_point = cur_start_point+index;
				newrange = 0;
			}
			j++;
		}
		else
		{
			if(!newrange)
			{
				(*(return_list+returnRangeCounter)).end_point= cur_start_point+index-1;
				returnRangeCounter++;
			}
			
		}
		index++;
	}
		return return_list;
	}
	else
	{
		(*return_list).start_point = _start_point;
		(*return_list).end_point = _start_point+ _number_tobetested;
		return return_list;
	}

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
