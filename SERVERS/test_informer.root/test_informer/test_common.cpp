
#include "test_common.h"

void new_ifm_record(ifm_record *record)
{
	if (record == NULL)
	{
		record = new ifm_record;
		if (record == NULL)
		{
			fprintf(stdout, "new_ifm_record: allocate ifm_record failed\n");
			exit(-1);
		}
	}

	for (int i = 0; i < MAX_PARA_COUNT; i++)
	{
		record->paras[i] = new char[MAX_PARA_SIZE];
		if (record->paras[i] == NULL)
		{
			fprintf(stdout, "new_ifm_record: allocate paras failed\n");
			exit(-1);
		}
	}
}

void zero_ifm_record(ifm_record *record)
{
	if (record != NULL)
	{
		for (int i = 0; i < MAX_PARA_COUNT; i++)
		{
			memset(record->paras[i], 0, MAX_PARA_SIZE);
		}
		record->para_count = 0;
		record->sequence_id = 0;
	}
}

void delete_ifm_record(ifm_record *record)
{
	if (record != NULL)
	{
		for (int i = 0; i < MAX_PARA_COUNT; i++)
		{
			delete []record->paras[i];
			record->paras[i] = NULL;
		}
		record->para_count = 0;
		record->sequence_id = 0;
	}
}

void print_ifm_record(ifm_record *record)
{
	if (record != NULL)
	{
		char log_message[102400] = {0};
		sprintf(log_message, "sequence_id=[%llu], para_count=[%u], paras=", record->sequence_id, record->para_count);
		for (uint32_t i = 0; i < record->para_count; i++)
		{
			strcat(log_message, "[");
			strcat(log_message, record->paras[i]);
			strcat(log_message, "]");
		}
		fprintf(stdout, "%s\n", log_message);
	}
}
