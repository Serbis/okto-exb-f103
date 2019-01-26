#include "../include/data.h"
#include <limits.h>

char* itoa2(int num) {
    char *buf = pmalloc(50);
    sprintf(buf, "%d", num);
    uint8_t len = (uint8_t) (strlen(buf) + 1);
    char *split = pmalloc(len);
    memcpy(split, buf, len);
    pfree(buf);

    return split;
}

char* strcpy2(char* str) {
    size_t strsize = strlen(str) + 1;
    char* dup_str = pmalloc(strsize);
    memcpy(dup_str, str, strsize);

    return dup_str;
}

long strtol2(const char *nptr, char **endptr, int base, long min, long max, uint8_t *result) {
	 const char *s;
     long acc, cutoff;
     int c;
     int neg, any, cutlim;

     /*
      * Skip white space and pick up leading +/- sign if any.
      * If base is 0, allow 0x for hex and 0 for octal, else
      * assume decimal; if base is already 16, allow 0x.
      */
     s = nptr;
     do {
         c = (unsigned char) *s++;
     } while (isspace(c));
     if (c == '-') {
         neg = 1;
         c = *s++;
     } else {
         neg = 0;
         if (c == '+')
             c = *s++;
     }
     if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
         c = s[1];
         s += 2;
         base = 16;
     }
     if (base == 0)
         base = c == '0' ? 8 : 10;

     cutoff = neg ? LONG_MIN : LONG_MAX;
     cutlim = cutoff % base;
     cutoff /= base;
     if (neg) {
         if (cutlim > 0) {
             cutlim -= base;
             cutoff += 1;
         }
         cutlim = -cutlim;
     }
     for (acc = 0, any = 0;; c = (unsigned char) *s++) {
         if (isdigit(c))
             c -= '0';
         else if (isalpha(c))
             c -= isupper(c) ? 'A' - 10 : 'a' - 10;
         else
             break;
         if (c >= base)
             break;
         if (any < 0)
             continue;
         if (neg) {
             if ((acc < cutoff || acc == cutoff) && c > cutlim) {
                 any = -1;
                 acc = LONG_MIN;
                 *result = 1;
                 //errno = ERANGE;
             } else {
                 any = 1;
                 acc *= base;
                 acc -= c;
             }
         } else {
             if ((acc > cutoff || acc == cutoff) && c > cutlim) {
                 any = -1;
                acc = LONG_MAX;
                *result = 2;
                 //errno = ERANGE;
             } else {
                 any = 1;
                 acc *= base;
                 acc += c;
             }
         }
     }
     if (endptr != 0)
         *endptr = (char *) (any ? s - 1 : nptr);

     if (max != -1) {
    	 if (acc < min || acc > max) {
    		 acc = LONG_MIN;
    	     *result = 3;
    	 } else {
    		 *result = 0;
    	 }
     } else {
    	 *result = 0;
     }

     return (acc);
}
