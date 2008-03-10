%{
#define T_PREPARE 5
#define T_TRUE 1
#define T_FALSE 0

#define MAX_PARAMS 300
#define MAX_PARAMS_LEN 200
#define MAX_RES 500000
#define MAX_INCLUDE_DEPTH 50
#define RELEASE "stable release"
#define FAULT_OFFSET 15
#define FAULT_OFFSET_NULL FAULT_OFFSET-1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "qstr.h"
#include "parse.h"
#include "utils.h"
#include "../config.h"
char parms[MAX_PARAMS][MAX_PARAMS_LEN];
int parms_count;

char sources[MAX_PARAMS][MAX_PARAMS_LEN];
int sources_count=0;

char funct[MAX_PARAMS_LEN];
int uniproc=0,uinternal=0;
char inclvar[MAX_PARAMS_LEN];
char res[MAX_RES],dupres[MAX_RES],tempres[MAX_RES];
char do_cmd[MAX_PARAMS][MAX_PARAMS_LEN];
int do_cmd_internal[MAX_PARAMS];
char ibuf[MAX_RES];
int qtp_do_cmd_count=0;
int elif=0;
int qtp_toolline=1;
size_t res_count;
int is_while,is_i, is_at_start=T_PREPARE;
int qtp_fwrite_counter=0;

FILE *fout_c,*fout_h;
struct gengetopt_args_info pinfo;
char option[201];

// Includes...
YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH+1];
int include_stack_ptr = 0;

%}

PARAMETER	[a-z][a-z0-9_]*
%x htmlcomment declare params replacer ifelse include endtag endinclude sorce

%%


"<!--"		{
	if (!(pinfo.extended_given)) BEGIN (htmlcomment); else
	{
		qtp_fwrite_counter+=4;strfcat (res,"<!--",MAX_RES,&res_count);
		if (is_at_start!=T_TRUE) is_at_start = T_FALSE;
	}
}

"<!---"		BEGIN (htmlcomment);

<htmlcomment>"proc "	{
	parms_count=0;is_while=0;is_i=0;uniproc=0;uinternal=0;
	do_cmd_internal[qtp_do_cmd_count] = 0;
	BEGIN (declare);
}

<htmlcomment>"loop "	{
	parms_count=0;is_while=0;is_i=0;uniproc=0;uinternal=0;
	do_cmd_internal[qtp_do_cmd_count] = 0;
	BEGIN (declare);
}

<htmlcomment>"exec "	{
	parms_count=0;is_while=0;is_i=0;uniproc=0;uinternal=0;
	do_cmd_internal[qtp_do_cmd_count] = 0;
	BEGIN (declare);
}

<htmlcomment>"extern "	{
	parms_count=0;is_while=0;is_i=0;uniproc=1;uinternal=0;
	do_cmd_internal[qtp_do_cmd_count] = 0;
	BEGIN (declare);
}

<htmlcomment>"intern "	{
	parms_count=0;is_while=0;is_i=0;uniproc=0;uinternal=1;
	do_cmd_internal[qtp_do_cmd_count] = 1;
	BEGIN (declare);
}

<htmlcomment>"lib "	{
	parms_count=0;is_while=0;is_i=0;uniproc=0;uinternal=2;
	do_cmd_internal[qtp_do_cmd_count] = 2;
	BEGIN (declare);
}

<htmlcomment>"source "	{
	BEGIN (sorce);
}

<sorce>[^,; ]+		{
	str1cpy (sources[sources_count++],qtptext,MAX_PARAMS_LEN);
}

<sorce>","	;

<sorce>" "	;

<sorce>";"	{
	int i;
	if (sources_count) {
		for (i=0;i<sources_count;i++)
			fprintf (fout_c,"#include \"%s\"\n",sources[i]);
		fprintf (fout_c,"\n");
		sources_count=0;
	}
	is_at_start = T_PREPARE;
	BEGIN (htmlcomment);
}

