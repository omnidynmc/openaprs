/* mic_e.cpp   Aug 10, 2000

  

   This is WA4DSY's version of mic_e.c modified to compile
   properly with g++ and not emit various warnings.  April 5, 1999 
   
   In June of 2000 I corrected numerous errors and attempted to
   bring this into compliance with the APRS Protocol Reference 1.0 .
   
   8-10-00: removed gmt time stamp from converted packet and changed
            "E>dsy" to "Mic-E".
   
   Dale Heatherington,  dale@wa4dsy.net
   
   */




/*
 * mic_e: function to reformat APRS Mic Encoder compressed position report
 *  into a conventional (non-binary) tracker report.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * (See the file "COPYING" that is included with this source distribution.)
 * 
 * Copyright (c) 1997,1999 Alan Crosswell
 * Alan Crosswell, N2YGK
 * 144 Washburn Road
 * Briarcliff Manor, NY 10510, USA
 * n2ygk@weca.org
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define _REENTRANT
#define _PTHREADS

#include <string>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "Command.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "mic_e.h"

#undef DEBUG

namespace openaprs {

static char *msgname[] = {
  "EMERGENCY.",
  "PRIORITY..",
  "Special...",
  "Committed.",
  "Returning.",
  "In Service",
  "Enroute...",
  "Off duty..",
  "          ",
  "Custom-6..",
  "Custom-5..",
  "Custom-4..",
  "Custom-3..",
  "Custom-2..",
  "Custom-1..",
  "Custom-0.."
};

using namespace std;

/* Latitude digit lookup table based on APRS Protocol Spec 1.0, page 42 */
static char lattb[] = "0123456789       0123456789     0123456789 ";

static unsigned int hex2i (u_char, u_char);


/* if this function fails to convert the packet it returns ZERO
   and the user supplied length variables, l1 and l2 are zero.
   Otherwise it returns non-zero values. */

