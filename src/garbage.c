#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

#define SUFFIX ".ses"

void garbage(char *path, char *prefix,time_t current, time_t seconds)
{
	DIR *dir;
	struct dirent *dirp;
	struct stat buf;
	char spath[200];

	dir = opendir(path);

	while ((dirp = readdir(dir))) if (strstr(dirp->d_name,SUFFIX))
		if (strstr(dirp->d_name,prefix)==dirp->d_name) {
			snprintf(spath,sizeof(spath)-1,
				"%s/%s",
				path,
				dirp->d_name
			);
			
			stat (spath,&buf);
			
			if (seconds+buf.st_mtime<current) unlink (spath);
	}

	closedir(dir);
}
