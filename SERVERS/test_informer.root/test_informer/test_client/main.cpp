#include <Windows.h>

#include "../test_common.h"

int main(int argc, char *argv[])
{
	if (argc <= 3)
	{
		fprintf(stdout, "Usage: %s service_name message_count paras[0]...paras[n] \n", argv[0]);
		exit(0);
	}

	char service_name[128] = {0};
	strcpy(service_name, argv[1]);

	int message_count = atoi(argv[2]);

	ifm_record record;
	new_ifm_record(&record);


	for (int i = 0; i < message_count; i++)
	{
		for (record.para_count = 0; (record.para_count < argc-3 && record.para_count < MAX_PARA_COUNT); record.para_count++)
		{
			strcpy(record.paras[record.para_count], argv[record.para_count+3]);
		}
		
		record.sequence_id = i + 1;
		if (ifm_post_request(service_name, record.sequence_id, record.paras, record.para_count) != 0)
		{
			fprintf(stdout, "post request failed, sequence_id=%llu \n", (unsigned long long)record.sequence_id);
			continue;
		}

		fprintf(stdout, "request: ");
		print_ifm_record(&record);

		zero_ifm_record(&record);

		for (int retry_times = 0; 
			(ifm_get_response(service_name, &record.sequence_id, record.paras, &record.para_count) != 0 
			&& retry_times < MAX_RETRY_TIMES); 
		retry_times++)
		{
			fprintf(stdout, "no response, retry_times=%d \n", retry_times);
			Sleep(1000);
		}

		fprintf(stdout, "response: ");
		print_ifm_record(&record);
	}

	delete_ifm_record(&record);

	system("pause");

	return 0;
}