int
fmt_mic_e (Packet *aprsPacket,
           const u_char * t,	/* tocall */
	   const u_char * i,	/* info */
	   const int l,		/* length of info */
	   u_char * buf1,	/* output buffer */
	   int *l1,		/* length of output buffer */
	   u_char * buf2,	/* 2nd output buffer */
	   int *l2)		/* length of 2nd output buffer */
{
  u_int msg, sp, dc, se, spd, cse;
  char north, west, c;
  int lon1, lonDD, lonMM, lonHH;
  char *bp;

  enum
  { none = 0, BETA, REV1 }
  rev = none;			/* mic_e revision level */
  int gps_valid = 0;
  char symtbl = '/', symbol = '$', adti = '!';
  int buf2_n = 0;
  char lat[6];
  int x;
  unsigned lx;
  char msgclass;
  int trash = 0;

  /* Try to avoid conversion of bogus packets   */

  if (l > 255)
    return 0;			/* info field must be less than 256 bytes long */

  if (strlen ((char *) t) != 6)
    return 0;			/* AX25 dest addr must be 6 bytes long. */

  for (x = 0; x < 6; x++)
    {
      c = t[x];			/* search for invalid characters in the ax25 destination */
      if ((c < '0')		/* Nothing below zero */
	  || (c > 'Z')		/* Nothing above 'Z' */
	  || (c == 'M')		/* M,N,O not allowed */
	  || (c == 'N') || (c == 'O'))
	trash |= 1;

      if ((c > '9') && (c < 'A'))
	trash |= 2;		/* symbols :;<>?@ are invalid */
      if ((x >= 3) && (c >= 'A') && (c <= 'K'))
	trash |= 4;		/* A-K not allowed in last 3 bytes */
    }

  for (x = 1; x < 7; x++)
    {
      if ((i[x] < 0x1c) || (i[x] > 0x7f))
	trash |= 8;		// Range check the longitude and speed/course
    }

  if (trash) {
#ifdef DEBUG
    printf("Trash=%d dst(%s) info(%s)\n",trash, t, i);   //Debug
#endif

    return 0;			/* exit without converting packet if it's malformed */
  } // if

  *l1 = *l2 = 0;

/* This switch statement processes the 1st information field byte, byte 0 */

  switch (i[0])
    {				/* possible valid MIC-E flags */
    case 0x60:			/* ` Current GPS data (not used in Kenwood TM-D700) */
      gps_valid = 1;
      rev = REV1;
      if (l < 9)
	return 0;		/* reject packet if less than 9 bytes */
      break;

    case 0x27:			/* ' Old GPS data or Current data in Kenwood TM-D700 */
      gps_valid = 0;
      rev = REV1;
      if (l < 9)
	return 0;		/* reject packet if less than 9 bytes */
      break;

    case 0x1c:			/* Rev 0 beta units current GPS data */
      gps_valid = 1;
      rev = BETA;
      if (l < 8)
	return 0;		/* reject packet if less than 8 bytes */
      break;

    case 0x1d:			/* Rev 0 beta units old GPS data */
      gps_valid = 0;
      rev = BETA;
      if (l < 8)
	return 0;		/* reject packet if less than 8 bytes */
      break;

    default:
      return 0;			/* Don't attempt to process a packet that's not a Mic-E type */
    }


  if ((i[9] == ']') || (i[9] == '>'))
    adti = '=';			/* Kenwood with messaging detected */



  if (l >= 7)
    {				/* <-- old beta units could have only 7 bytes, new units should have 9 or more bytes */


      msg =
	((t[0] >= 'P') ? 4 : 0) | ((t[1] >= 'P') ? 2 : 0) | ((t[2] >= 'P') ? 1
							     : 0);	//Standard messages  0..7


      if (msg == 0)
	{
	  msg = (((t[0] >= 'A') && (t[0] < 'L')) ? 0xC : 0)	//Custom messages  9..15
	    | (((t[1] >= 'A') && (t[1] < 'L')) ? 0xA : 0)
	    | (((t[2] >= 'A') && (t[2] < 'L')) ? 9 : 0);

	}
      msg &= 0xf;		/* make absolutly sure we don't exceed the array bounds */

      msgclass = (msg > 7) ? 'C' : 'M';	/* Message class is either standard=M or custom=C */

/* Recover the latitude digits from the ax25 destination field */
      for (x = 0; x < 6; x++)
	{
	  lx = t[x] - '0';	//subtract offset from raw latitude character
	  if ((lx >= 0) && (lx < strlen (lattb)))
	    {
	      lat[x] = lattb[lx];	/* Latitude output digits into lat[] array */
	    }
	  else
	    lat[x] = ' ';
	}

      if (lat[0] == '9')
	{
	  if ((lat[1] != '0')	//reject lat degrees > 90 degrees
	      || (lat[2] != '0')
	      || (lat[3] != '0') || (lat[4] != '0') || (lat[5] != '0'))
	    return 0;
	}


#if 0
      printf ("lat[2] = %d lat[4] = %d\n", lat[2], lat[4]);
      printf ("lat[2] = %c lat[4] = %c\n", lat[2], lat[4]);
      printf ("lat = %s\n", lat);

      if (lat[2] > '5')
	return 0;
      //reject lat minutes >= 60
      if (lat[4] > '5')
	return 0;
      //reject lat seconds >= 60
#endif



      if ((lat[0] == ' ') || (lat[1] == ' '))
	return 0;		//Reject ambiguious degrees


      north = t[3] >= 'P' ? 'N' : 'S';
      west = t[5] >= 'P' ? 'W' : 'E';
      lon1 = t[4] >= 'P';	/* +100 deg offset */

/* Process info field bytes 1 through 6; the Longitude, speed and course bits */
      lonDD = i[1] - 28;
      lonMM = i[2] - 28;
      lonHH = i[3] - 28;

#ifdef DEBUG
      fprintf (stderr,
	       "raw lon 0x%02x(%d) 0x%02x(%d) 0x%02x(%d) 0x%02x(%d)\n", i[1],
	       i[1], i[2], i[2], i[3], i[3], i[4], i[4]);
      fprintf (stderr, "before: lonDD=%d, lonMM=%d, lonHH=%d lon1=0x%0x\n",
	       lonDD, lonMM, lonHH, lon1);
#endif

      if (rev >= REV1)
	{
	  if (lon1)		/* do this first? */
	    lonDD += 100;
	  if (180 <= lonDD && lonDD <= 189)
	    lonDD -= 80;
	  if (190 <= lonDD && lonDD <= 199)
	    lonDD -= 190;
	  if (lonMM >= 60)
	    lonMM -= 60;
	}
#ifdef DEBUG
      fprintf (stderr, "after: lonDD=%d, lonMM=%d, lonHH=%d lon1=0x%0x\n",
	       lonDD, lonMM, lonHH, lon1);
#endif
      sp = i[4] - 28;
      dc = i[5] - 28;
      se = i[6] - 28;
      buf2_n = 6;		/* keep track of where we are */
#ifdef DEBUG
      fprintf (stderr, "sp=%02x dc==%02x se=%02x\n", sp, dc, se);
#endif

      spd = sp * 10 + dc / 10;
      cse = (dc % 10) * 100 + se;
      if (rev >= REV1)
	{
	  if (spd >= 800)
	    spd -= 800;
	  if (cse >= 400)
	    cse -= 400;
	}

      if (cse > 360)
	return 0;		// Reject invalid course value
      if (spd > 999)
	return 0;		// reject invalid speed value

#ifdef DEBUG
      fprintf (stderr, "symbol=0x%02x, symtbl=0x%02x\n", i[7], i[8]);
#endif

/* Process information field bytes 7 and 8 (Symbol Code and Sym Table ID) */

      symtbl = (l >= 8 && rev >= REV1) ? i[8] : '/';
      /* rev1 bug workaround:  sends null symbol/table bytes */

      symbol = i[7];
      if (!isprint (symbol))
	return 0;		/* Reject packet if invalid symbol code */



      if ((symtbl != '/' && symtbl != '\\')
	  && !('0' <= symtbl && symtbl <= '9')
	  && !('a' <= symtbl && symtbl <= 'j')
	  && !('A' <= symtbl && symtbl <= 'Z')
	  && (symtbl != '*' && symtbl != '!'))
	{
	  //    printf("Bad symtbl code = %c\n",symtbl);  //Debug
	  return 0;		/* Reject packet if invalid symtbl code */

//    printf("acket\n");      

	}
      buf2_n = (rev == BETA) ? 8 : 9;
      if (l >= 10)
	{
	  buf2_n = (rev == BETA) ? 8 : 9;
	  switch (i[buf2_n])
	    {

	    case 0x60:		/* two-channel telemetry (Hex data) */
	      sprintf ((char *) buf2, "T#MIC,%03d,%03d",
		       hex2i (i[buf2_n + 1], i[buf2_n + 2]),
		       hex2i (i[buf2_n + 3], i[buf2_n + 4]));
	      buf2_n += 5;
	      *l2 = strlen ((char *) buf2);
	      break;

	    case 0x27:		/* five-channel telemetry (Hex data) */
	      sprintf ((char *) buf2, "T#MIC,%03d,%03d,%03d,%03d,%03d",
		       hex2i (i[buf2_n + 1], i[buf2_n + 2]),
		       hex2i (i[buf2_n + 3], i[buf2_n + 4]),
		       hex2i (i[buf2_n + 5], i[buf2_n + 6]),
		       hex2i (i[buf2_n + 7], i[buf2_n + 8]),
		       hex2i (i[buf2_n + 9], i[buf2_n + 10]));
	      buf2_n += 11;
	      *l2 = strlen ((char *) buf2);
	      break;

	    case 0x1d:		/* five-channel telemetry (This works only if the input device supports raw binary data) */
	      sprintf ((char *) buf2, "T#MIC,%03d,%03d,%03d,%03d,%03d",
		       i[buf2_n + 1], i[buf2_n + 2], i[buf2_n + 3],
		       i[buf2_n + 4], i[buf2_n + 5]);
	      buf2_n += 6;
	      *l2 = strlen ((char *) buf2);
	      break;

	    default:
	      break;		/* no telemetry */
	    }			/* end switch */
	}



      /* This sprintf statement creates the output string */

      sprintf ((char *) buf1,
	       "%c%c%c%c%c.%c%c%c%c%03d%02d.%02d%c%c%03d/%03d/Mic-E/%c%d/%s",
	       adti, lat[0], lat[1], lat[2], lat[3], lat[4], lat[5], north,
	       symtbl, lonDD, lonMM, lonHH, west, symbol, cse, spd, msgclass,
	       ~msg & 0x7, msgname[msg]);

		/* KB0THN: the above line is a little hard to read. msgclass is either 'C' for a custom message
		 * or 'M' for a standard message. ~msg & 0x7 gives you a message code based on the index into his
		 * const array at the top, and msgname[msg] gives you text of the message */

      bp = (char *) &buf1[(*l1 = strlen ((char *) buf1))];

//printf("<-- buf2_n = %d l = %d\n",buf2_n,l);

      if (buf2_n < l)
	{			/* additional comments/btext */
	  *bp++ = ' ';		/* add a space */
	  (*l1)++;
	  memcpy (bp, &i[buf2_n], l - buf2_n);
	  *l1 += l - buf2_n;
	}
    }

  // printf("buf2_n = %d l=%d  %s\n",buf2_n,l,buf1);
  return ((*l1 > 0) + (*l2 > 0));
  //return (strlen(buf1) - buf2_n);
}

