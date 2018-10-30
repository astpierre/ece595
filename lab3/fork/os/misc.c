//
//	misc.c
//
//	Miscellaneous routines for the operating system.  These include
//	basic routines that would be in libc if we had access to it.
//

#include "misc.h"

//----------------------------------------------------------------------
//
//	dstrcpy
//	dstrncpy
//	dstrcat
//
//	These three functions do exactly what their counterparts in the
//	standard C library do.
//
//----------------------------------------------------------------------
char*
dstrcpy (char *to, const char *from)
{
  char	*tokeep = to;
  char	last;

  do {
    last = *(to++) = *(from++);
  } while (last != '\0');
  return (tokeep);
}

char*
dstrncpy (char *to, const char *from, int n)
{
  char	*tokeep = to;
  char	last;

  do {
    // Stop if we're exceeded the maximum string length
    if (n-- <= 0) {
      break;
    }
    last = *(to++) = *(from++);
  } while (last != '\0');
  return (tokeep);
}

char *
dstrcat (char *onto, const char *addn)
{
  char	*konto = onto;

  while (*onto != '\0') {
    onto++;
  }
  dstrcpy (onto, addn);
  return (konto);
}

int
dstrncmp (const char *s1, const char *s2, int n)
{
  int		i;

  for (i = 0; i < n; i++) {
    // If they don't match, end the loop
    if (*s2 == '\0') {
      // If the second string is NULL, we're at the end of the string and
      // have a match.
      return (0);
    } else if (*s1 != *s2) {
      break;
    }
    s1++;
    s2++;
  }
  if (i == n) {
    return (0);
  } else {
    return (((*s1 - *s2) < 0) ? -1 : ((*s1 == *s2) ? 0 : 1));
  }
}

int
dstrlen (const char *s)
{
  int		i = 0;

  while (*(s++) != '\0') {
    i++;
  }
  return (i);
}

const char *
dstrstr (const char *buf, const char *pattern)
{
  int	n = dstrlen (pattern);

  while (*buf != '\0') {
    if (dstrncmp (buf, pattern, n) == 0) {
      return (buf);
    }
    buf++;
  }
  return ((char *)0);
}


//----------------------------------------------------------------------
//
//	dindex
//	dmindex
//
//	dindex is identical to the libc function index.  dmindex is
//	similar to dindex, but it reports the index in the string of the
//	first match for any one of the characters passed (rather than
//	matching only on a single specific character).
//
//----------------------------------------------------------------------
const char *
dmindex (const char *s, const char *match)
{
    const char	*c;

    while (*s != '\0') {
	for (c = match; *c != '\0'; c++) {
	    if (*s == *c) {
		return (s);
	    }
	}
    }
    return ((const char *)0);
}

const char*
dindex (const char *s, int c)
{
    while (*s != '\0') {
	if (*s == c) {
	    return (s);
	}
	s++;
    }
    return ((char *)0);
}

//----------------------------------------------------------------------
//
//	ditoa: Convert an integer into an ASCII number.
//	dstrtol: Convert ASCII into an integer
//
//----------------------------------------------------------------------
void
ditoa (int n, char *buf)
{
    int		k;

    if (n < 0) {
	*(buf++) = '-';
	n = -n;
    } else if (n == 0) {
	*(buf++) = '0';
    } else {
	for (k = 1; k <= n; k *= 10) {
	}
	do {
	    k /= 10;
	    *(buf++) = (n / k) + '0';
	    n %= k;
	} while (k > 1);
    }
    *buf = '\0';
}

int
dstrtol (char *c,  char **newpos, int base)
{
    int		v = 0;
    int		curdigit;
    int		sign;

    while ((*c == ' ') || (*c == '\t') || (*c == '\n')) {
	c++;
    }
    if (newpos != (char **)0) {
	*newpos = c;
    }
    if (*c == '\0') {
	return (v);
    }

    if (*c == '-') {
	sign = -1;
	c++;
    } else {
	sign = 1;
    }
    if (base == 0) {
	if (*c == '0') {
	    c++;
	    if ((*c == 'x') || (*c == 'X')) {
		base = 16;
		c++;
	    } else {
		base = 8;
	    }
	} else {
	    base = 10;
	}
    }
    if ((base <= 0) || (base > 16)) {
	return (v);
    }
    do {
	if ((*c >= '0') && (*c <= '9')) {
	    curdigit = (*c) - '0';
	} else if ((*c >= 'a') && (*c <= 'z')) {
	    curdigit = (*c) + 10 - 'a';
	} else if ((*c >= 'A') && (*c <= 'Z')) {
	    curdigit = (*c) + 10 - 'A';
	} else {
	    curdigit = 1000;
	}
	if (curdigit < base) {
	    v *= base;
	    v += curdigit;
	    c++;
	}
    } while (curdigit < base);
    if (newpos != (char **)0) {
	*newpos = c;
    }
    v *= sign;
    return (v);
}

//----------------------------------------------------------------------
//
//	bcopy: Copy bytes from one location to another.
//	bzero: Set all the bytes in a region to zero.
//
//----------------------------------------------------------------------
void
bcopy (char *src, char *dst, int count)
{
  while (count-- > 0) {
    *(dst++) = *(src++);
  }
}

void
bzero (char *dst, int count)
{
  while (count-- > 0) {
    *(dst++) = 0;
  }
}

//----------------------------------------------------------------------
//
//	Math functions
//
//----------------------------------------------------------------------

int min(int a, int b) {
  return ((a < b) ? a : b);
}

int max(int a, int b) {
  return (( a > b) ? a : b);
}
