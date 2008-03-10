/* parse.h */

/* File autogenerated by gengetopt version 2.18  */

#ifndef PARSE_H
#define PARSE_H

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef PARSE_PACKAGE
#define PARSE_PACKAGE "qtp"
#endif

#ifndef PARSE_VERSION
#define PARSE_VERSION VERSION
#endif

struct gengetopt_args_info
{
  const char *help_help; /* Print help and exit help description.  */
  const char *version_help; /* Print version and exit help description.  */
  char * source_arg;	/* source filename (*.q) for processing.  */
  char * source_orig;	/* source filename (*.q) for processing original value given at command line.  */
  const char *source_help; /* source filename (*.q) for processing help description.  */
  char * out_arg;	/* project name (default source filename before dot).  */
  char * out_orig;	/* project name (default source filename before dot) original value given at command line.  */
  const char *out_help; /* project name (default source filename before dot) help description.  */
  char * out_c_arg;	/* result filename (*.c).  */
  char * out_c_orig;	/* result filename (*.c) original value given at command line.  */
  const char *out_c_help; /* result filename (*.c) help description.  */
  char * out_h_arg;	/* result header filename (*.h).  */
  char * out_h_orig;	/* result header filename (*.h) original value given at command line.  */
  const char *out_h_help; /* result header filename (*.h) help description.  */
  char ** file_arg;	/* internal file for writing cached output.  */
  char ** file_orig;	/* internal file for writing cached output original value given at command line.  */
  int file_min; /* internal file for writing cached output's minimum occurreces */
  int file_max; /* internal file for writing cached output's maximum occurreces */
  const char *file_help; /* internal file for writing cached output help description.  */
  char * prefix_arg;	/* function prefix (like qtp_) (default='').  */
  char * prefix_orig;	/* function prefix (like qtp_) original value given at command line.  */
  const char *prefix_help; /* function prefix (like qtp_) help description.  */
  char ** include_arg;	/* include .h file in template.  */
  char ** include_orig;	/* include .h file in template original value given at command line.  */
  int include_min; /* include .h file in template's minimum occurreces */
  int include_max; /* include .h file in template's maximum occurreces */
  const char *include_help; /* include .h file in template help description.  */
  const char *contentlength_help; /* Insert 'Content-Length:' tag help description.  */
  const char *nostd_help; /* no std output from Q-Lang cgi daemon help description.  */
  const char *nocgix_help; /* No CGIX support enabled help description.  */
  const char *userarg_help; /* Enable passing of user argument help description.  */
  const char *extended_help; /* Use <!--- (three minus) as delimiter help description.  */
  const char *module_help; /* Include QUERY-module in source help description.  */
  
  int help_given ;	/* Whether help was given.  */
  int version_given ;	/* Whether version was given.  */
  int source_given ;	/* Whether source was given.  */
  int out_given ;	/* Whether out was given.  */
  int out_c_given ;	/* Whether out-c was given.  */
  int out_h_given ;	/* Whether out-h was given.  */
  unsigned int file_given ;	/* Whether file was given.  */
  int prefix_given ;	/* Whether prefix was given.  */
  unsigned int include_given ;	/* Whether include was given.  */
  int contentlength_given ;	/* Whether contentlength was given.  */
  int nostd_given ;	/* Whether nostd was given.  */
  int nocgix_given ;	/* Whether nocgix was given.  */
  int userarg_given ;	/* Whether userarg was given.  */
  int extended_given ;	/* Whether extended was given.  */
  int module_given ;	/* Whether module was given.  */

} ;

extern const char *gengetopt_args_info_purpose;
extern const char *gengetopt_args_info_usage;
extern const char *gengetopt_args_info_help[];

int parse (int argc, char * const *argv,
  struct gengetopt_args_info *args_info);
int parse2 (int argc, char * const *argv,
  struct gengetopt_args_info *args_info,
  int override, int initialize, int check_required);
int parse_file_save(const char *filename,
  struct gengetopt_args_info *args_info);

void parse_print_help(void);
void parse_print_version(void);

void parse_init (struct gengetopt_args_info *args_info);
void parse_free (struct gengetopt_args_info *args_info);

int parse_required (struct gengetopt_args_info *args_info,
  const char *prog_name);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* PARSE_H */