/*------------------------------------------------------------------------*/

static unsigned int
hex2i (u_char a, u_char b)
{
  unsigned int r = 0;

  if (a >= '0' && a <= '9')
    {
      r = (a - '0') << 4;
    }
  else if (a >= 'A' && a <= 'F')
    {
      r = (a - 'A' + 10) << 4;
    }
  if (b >= '0' && b <= '9')
    {
      r += (b - '0');
    }
  else if (b >= 'A' && b <= 'F')
    {
      r += (b - 'A' + 10);
    }
  return r;
}

}
/*------------------------------END---------------------------------------------*/

#if 0
// My test routine
int
main (void)
{
#if 0
  int fmt_mic_e (const u_char * t,	/* tocall */
		 const u_char * i,	/* info */
		 const int l,	/* length of info */
		 u_char * buf1,	/* output buffer */
		 int *l1,	/* length of output buffer */
		 u_char * buf2,	/* 2nd output buffer */
		 int *l2,	/* length of 2nd output buffer */
		 time_t tick)	/* received timestamp of the packet */
#endif
  char buff1[1024];
  char buff2[1024];
  char buff3[1024];
  char buff4[1024];
  int foo;
  int foo1;


  strcpy (buff1, "3X2PPU");
  strcpy (buff2, "'lSAl 7u/]\"6.}Lee's TM-D700A!");
  strcpy (buff3, "");
  strcpy (buff4, "");

  foo = strlen (buff2);
  foo1 = 1024;

//fmt_mic_e(buff1,buff2,foo,buff3,&foo1,buff4,&foo1,991192956);
  fmt_mic_e (buff1, buff2, foo, buff3, &foo1, buff4, &foo1);

  printf ("strlen = %d\n", foo);
  printf ("buff1 = %s\n", buff1);
  printf ("buff2 = %s\n", buff2);
  printf ("buff3 = %s\n", buff3);
  printf ("buff4 = %s\n", buff4);
}

#endif
#undef DEBUG
