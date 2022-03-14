/*
 * mic_e.h: APRS Mic Encoder special case functions
 */

#include "Command.h"

namespace openaprs {

#define u_char unsigned char

extern int fmt_mic_e(Packet *, const u_char *t,	/* tocall */
	  const u_char *i,	/* info */
	  const int l,		/* length of info */
	  u_char *buf1,		/* output buffer */
	  int *l1,		/* length of output buffer */
	  u_char *buf2,		/* 2nd output buffer */
	  int *l2);		/* length of 2nd output buffer */

}
