
#ifndef _TEST_COMMON_H_
#define _TEST_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "informerLib\informer_lib.h"

#define MAX_RECORD_COUNT 100
#define MAX_PARA_COUNT 20
#define MAX_PARA_SIZE 256

#define MAX_RETRY_TIMES 3

struct ifm_record
{
	uint64_t sequence_id;
	uint32_t para_count;
	char *paras[MAX_PARA_COUNT];
};

void new_ifm_record(ifm_record *record);
void zero_ifm_record(ifm_record *record);
void delete_ifm_record(ifm_record *record);
void print_ifm_record(ifm_record *record);

#endif //_TEST_COMMON_H_
