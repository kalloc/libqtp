#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include "md5.h"

char *gensession()
{
	static int first=1;
	static unsigned int session_counter=0;
	char session[100];
	char counter[100];
	char temp[100];
	void *rand_ptr;
	static char result[150];
	
	if (first) {
		first=0;
		rand_ptr = malloc(5);
		srandom(getpid()*time(NULL)* ((unsigned long)&rand_ptr) );
		free (rand_ptr);
	}
	
	snprintf (counter,sizeof(counter)-1,"%u",++session_counter);

	snprintf (session,sizeof(session)-1,"%ld",random());
	
	snprintf (temp,sizeof(temp)-1,"%s",MD5SUM(counter));
	
	snprintf (result,sizeof(result)-1,
		"%s%s",
		temp,
		MD5SUM(session)
	);
	
	return result;
}