<declare>{PARAMETER}	{
	str1cpy (funct,qtptext,MAX_PARAMS_LEN);
	if (!uniproc)
	    snprintf (do_cmd[qtp_do_cmd_count++],MAX_PARAMS_LEN,"%s%s_%s",
			    pinfo.prefix_arg,
			    pinfo.out_arg,
			    qtptext);
	else
	    str1cpy (do_cmd[qtp_do_cmd_count++],qtptext,MAX_PARAMS_LEN);
	BEGIN (params);
}

<params>{PARAMETER}[^,;]+ str1cpy (parms[parms_count++],qtptext,MAX_PARAMS_LEN);

<params>";"		{
	int i;
	if (qtp_fwrite_counter==0) res[res_count-FAULT_OFFSET_NULL] = '\0';
	str1cpy (dupres,res,MAX_RES);
	if (uniproc) {
		fprintf (fout_c,"void %s_do (%s);\n",funct,option);
	} else {
		if (!uinternal) 
			fprintf (fout_c,"void %s%s_%s_do (%s);\n",
					pinfo.prefix_arg,
					pinfo.out_arg,
					funct,option);
		fprintf (fout_c,"void %s%s_%s (",
				pinfo.prefix_arg,
				pinfo.out_arg,
				funct);
		fprintf (fout_h,"void %s%s_%s (",
				pinfo.prefix_arg,
				pinfo.out_arg,
				funct);
	}
	if (parms_count>0) {
		fprintf (fout_c,"%s",parms[0]);
		fprintf (fout_h,"%s",parms[0]); 
	}
	for (is_i=1;is_i<parms_count;is_i++) {
		fprintf (fout_c,", %s",parms[is_i]);
		fprintf (fout_h,", %s",parms[is_i]);
	}
	if (!uniproc) { 
	    fprintf (fout_c,") {\n");
	    if (!pinfo.nostd_given) {
		if (!pinfo.contentlength_given) {
		    replace (dupres,"[[[FWRITE]]]","fwrite",1);
		    replace (dupres,"[[[OFILE]]]","stdout",1);
		    if (is_at_start==T_FALSE) fprintf (fout_c,"\tfwrite (\"");
		    fprintf (fout_c,"%s",dupres);
		    if (qtp_fwrite_counter>0) 
			    fprintf (fout_c,"\",%d,1,stdout);\n",
					    qtp_fwrite_counter);
		} else {
		    replace (dupres,"[[[FWRITE]]]","qcm_inject",1);
		    replace (dupres,"[[[OFILE]]]","stdout",1);
		    if (is_at_start==T_FALSE)
			    fprintf (fout_c,"\tqcm_inject (\"");
		    fprintf (fout_c,"%s",dupres);
		    if (qtp_fwrite_counter>0)
			    fprintf (fout_c,"\",%d,1,stdout);\n",
					    qtp_fwrite_counter);
		}
	    }
	    if (!pinfo.contentlength_given)
		for (i=0;i<pinfo.file_given;i++) {
		    fprintf (fout_c,"\tif (!%s) return;\n",pinfo.file_arg[i]);
		    str1cpy (dupres,res,MAX_RES);
		    replace (dupres,"[[[FWRITE]]]","fwrite",1);
		    replace (dupres,"[[[OFILE]]]",pinfo.file_arg[i],1);
		    if (is_at_start==T_FALSE)
			    fprintf (fout_c,"\tfwrite (\"");
		    fprintf (fout_c,"%s",dupres);
		    if (qtp_fwrite_counter>0)
			    fprintf (fout_c,"\",%d,1,%s);\n",
					    qtp_fwrite_counter,
					    pinfo.file_arg[i]);
		}	
	    fprintf (fout_c,"}\n");
	    fprintf (fout_h,");\n");
	}
	res[0] = '\0'; res_count = 0;
	qtp_fwrite_counter=0;
	is_at_start = T_PREPARE;
	BEGIN(htmlcomment);
}

<params>" "		;
<params>","		;

<htmlcomment>[\n]+ ;
<htmlcomment>. ;
<declare>. ;

<params>"-->"[\r\n]*	BEGIN(INITIAL);

