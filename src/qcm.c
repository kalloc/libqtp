/*
 *    Quick Cache Manager (for QTP preprocessor) version 1.0.3
 *    Copyright (C) 2004  Denis Slyusarev & ainmarh lab
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <cgix.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <string.h>
#include <time.h>
#include "queue.h"
#include "qstr.h"

#ifdef LINUX_SENDFILE
#include <sys/sendfile.h>
#endif

#define BUF_SIZE 200
#define INT_BUF_SIZE 2048
#define BIG_BUF_SIZE 40000

// Variables for parms management
TAILQ_HEAD (prm_head_t,prm_t) prm_head;
static struct prm_t {
	char *name;
	size_t len;
	TAILQ_ENTRY (prm_t) prm_s;
} *prm;
static int prm_flag=1;
static char buf[BUF_SIZE+1];
static char intbuf[INT_BUF_SIZE+1];
#ifndef LINUX_SENDFILE
static char bigbuf[BIG_BUF_SIZE];
#endif
// --------------------------------

// Variables for DIR management
static size_t dir_len = 0;
static char *dir_path = NULL;
// --------------------------------

// Variables for CHECKER management
#define CHECKER_SIZE 1024
static size_t checker_len = 0;
static char checker_name[CHECKER_SIZE] = "";
// --------------------------------

// Variables for REDIRECTOR/CACHEWRITER management
static size_t redirector_len = 0;
static char *redirector = NULL;
static int contentlength = 0;
// --------------------------------

// Variables for CONTENT-LENGTH support in QTP/QCM.
char *qtp_content=NULL;
char *qtp_content_temp=NULL;
size_t qtp_content_length=0;
size_t qtp_content_i=0;
size_t qcm_cl_internal=65535;
size_t qcm_cl_temp=16384;
// --------------------------------

// ---------------- Internal functions ------------------------ //
void qcm_printf (const char *format, ...)
{
	va_list ap;
	size_t len;
	if (!qtp_content_temp) return;
	va_start (ap,format);
	len = vsnprintf (
		qtp_content_temp,
		qcm_cl_temp,
		format,
		ap
	);
	va_end (ap);
	// Checking for security hole.
	if (!qtp_content) return;
	if (len>(qcm_cl_temp-5)) return;
	if (len==-1) return; // In GLIBC 2.0
	if ((qtp_content_length+len)>(qcm_cl_internal-5)) return;
	// All right! :)
	memcpy ((char *)(qtp_content+qtp_content_length),qtp_content_temp,len);
	qtp_content_length+=len;

}	

void qcm_inject (const char *in,size_t c1,size_t c2,FILE *fl)
{
	if (!qtp_content) return;
	if (((c1*c2)+qtp_content_length) > qcm_cl_internal) return;
	memcpy ((char *)(qtp_content+qtp_content_length),in,c1*c2);qtp_content_length+=(c1*c2);
}

static void qcm_int_touch (const char *name)
{
	FILE *fl;
	struct utimbuf ut;
	time_t tm;
	if (access(name,F_OK)) {
		fl = fopen (checker_name,"w");
		fclose (fl);
	} else {
		tm = time(NULL);
		ut.actime = tm;
		ut.modtime = tm;
		utime (name,&ut);
	}
}

static int qcm_int_checker (time_t time)
{
	struct stat st;
	if (access(checker_name,F_OK)) return 1;
	stat (checker_name,&st);
	if (st.st_mtime<time) return 1;
	return 0;
}

static void qcm_int_action (size_t len)
{
#ifndef LINUX_SENDFILE
	size_t readed;
#endif
	FILE *fl = fopen (intbuf,"r");
	if (!fl) return;
	if (contentlength) printf ("Content-Length: %lld\r\n",len);
	printf ("\r\n");
#ifdef LINUX_SENDFILE
	sendfile (fileno(stdout),fileno(fl),0,len);
#else
	while (!feof(fl))
	{
		readed = fread (bigbuf,1,BIG_BUF_SIZE,fl);
		fwrite (bigbuf,readed,1,stdout);
	}
#endif
	fclose (fl);
}


// ------------------- USER side functions -------------------- //

/** Set QCM/QTP CONTENT-LENGTH buffers size */
void qcm_contentlength_set (size_t internal, size_t temp)
{
	qcm_cl_internal=internal;
	qcm_cl_temp=temp;
}

