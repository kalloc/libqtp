#include <stdio.h>
#include <stdlib.h>
#include <cgix.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <utime.h>
#include <time.h>
#include "idb.h"
#include "gensession.h"
#include "garbage.h"

static char *qsm_name=NULL;
static char *qsm_path=NULL;
static char qsm_session[150];
static int qsm_seconds;
static char filename[250];

// Internal Functions

static IDB idb=NULL;

void qsm_int_close_idb()
{
	if (idb) {
		idb_close(idb);
		idb=NULL;
	}
}

static void qsm_int_touch (const char *name)
{
	if (!access(name,F_OK)) utime (name,NULL);
	
}

void qsm_int_filter_do()
{
	garbage (qsm_path,qsm_name,time(NULL),qsm_seconds);
	exit(0);
}

void qsm_int_filter()
{
	while (1) {
		if (!fork())
			qsm_int_filter_do();
		wait(NULL);
		sleep(qsm_seconds);
	}
}		

void qsm_int_spawn_filter()
{
	static FILE *check=NULL;
	char filename[200];

	snprintf (filename,sizeof(filename)-1,
		"%s/%s.lock",
		qsm_path,
		qsm_name
	);

	check = fopen(filename,"w");

	if ( flock(fileno(check),LOCK_EX|LOCK_NB)==0 )
		if (!fork()) {
			fclose(stdin);
			fclose(stdout);
			fclose(stderr);
			qsm_int_filter();
		}
	fclose(check);
}

void qsm_int_set_filename()
{
	snprintf (filename,sizeof(filename)-1,
		"%s/%s.%s.ses",
		qsm_path,
		qsm_name,
		qsm_session
	);
}

// ----------------------

void qsm_init(char *path, char *name, int seconds)
{
	size_t name_len,path_len;
	if ((!name)||(!path)) return;
	if (!seconds) return;

	qsm_seconds = seconds;
	
	name_len=strlen(name)+1;
	path_len=strlen(path)+1;
	
	if (qsm_name) free (qsm_name);
	if (qsm_path) free (qsm_path);
	
	qsm_name = (char *)malloc(name_len);
	qsm_path = (char *)malloc(path_len);
	
	memcpy (qsm_name,name,name_len);
	memcpy (qsm_path,path,path_len);
	
	qsm_int_spawn_filter();
}

int qsm_init_session()
{
	static time_t cur_time=0;
	time_t get_time;

	qsm_int_close_idb();

	get_time = time(NULL);
	
	if (!cur_time) {
		cur_time = get_time;
//		qsm_int_spawn_filter();
	} else
		if ((cur_time+qsm_seconds)<get_time) {
			cur_time = get_time;
			/* qsm_int_spawn_filter(); !!! Not functionality !!! */
		}
	
	if (cgiCookieString(qsm_name,
		qsm_session,
		sizeof(qsm_session)-1)==cgiFormSuccess) {
			qsm_int_set_filename();
			return 1;
		}

	snprintf (qsm_session,sizeof(qsm_session)-1,"%s",gensession());

	printf ("Set-Cookie: %s=%s\r\n",qsm_name,qsm_session);
	qsm_int_set_filename();
	return 0;
}

void qsm_set_param(char *key, char *value)
{
	qsm_int_close_idb();

	if ((!qsm_name)||(!qsm_path)) return;
	
	idb = idb_open(filename);
	if (!idb) return;
	
	idb_put(idb,key,value);
	
	qsm_int_close_idb();

	qsm_int_touch(filename);
}

char *qsm_get_param(char *key)
{
	char *temp;

	if (!idb) {
		idb = idb_open_reader(filename);
		if (!idb) return NULL;
	}
	
	temp = idb_get(idb,key);
	
	qsm_int_touch(filename);
	
	return (temp);
}

int qsm_del_param(char *key)
{
	int temp;
	
	qsm_int_close_idb();
	
	idb = idb_open(filename);
	if (!idb) return (-1);
	
	temp = idb_del(idb,key);
	
	qsm_int_close_idb();
	
	qsm_int_touch(filename);
	
	return (temp);
}
