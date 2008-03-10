// Quickstring Toolkit $Id: qstr.c,v 1.1.1.1 2003/12/29 13:13:09 melon Exp $
// Copyright (C) 2003   Denis Slyusarev

#include <sys/types.h>
#include <string.h>

char *str1cpy (char *dest, const char *src, size_t n)
{
	int len,flag=0;
	if (!src) return NULL;
	len = strlen (src);
	if (len>=n) { len=n;flag=1; }
	memcpy ((void *)dest,(void *)src,len);
	if (!flag) dest[len] = '\0'; else dest[len-1] = '\0';
	return dest;
}

size_t str1cat(char *dst,
	       const char *src,
	       size_t size) {
  size_t dl = size ? strlen(dst) : 0;
  size_t sl = strlen(src);
  size_t tl = sl + dl;

  if(tl >= size) {
    if(size) {
      memcpy(dst + dl, src, size - dl - 1);
      dst[size - 1] = 0;
    }
  } else
    memcpy(dst + dl, src, sl + 1);
  return tl;
}

size_t strfcat(char *dst,
	       const char *src,
	       size_t size,
	       size_t *current) {
  size_t dl = size ? *current : 0;
  size_t sl = strlen(src);
  size_t tl = sl + dl;

  if(tl >= size) {
    if(size) {
      memcpy(dst + dl, src, size - dl - 1);
      dst[size - 1] = 0;
      *current = size-1;
    }
  } else {
    memcpy(dst + dl, src, sl + 1);
    *current = tl;
  }
  return tl;
}
