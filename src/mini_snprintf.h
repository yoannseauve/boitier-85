#ifndef __mini_snfrinf_h__
#define __mini_snfrinf_h__

#include <stdarg.h>
//return the number of printable character produced
int mini_snprintf(char buffer[], unsigned int bufferSize, const char* formatString, ...);
int mini_snscanf(char buffer[], unsigned int bufferSize, const char* formatString, ...);

#endif