<params>.		{
	fprintf (stderr,"Error: Unexpected '%s' at line %d !\n",
			qtptext,
			qtp_toolline);
	exit (0);
}

<htmlcomment>"-->"[\r\n]*	BEGIN(INITIAL);

"{var "		BEGIN (replacer);

<replacer>[^}]+ {
	    // Only fast mode
	    if(qtp_fwrite_counter) {
			strfcat(res,"\"",MAX_RES,&res_count);
			snprintf (tempres,MAX_RES,",%d,1,[[[OFILE]]]);\n",
					qtp_fwrite_counter);
			strfcat(res,tempres,MAX_RES,&res_count);
	    } else {
		    if (is_at_start==T_PREPARE) is_at_start = T_TRUE;
		    else res_count-=FAULT_OFFSET;
	    }
	    if (!pinfo.contentlength_given) {
		strfcat(res,"\tfprintf([[[OFILE]]],",MAX_RES,&res_count);
		strfcat(res,qtptext,MAX_RES,&res_count);
		strfcat(res,");\n\t[[[FWRITE]]] (\"",MAX_RES,&res_count);
	    } else {
		snprintf (ibuf,MAX_RES,"\tqcm_printf (");
		strfcat (res,ibuf,MAX_RES,&res_count);
		strfcat(res,qtptext,MAX_RES,&res_count);
		snprintf (ibuf,MAX_RES,");\n\t[[[FWRITE]]] (\"");
		strfcat(res,ibuf,MAX_RES,&res_count);
	    }


	    qtp_fwrite_counter=0;
		BEGIN (endtag);
}

"{if "		{
		elif=0;
		BEGIN (ifelse);
}

"{elif "	{
		elif=1;
		BEGIN (ifelse);
}

<ifelse>[^}]+	{
		// Only fast mode
		if(qtp_fwrite_counter) {
			strfcat(res,"\"",MAX_RES,&res_count);
			snprintf (tempres,MAX_RES,",%d,1,[[[OFILE]]]);\n\t",
					qtp_fwrite_counter);
			strfcat(res,tempres,MAX_RES,&res_count);
		} else {
			if (is_at_start==T_PREPARE) is_at_start = T_TRUE; 
			else {
				res_count-=FAULT_OFFSET;
				if (res_count<0) res_count=0;
			}
		}
		if (elif)
			strfcat(res,"} else if(",MAX_RES,&res_count);
		else
			strfcat(res,"if(",MAX_RES,&res_count);
		strfcat(res,qtptext,MAX_RES,&res_count);
		strfcat(res,") {\n\t",MAX_RES,&res_count);

		// It's fast mode.
		strfcat(res,"[[[FWRITE]]] (\"",MAX_RES,&res_count);

		qtp_fwrite_counter=0;
		BEGIN (endtag);
}

"{else"		{
		// It's fast mode.
		if(qtp_fwrite_counter) {
			strfcat(res,"\"",MAX_RES,&res_count);
			snprintf (tempres,MAX_RES,",%d,1,[[[OFILE]]]);\n\t",
					qtp_fwrite_counter);
			strfcat(res,tempres,MAX_RES,&res_count);
		} else
		if (is_at_start==T_PREPARE) is_at_start = T_TRUE; 
		else res_count-=FAULT_OFFSET;

		strfcat(res,"} else {\n\t",MAX_RES,&res_count);

		// It's fast mode.
		strfcat(res,"[[[FWRITE]]] (\"",MAX_RES,&res_count);

		qtp_fwrite_counter=0;
		BEGIN (endtag);
}

"{/if"		{
		// Only fast mode.
		if(qtp_fwrite_counter) {
			strfcat(res,"\"",MAX_RES,&res_count);
			snprintf (tempres,MAX_RES,",%d,1,[[[OFILE]]]);\n\t",
					qtp_fwrite_counter);
			strfcat(res,tempres,MAX_RES,&res_count);
		} else
		if (is_at_start==T_PREPARE) is_at_start = T_TRUE;
		else res_count-=FAULT_OFFSET;

		strfcat(res,"}\n\t",MAX_RES,&res_count);

		// It's fast mode.
		strfcat(res,"[[[FWRITE]]] (\"",MAX_RES,&res_count);

		qtp_fwrite_counter=0;
		BEGIN (endtag);
}