/** Init the qcm structures */
void qcm_init ()
{
	TAILQ_INIT(&prm_head);
	if (!qtp_content)
		qtp_content = (char *)malloc(qcm_cl_internal);
	if (!qtp_content_temp)
		qtp_content_temp = (char *)malloc(qcm_cl_temp);
	qtp_content[0] = '\0';qtp_content_temp[0] = '\0';
}

/** QCM set content-length */
void qcm_contentlength_on()
{
	contentlength=1;
}

void qcm_contentlength_off()
{
	contentlength=0;
}

/** Set REDIRECTOR prefix for cache redirector */
void qcm_redirector (char *prefix)
{
	if (redirector) free (redirector);redirector=NULL;
	if (!prefix) return;
	redirector_len = strlen(prefix);
	redirector = malloc(redirector_len + 1);
	memcpy (redirector,prefix,redirector_len+1);
}

/** Set dir for removed cache */
void qcm_checker (char *name, ...)
{
	va_list ap;
	va_start (ap,name);
	checker_len = vsnprintf (checker_name,CHECKER_SIZE,name,ap);
	va_end (ap);
}

/** Set QCM cache dir for working */
void qcm_dir (char *path)
{
	if (dir_path) free (dir_path);
	dir_len = strlen(path);
	dir_path = malloc(dir_len+2);
	memcpy (dir_path,path,dir_len+1);
	if (dir_path[dir_len-1]!='/')
	{
		dir_path[dir_len++] = '/';
		dir_path[dir_len] = '\0';
	}
}

/** Open QCM file */
FILE *qcm_open (char *name, ...)
{
	char ebuf[BUF_SIZE];
	struct stat st;
	va_list ap;
	size_t intbuf_len=0;

	va_start (ap,name);
	vsnprintf (ebuf,BUF_SIZE,name,ap);
	va_end (ap);
	
	if ((!dir_path)||(!checker_name)) return NULL;
	intbuf[0] = '\0';
	strfcat (intbuf,dir_path,INT_BUF_SIZE,&intbuf_len);
	if (intbuf[intbuf_len-1]!='/') strfcat (intbuf,"/",INT_BUF_SIZE,&intbuf_len);
	strfcat (intbuf,ebuf,INT_BUF_SIZE,&intbuf_len);
	strfcat (intbuf,"-",INT_BUF_SIZE,&intbuf_len);
	for (prm = prm_head.tqh_first; prm != NULL; prm = prm->prm_s.tqe_next)
	{
		if (cgiFormString (prm->name,buf,BUF_SIZE)==cgiFormSuccess)
			strfcat (intbuf,buf,INT_BUF_SIZE,&intbuf_len);
		strfcat (intbuf,"_",INT_BUF_SIZE,&intbuf_len);
	}
	strfcat (intbuf,".cache",INT_BUF_SIZE,&intbuf_len);
	if (access(intbuf,F_OK)) return (fopen (intbuf,"w"));
	stat (intbuf,&st);
	if (qcm_int_checker(st.st_mtime)) { qcm_int_action(st.st_size);return NULL; }
	return (fopen (intbuf,"w"));
}

/** Close QCM file */
void qcm_close (FILE **fl)
{
	fclose (*fl);
	(*fl) = NULL;
}

/** Clean current cache dir */
void qcm_clear ()
{
	qcm_int_touch(checker_name);
}

/** Add parameter to QCP processor list
 *  if parameter==NULL clear all parameters */
void qcm_set (char *parameter)
{
	size_t len;
	if (!parameter)
	{
		while (prm_head.tqh_first != NULL)
		{
			free (prm_head.tqh_first->name);
			prm = prm_head.tqh_first;
			TAILQ_REMOVE (&prm_head, prm_head.tqh_first, prm_s);
			free (prm);
		}
		return;
	}
	if (prm_flag) { prm_flag=0; TAILQ_INIT (&prm_head); }
	len = strlen(parameter);
	prm = malloc (sizeof(struct prm_t));
	prm->name = malloc(len+1);
	prm->len = len;
	memcpy (prm->name,parameter,len+1);
	TAILQ_INSERT_TAIL (&prm_head, prm, prm_s);
}
