// Quickstring Toolkit [header]
// Copyright (C) 2003   Denis Slyusarev
#include <sys/types.h>
#define YY_NO_UNPUT

char *str1cpy (char *dest, const char *src, size_t n);
size_t str1cat(char *dst,
	       const char *src,
	       size_t size);
size_t strfcat(char *dst,
	       const char *src,
	       size_t size,
	       size_t *current);