"{include "	BEGIN(include);

<<EOF>> {
	if (--include_stack_ptr<0) yyterminate(); else
	{
	    qtp_delete_buffer(YY_CURRENT_BUFFER);
	    qtp_switch_to_buffer (include_stack[include_stack_ptr]);
	}
}
	

<include>[^}]+ {
		str1cpy (inclvar,qtptext,MAX_PARAMS_LEN);

		BEGIN (endinclude);
}

<endinclude>.[\r\n]* {

		if (include_stack_ptr == MAX_INCLUDE_DEPTH) {
		    fprintf (stderr,"Include max depth reached!\n");
		    exit (1);
		}
		include_stack[include_stack_ptr++] = YY_CURRENT_BUFFER;
		
		qtpin = fopen (inclvar,"r");
		if (!qtpin) {
		    fprintf (stderr,"Cant open [%s] file!!!\n",inclvar);
		    exit (1);
		}
		
		qtp_switch_to_buffer(qtp_create_buffer (qtpin, YY_BUF_SIZE));
		BEGIN (INITIAL);
}

<endtag>.[\r\n]* {
		BEGIN (INITIAL);
}

"\r"		;
"\n"		{
	qtp_toolline++;qtp_fwrite_counter+=2;
	strfcat (res,"\\r\\n",MAX_RES,&res_count);
}
"\""	{
	qtp_fwrite_counter++;strfcat (res,"\\\"",MAX_RES,&res_count);
	if (is_at_start!=T_TRUE) is_at_start = T_FALSE;
}
"{{"	{
	qtp_fwrite_counter++;strfcat (res,"{",MAX_RES,&res_count);
	if (is_at_start!=T_TRUE) is_at_start = T_FALSE;
}
"}}"	{
	qtp_fwrite_counter++;strfcat (res,"}",MAX_RES,&res_count);
	if (is_at_start!=T_TRUE) is_at_start = T_FALSE;
}
"\\"	{
	qtp_fwrite_counter++;strfcat (res,"\\\\",MAX_RES,&res_count);
	if (is_at_start!=T_TRUE) is_at_start = T_FALSE;
}
.	{
	qtp_fwrite_counter++;strfcat (res,qtptext,MAX_RES,&res_count);
	if (is_at_start!=T_TRUE) is_at_start = T_FALSE;
}

%%

int qtpwrap() { return 1; }

void qtp_show_about() {
	printf ("QTP (Qlang Template Preprocessor) version %s (%s)\n"
		"[ This tool is part of QPLATFORM project. ]\n\n"
		"Copyrght (c) 2003,2004,2006,2007  ainmarh labs\n\n",
			VERSION,
			OS_NAME);
}

