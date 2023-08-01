#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <unordered_map>
#include <array>
#include <cstdint>
#include <list>
#include <utility>
#include <iostream>
#include <tuple>
#include <vector>
#include <cstring>
#include <cstdlib>

#ifndef SIM_CYCLE
#include "utils.h"
#include "bt9.h"
#include "bt9_reader.h"
#else
#include "../common/utils.h"
#endif // SIM_CYCLE

#define TABLE_WIDTH 11
#define TABLE_SIZE 32
#define WEIGHT_SIZE 3
#define NAME " (PERCEPTRON) "
int THRESHOLD=(int)pow(2,WEIGHT_SIZE-1);

class PREDICTOR
{
private:
	uint64_t history=0;
	std::vector<std::vector<int>>weight_table;
	int last_prediction=0;
	int upper=(int)pow(2,WEIGHT_SIZE-1)-1;
	int lower=-(int)pow(2,WEIGHT_SIZE-1)+1;

	uint64_t getIndex(uint64_t history,uint64_t address)
	{
		return (history^(address>>2))%TABLE_SIZE;
	}
public:
	PREDICTOR (void)
	{
	    fprintf (stderr, NAME);
	    fprintf (stderr, " (TABLE_SIZE %d) ", TABLE_SIZE);
	    fprintf (stderr, " (TABLE_WIDTH %d) ", TABLE_WIDTH);
	    fprintf (stderr, "\n");
		init ();
	}

	void init()
	{
		for (int i=0; i<TABLE_SIZE; i++)
		{
			std::vector<int> row(TABLE_WIDTH,0);
			weight_table.push_back(row);
		}
	}

	bool GetPrediction (UINT64 address)
	{
		uint64_t index=getIndex(history,address);
		int prediction=0;
		uint64_t current_history=(history<<1)+1;
		int x;
		for (int i=0; i<TABLE_WIDTH; i++)
		{
			if (current_history%2)
				x=1;
			else
				x=-1;
			prediction+=weight_table[index][i]*x;
			current_history=current_history>>1;
		}
		last_prediction=prediction;
		if (prediction>=0)
		{
			return true;
		}
		return false;
	}

	void UpdatePredictor (UINT64 address, OpType opType, bool result,
	                      bool predDir, UINT64 branchTarget)
	{
		uint64_t index=getIndex(history,address);
		uint64_t current_history=(history<<1)+1;
		int x;
		if ((last_prediction>=0)!=result || abs(last_prediction)<=THRESHOLD)
		{
			for (int i=0; i<TABLE_WIDTH; i++)
			{
				int t=-1;
				if (result)
					t=1;
				if (current_history%2)
					x=1;
				else
					x=-1;
				weight_table[index][i]+=x*t;
				if (weight_table[index][i]>upper)
				{
					weight_table[index][i]=upper;
				}
				if (weight_table[index][i]<lower)
				{
					weight_table[index][i]=lower;
				}
				current_history=current_history>>1;
			}
		}
		history=(history<<1)+result;
	}

	void TrackOtherInst (UINT64 PC, OpType opType, bool taken,
	                     UINT64 branchTarget)
	{

	}

};

#endif
