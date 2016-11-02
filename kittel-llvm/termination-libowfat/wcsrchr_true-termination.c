#include <stddef.h>
#include <stdlib.h>

#define __unlikely(x) (x)
#define __likely(x) (x)
#define wchar_t int

extern int nondet_int();

wchar_t* wcsrchr(const wchar_t *wcs, wchar_t wc) {
  wchar_t* last=0;
  for (; *wcs; ++wcs)
    if (*wcs == wc)
      last=(wchar_t*)wcs;
  return last;
}


int main(){
  int in_len = nondet_int();
  if(in_len < 1){return 1;}
  wchar_t* hay = alloca(in_len*sizeof(wchar_t));
  hay[in_len-1]=0;
  wchar_t needle = nondet_int();
  return (int) wcsrchr(hay, needle);
}