int main (int argc, char **argv)
{

	char fname[201],*ptr;int i;
	res_count = 0;res[0] = '\0';

	parse (argc,argv,&pinfo);
	
	if (pinfo.userarg_given) snprintf (option,200,"void *userarg");
	else option[0]='\0';

	qtpin = fopen (pinfo.source_arg,"r");

	if (!qtpin) {
		qtp_show_about();
		printf ("Error: Can't open input file [%s]...\n\n",
				pinfo.source_arg);
		return 0;
	}
	
	if (!pinfo.out_given) {
		pinfo.out_arg = malloc (strlen(pinfo.source_arg)+2);
		str1cpy (pinfo.out_arg,
				pinfo.source_arg,
				strlen(pinfo.source_arg)+2);
	}
	
	if ( (ptr = strchr (pinfo.out_arg,'.')) ) ptr[0] = '\0';
	
	if (pinfo.out_c_given) snprintf (fname,200,"%s",pinfo.out_c_arg);
	else snprintf (fname,200,"%s.c",pinfo.out_arg);
	fout_c = fopen (fname,"w");
	if (!fout_c) {
		qtp_show_about();
		printf ("Error: Can't open output file [%s]...\n\n",fname);
		return 0;
	}

	if (pinfo.out_h_given) snprintf (fname,200,"%s",pinfo.out_h_arg);
	else snprintf (fname,200,"%s.h",pinfo.out_arg);
	fout_h = fopen (fname,"w");
	if (!fout_h) {
		qtp_show_about();
		printf ("Error: Can't open output file [%s]...\n\n",fname);
		return 0;
	}
	fprintf (stderr,"Processing %s...\n",pinfo.source_arg);
	fprintf (fout_c,"/* Generated from "
			"[%s] by QTP (version %s %s) */\n\n",
			pinfo.source_arg,
			VERSION,
			OS_NAME);
	fprintf (fout_h,"/* Generated from "
			"[%s] by QTP (version %s %s) */\n\n",
			pinfo.source_arg,
			VERSION,
			OS_NAME);
	if (!pinfo.nocgix_given) 
		fprintf(fout_c,"#include <string.h>\n"
				"#include <stdio.h>\n"
				"#include <cgix.h>\n"
				"#include <qcm_internal.h>\n");
	else
		fprintf(fout_c,"#include <string.h>\n"
				"#include <stdio.h>\n");
	for (i=0;i<pinfo.include_given;i++)
		fprintf (fout_c,"#include \"%s\"\n",pinfo.include_arg[i]);
	if (pinfo.contentlength_given) {
		fprintf (fout_c,"extern char *qtp_content;\n");
		fprintf (fout_c,"extern char *qtp_content_temp;\n");
		fprintf (fout_c,"extern size_t qtp_content_length;\n");
		fprintf (fout_c,"extern size_t qtp_content_i;\n");
		fprintf (fout_c,"extern size_t qcm_cl_internal;\n");
		fprintf (fout_c,"extern size_t qcm_cl_temp;\n");
	}
	for (i=0;i<pinfo.file_given;i++)
		fprintf (fout_c,"extern FILE *%s;\n",pinfo.file_arg[i]);
	fprintf (fout_c,"\n");
	if ((!pinfo.nostd_given)&&(pinfo.contentlength_given)) {
		fprintf (fout_c,"void %s%s_commit () "
				"{\n\tprintf (\"Content-Length: %%lld\\r\\n\""
				",(long long)qtp_content_length);\n",
				pinfo.prefix_arg,
				pinfo.out_arg);
		fprintf (fout_c,"\tfwrite (qtp_content,1,qtp_content_length"
				",stdout);\n");
		for (i=0;i<pinfo.file_given;i++) 
			fprintf (fout_c,"\tfwrite (qtp_content,1,"
					"qtp_content_length,%s);\n",
					pinfo.file_arg[i]);
		fprintf (fout_c,"}\n\n");
	}
	qtplex();
	for (i=0;i<qtp_do_cmd_count;i++)
		if (do_cmd_internal[i]==1) 
			fprintf (fout_c,"void %s_do(%s) {\n\t%s();\n}\n\n",
					do_cmd[i],
					option,
					do_cmd[i]);
	fprintf (fout_c,"\nvoid %s%s(%s) {\n",
			pinfo.prefix_arg,
			pinfo.out_arg,
			option);
	fprintf (fout_h,"\nvoid %s%s(%s);\n",
			pinfo.prefix_arg,
			pinfo.out_arg
			,option);
	if (!(pinfo.nostd_given)) fprintf (fout_c,"\tprintf (\"\\r\\n\");\n");
	for (i=0;i<qtp_do_cmd_count;i++)
		if (do_cmd_internal[i]!=2) fprintf (fout_c,"\t%s_do(%s);\n",
				do_cmd[i],
				option[0]?"userarg":"");
	if (pinfo.contentlength_given) {
		fprintf (fout_c,"\t%s%s_commit();\n",
				pinfo.prefix_arg,
				pinfo.out_arg);
		fprintf (fout_c,"\tqtp_content_length=0;\n");
	}
	fprintf (fout_c,"}\n");
	fclose (fout_c);
	fclose (fout_h);
	return 0;
}
