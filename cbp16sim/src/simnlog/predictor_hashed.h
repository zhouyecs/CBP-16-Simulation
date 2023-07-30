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
#include "utils.h"
#include "bt9.h"
#include "bt9_reader.h"

#define HASH_FUNC 1 //1, {mi,P,G}; 2, {mi,PcG,NULL}; 3, {mi,PxP,NULL}; 4, {mi,PxG,NULL}; 5, {mi,AxG,NULL};

#define TABLE_COUNT 4
#define TABLE_SIZE 32
#define WEIGHT_SIZE 3
#define NAME " (HASHED PERCEPTRON) "
int THRESHOLD=(int)pow(2,WEIGHT_SIZE-1);

class PREDICTOR
{
private:
	uint64_t history=0;
	std::vector<std::vector<int>>weight_table;
	std::vector<uint64_t> past_branch_pc;
	int last_prediction=0;
	int upper=(int)pow(2,WEIGHT_SIZE-1)-1;
	int lower=-(int)pow(2,WEIGHT_SIZE-1)+1;

	uint64_t getIndex(uint64_t table_id)
	{
#if HASH_FUNC==1
		return past_branch_pc[table_id]%TABLE_SIZE;
#elif HASH_FUNC==2
		return ((past_branch_pc[2*table_id]<<1) + (history>>(2*table_id+1))%2)%TABLE_SIZE;
#elif HASH_FUNC==3
		return (past_branch_pc[2*table_id] ^ (past_branch_pc[2*table_id+1]<<1))%TABLE_SIZE;
#elif HASH_FUNC==4
		uint64_t length=(uint64_t)log2(TABLE_SIZE);
		uint64_t history_seg=history>>(length*table_id);
		return (past_branch_pc[table_id] ^ history_seg)%TABLE_SIZE;
#elif HASH_FUNC==5
		uint64_t length=(uint64_t)log2(TABLE_SIZE);
		uint64_t history_seg=history>>(length*table_id);
		return (past_branch_pc[0] ^ history_seg)%TABLE_SIZE;
#else
		return 0;
#endif
	}

	int64_t getWeight(uint64_t table_id)
	{
#if HASH_FUNC==0 || HASH_FUNC==1
		uint64_t index=getIndex(table_id);
		uint64_t current_history=(history<<1)+1;
		uint64_t history_bit=(current_history>>table_id)%2;
		int x=-1;
		if (history_bit)
		{
			x=1;
		}
		return weight_table[table_id][index]*x;
#else
		uint64_t index=getIndex(table_id);
		return weight_table[table_id][index];
#endif
	}
public:
    PREDICTOR (void)
	{
	    fprintf (stderr, NAME);
	    fprintf (stderr, " (TABLE_SIZE %d) ", TABLE_SIZE);
	    fprintf (stderr, " (TABLE_COUNT %d) ", TABLE_COUNT);
	    fprintf (stderr, "\n");
		init ();
	}

	void init()
	{
		for (int i=0; i<TABLE_COUNT; i++)
		{
			std::vector<int> table(TABLE_SIZE,0);
			weight_table.push_back(table);
			past_branch_pc.insert(past_branch_pc.begin(),2,0);
		}
	}
	//PERCEPTRON():weight_table(TABLE_SIZE,std::vector<int>(TABLE_WIDTH,0)){};
	bool GetPrediction (UINT64 address)
	{
		int prediction=0;
		past_branch_pc.insert(past_branch_pc.begin(),address);
		past_branch_pc.pop_back();
		for (int i=0; i<TABLE_COUNT; i++)
		{
			int64_t w=getWeight(i);
			prediction+=w;
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
		uint64_t current_history=(history<<1)+1;
		int x;
		if ((last_prediction>=0)!=result || abs(last_prediction)<=THRESHOLD)
		{
			for (int i=0; i<TABLE_COUNT; i++)
			{
				uint64_t index=getIndex(i);
#if HASH_FUNC==0 || HASH_FUNC==1
				int t=-1;
				if (result)
					t=1;
				if (current_history%2)
					x=1;
				else
					x=-1;
				weight_table[i][index]+=x*t;
#else
				if (result)
					weight_table[i][index]++;
				else
					weight_table[i][index]--;
#endif
				if (weight_table[i][index]>upper)
				{
					weight_table[i][index]=upper;
				}
				if (weight_table[i][index]<lower)
				{
					weight_table[i][index]=lower;
				}
				current_history=current_history>>1;
			}
		}
		history=(history<<1)+result;
	}

};

#endif

