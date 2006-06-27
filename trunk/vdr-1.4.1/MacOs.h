#ifndef __MACOS_H__
#define __MACOS_H__

#define __attribute_format_arg__(x...)

#include <stdlib.h>

#include <strings.h>
inline char * strndup(const char * str, int maxlen) {
  int len=strlen(str);
  if ( maxlen > len )
        return strdup(str);
  char *new_str= (char *) malloc(len);
  strncpy(new_str,str,len);
  new_str[len-1]=0;
  return new_str;
};

inline char * strchrnul(const char *s, int c)
{ 
  while (*s && (*s!=c) )
        s++;

  return (char *) s;
}

#endif
