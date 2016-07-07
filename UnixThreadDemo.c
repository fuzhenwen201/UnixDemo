#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define handle_error_en(func_name, en) \
	do{printf("%s failed, error: %s\n", func_name, strerror(en)); exit(EXIT_FAILURE);}while(0)

#define handle_error(func_name) \
	do{printf("%s failed, error: %s\n", func_name, strerror(errno)); exit(EXIT_FAILURE);}while(0)

static void *thread_run(void *arg)
{
	sleep(60);
	return NULL;		
}

int main(int argc, char **argv)
{
	int opt, stack_size, threads_num;
	int iRet;
	pthread_attr_t attr;

	while((opt = getopt(argc, argv, "s:n:")) != -1)
	{
		switch(opt)
		{
		case 's':
			stack_size = strtoul(optarg, NULL, 0);
			break;
		case 'n':
			threads_num = strtoul(optarg, NULL, 10);
			break;
		default:
			fprintf(stderr, "Usage: %s [-s stack_size] [-n threads_num]\n", argv[0]);
			exit(EXIT_FAILURE);			
		}
	}
	printf("stack_size: %d, threads_num: %d\n", stack_size, threads_num);

	if((iRet = pthread_attr_init(&attr)) != 0)
	{
		handle_error_en("pthread_attr_init", iRet);
	}

	if(stack_size > 0)
	{
		if((iRet = pthread_attr_setstacksize(&attr, stack_size)) != 0)
		{
			handle_error_en("pthread_attr_setstacksize", iRet);
		}		
	}


	// create pthread
	int i=0, threads_total_num=0;
	pthread_t *threads_id = (pthread_t *)malloc(threads_num*sizeof(pthread_t));
	if(threads_id == NULL)
	{
		handle_error("malloc");
	}

	while(i < threads_num)
	{
		if((iRet = pthread_create(threads_id + i, &attr, thread_run, NULL)) != 0)
		{			
			//handle_error_en("pthread_create", iRet);
			break;
		}
		i++;		
		threads_total_num++;
	}

	if(threads_total_num == threads_num)
	{
		printf("threads_num: %d all create success!\n", threads_num);		
	}
	else
	{
		printf("pthread_create failed, error: %s, total_num=%d\n", strerror(iRet), threads_total_num);
	}

	if((iRet = pthread_attr_destroy(&attr)) != 0)
	{
		handle_error_en("pthread_attr_destroy", iRet);		
	}

	for(i = 0; i < threads_total_num; i++)
	{
		if((iRet = pthread_join(threads_id[i], NULL)) != 0)
		{
			handle_error_en("pthread_join", iRet);		
		}		
	}

	free(threads_id);
	exit(EXIT_FAILURE);
}
