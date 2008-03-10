#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include "../config.h"

typedef struct entry_s {
	char name[1024];
	struct entry_s *next;
} entry_t;

entry_t *first=NULL;
entry_t *last=NULL;

int main(int argc, char **argv)
{
	DIR *dir;
	struct dirent *entry;
	char buf[8192];
	char fname[1024];
	char patch[1024]="";
	char *ptr;
	int flag=0;
	int i;

	for (i=1;i<argc;i++) {
		strcat(patch,argv[i]);
		strcat(patch," ");
	}
	
	dir = opendir("./templates");
	if (!dir) {
		printf ("ERROR: No 'template' directory found!\n");
		return -1;
	}
	
	printf ("QTP make project ver %s\n\n",VERSION);
	
	while ( (entry = readdir(dir)) ) if (strstr(entry->d_name,".qtp")) {
		printf ("Processing %s into [.c] and [.h]...\n",entry->d_name);
		strcpy(fname,entry->d_name);
		ptr = strstr(fname,".qtp");if (!ptr) return -1;
		ptr[0] = '\0';
		
		snprintf (buf,sizeof(buf)-1,"qtp %s -s templates/%s -o qtp_%s",
				patch,
				entry->d_name,
				fname
			 );
		printf ("%s\n",buf);
		system (buf);
		flag = 1;
	}
	closedir(dir);

	if(flag) {
		printf ("Compiling sources...\n");
		printf ("gcc -Wall -c qtp_*.c\n");
		system ("gcc -Wall -c qtp_*.c");
		printf ("Ar object files in templates.a...\n");
		system ("ar q templates.a qtp_*.o");
		printf ("Generating templates.h...\n");
		system ("cat qtp_*.h > templates.h\n");
		system ("rm qtp_*.o qtp_*.c qtp_*.h");
	}
	return 0;
}
