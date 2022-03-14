/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                    Daniel Robert Karrels                             **
 **                    Dynamic Networking Solutions                      **
 **                                                                      **
 ** This program is free software; you can redistribute it and/or modify **
 ** it under the terms of the GNU General Public License as published by **
 ** the Free Software Foundation; either version 1, or (at your option)  **
 ** any later version.                                                   **
 **                                                                      **
 ** This program is distributed in the hope that it will be useful,      **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of       **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        **
 ** GNU General Public License for more details.                         **
 **                                                                      **
 ** You should have received a copy of the GNU General Public License    **
 ** along with this program; if not, write to the Free Software          **
 ** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            **
 **************************************************************************
 $Id: openaprs_string.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
 **************************************************************************/

#include <cctype>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <string>
#include <list>

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <regex.h>

#include "Server.h"

#include "config.h"
#include "openaprs.h"
#include "openaprs_string.h"


namespace openaprs {

using std::string;

int cmp(const char *s1, const char *s2) {
  return (strcasecmp(s1, s2));
}

int stpcrlf(char *string) {
  /* Loops through a string, terminates on <CR> or <LF> */
  while (*string) {
    if (*string == '\r' || *string == '\n')
      *string = '\0';

    string++;
  } // while

  return 1;
}

const string dec2oct(const unsigned int convertMe) {
  char tempBuf[4];

  snprintf(tempBuf, 4, "%.3o", convertMe);

  return tempBuf;
} // dec2oct

const string f2c(const double convertMe) {
  char tempBuf[7];
  double temperature;

  temperature = ((convertMe - 32) / 9) * 5;

  snprintf(tempBuf, 7, "%0.2f", temperature);

  return tempBuf;
} // f2c

char *mask(char *hostmask, const char *host, const int hostsize) {
  char mask[HOSTLEN + 1] = {0};
  char temphost[HOSTLEN + 1] = {0};
  char *splithost[HOSTLEN + 1];
  char *ident;
  char *ch;
  int i = 0;
  int j = 0;

  if (host == NULL)
    return NULL;

  strncpy(temphost, host, sizeof(temphost));

  ident = strtok(temphost, "@");

  if (*ident == '~')
    *ident = '*';

  ch = strtok(NULL, "@");

  for (splithost[i = 0] = strtok(ch, ".");
       splithost[i];
       splithost[++i] = strtok(NULL, "."));

  switch (i) {
    case 2:
      snprintf(mask, sizeof(mask), "%s.%s", splithost[0], splithost[1]);
      break;
    default:
      if (i == 4 &&
	  strlen(splithost[0]) < 4 &&
	  strlen(splithost[1]) < 4 &&
	  strlen(splithost[2]) < 4 &&
	  strlen(splithost[3]) < 4)
	snprintf(mask, sizeof(mask), "%s.%s.%s.*", splithost[0], splithost[1], splithost[2]);
      else {
	mask[0] = '*';
	for (j = 1; j < i; j++) {
	  strcat(mask, ".");
	  strcat(mask, splithost[j]);
	}
      }
      break;
  }

  snprintf(hostmask, hostsize, "%s@%s", ident, mask);
  return hostmask;
}

bool isnum(const char *ch) {
  while(*ch) {
    if (!isdigit(*ch))
      return(false);

    ch++;
  } 
  return true;
}

const string openaprs_string_graph(const int graphMin, const int graphMax, const int graphLength) {
  float ourGoal;		// graph calc
  int i;			// counter
  string goalGraph;		// graph image

  // setup our graph
  ourGoal = ((float) graphMin / (float) graphMax) * (float) graphLength;

  goalGraph = "[";
  for(i = 1; i <= graphLength; i++)
    if (i <= (int) ourGoal)
      goalGraph += "X";
        else
      goalGraph += ".";
  goalGraph += "]";

  return goalGraph;
} // openaprs_string_graph

const string &openaprs_string_trim(string &trimMe) {
  string tempBuf;             // temp buffer
  unsigned int i;             // counter

  for(i = 0; i < trimMe.length(); i++)
    if (trimMe[i] != ' ')
      break;

  tempBuf = trimMe.substr(i, trimMe.length());
  trimMe = tempBuf;

  for(i = trimMe.length(); i > 0; i--)
    if (trimMe[(i - 1)] != ' ')
      break;

  tempBuf = trimMe.substr(0, i);
  trimMe = tempBuf; 

  return trimMe;
} // openaprs_string_trim

const bool openaprs_string_acceptable(const char *acceptChars, const string &acceptMe) {
  unsigned int i;			// counter
  unsigned int j;			// counter

  // make sure the search is acceptable
  for(i = 0; i < acceptMe.length(); i++) {
    for(j = 0; j < strlen(acceptChars); j++) {
      if (*(acceptChars + j) == acceptMe[i])
        break;
    } // for
      
    // we didn't find it as an acceptible char
    if (j == strlen(acceptChars))
      return false;  
  } // for

  return true;
} // openaprs_string_acceptable

const unsigned int openaprs_string_strip_crlf(string &stripMe) {
  string tempBuf;		// temp buffer
  unsigned int i;		// counter
  unsigned int j;		// counter

  // initialize variables
  tempBuf = "";
  j = 0;

  for(i = 0; i < stripMe.length(); i++) {
    if (stripMe[i] == '\r' || stripMe[i] == '\n') {
      j++;
      continue;
    } // if

    tempBuf += stripMe[i];
  } // for

  stripMe = tempBuf;

  return j;
} // openaprs_string_strip_crlf

const bool openaprs_string_valid_hostmask(const string validHostmask, const bool validShort) {
  char acceptChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~@!*";
  char acceptCharsShort[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~@*";
  size_t pos;			// poisition in string
  size_t pos2;			// position in string

  if (validShort == false)
    // if there is no !. host is invalid
    if ((pos = validHostmask.find("!")) == string::npos)
      return false;

  // if there is no @, host is invalid
  if ((pos2 = validHostmask.find("@")) == string::npos)
    return false;

  if (validShort == false)
    // if ! is after @ it's not a valid hostname
    if (pos > pos2)
      return false;

  if (validShort == false)
    return openaprs_string_acceptable(acceptChars, validHostmask);
  else
    return openaprs_string_acceptable(acceptCharsShort, validHostmask);
} // openaprs_string_valid_hostmask

const bool openaprs_string_valid_username(const string validUsername) {
  char acceptChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_`[]";

  // usernames are a max of 16 characters
  if (validUsername.size() > 16)
    return false;

  return openaprs_string_acceptable(acceptChars, validUsername);
} // openaprs_string_valid_username

const unsigned int openaprs_string_pad(string &padMe, const unsigned int padLength) {
  unsigned int i;			// counter

  for(i = 0; i < padLength; i++) {
    padMe += " ";
  } // for

  return padMe.length();
} // openaprs_string_pad

const unsigned int openaprs_string_to_lower(string &lowerMe) {
  size_t pos;                   // poisition in string
 
  // string remaining color codes
  for(pos = 0; pos < lowerMe.length(); pos++)
    lowerMe[pos] = tolower(lowerMe[pos]);

  return lowerMe.length();
} // openaprs_string_to_lower

const string &openaprs_string_int(string &returnString, int convertMe) {
  char convertBuffer[16];		// convert buffer

  snprintf(convertBuffer, sizeof(convertBuffer), "%d", convertMe);

  returnString = convertBuffer;

  return returnString;
} // openaprs_string_int

const bool openaprs_string_duration(const string durationString, unsigned int &returnDuration) {
  char acceptChars[] = "0123456789";
  char timeSymbol[] = { 'm', 'h', 'd', 'w', 'n', 'y', 0 };
  int timeValue[] = { OPENAPRS_TIME_MINUTE,		OPENAPRS_TIME_HOUR,
                      OPENAPRS_TIME_DAY,		OPENAPRS_TIME_WEEK,
                      OPENAPRS_TIME_MONTH,		OPENAPRS_TIME_YEAR,
                      0 };
  unsigned int oldDuration;					// old duration
  unsigned int newDuration;					// new duration
  unsigned int i;						// counter

  if (durationString.length() < 2 ||
      openaprs_string_acceptable(acceptChars, durationString.substr(1, durationString.length())) == false)
    return false;

  // initialize variables
  oldDuration = newDuration = atoi(durationString.substr(1, durationString.length()).c_str());

  for(i = 0; timeSymbol[i] != 0; i++)
    if (durationString[0] == timeSymbol[i])
      newDuration = newDuration * timeValue[i];
      
  if (newDuration == oldDuration)
    return false;

  // set our return variables
  returnDuration = newDuration;

  return true;
} // openaprs_string_duration

const bool openaprs_timeval_duration(const time_t durationTime, struct openaprs_duration &openaprs_duration) {
  openaprs_duration.days = durationTime / 86400;
  openaprs_duration.days_total = durationTime / 86400;
  openaprs_duration.hours = (durationTime / 3600) - (openaprs_duration.days * 24);
  openaprs_duration.hours_total = durationTime / 3600;
  openaprs_duration.minutes = (durationTime / 60) - ((openaprs_duration.hours * 60) + (openaprs_duration.days * 1440));
  openaprs_duration.minutes_total = durationTime / 60;
  openaprs_duration.seconds = durationTime - ((openaprs_duration.minutes * 60) + (openaprs_duration.hours * 3600) + (openaprs_duration.days * 86400));
  openaprs_duration.seconds_total = durationTime;

  return true;
} // openaprs_timeval_duration

const bool openaprs_timeval_difference(const struct timeval &tvOld, const struct timeval &tvNew, struct timeval &tvDifference) {
  unsigned long int differenceMicroseconds;			// difference in microseconds

  differenceMicroseconds = (unsigned long int) ((tvNew.tv_sec * 1000000) + tvNew.tv_usec) - ((tvOld.tv_sec * 1000000) + tvOld.tv_usec);
  tvDifference = (struct timeval) { differenceMicroseconds / 1000000, differenceMicroseconds % 1000000 };

  return true;
} // openaprs_timeval_difference

const char *openaprs_date(time_t dateTS) {
  int minswest;				// number of minutes west
  static const char *months[] = { "January", "February",
                                  "March", "April",
                                  "May", "June",
                                  "July", "August",
                                  "September", "October",
                                  "November", "December" };
  static const char *weekdays[] = { "Sunday",	"Monday",
               		        "Tuesday", 	"Wednesday",
              		         "Thursday",	"Friday",
                     		  "Saturday" };
  char plus;				// + or - time
  struct tm *lt;			// local time structure
  struct tm *gm;			// gm time structure
  struct tm gmbuf;			// gm time buffer structure
  static char buf[80];			// buffer to put time into

  if (!dateTS)
    dateTS = time(NULL);

  gm = gmtime(&dateTS);
  memcpy(&gmbuf, gm, sizeof(gmbuf));
  gm = &gmbuf;
  lt = localtime(&dateTS);

  minswest = (gm->tm_hour - lt->tm_hour) * 60 + (gm->tm_min - lt->tm_min);
  if (lt->tm_yday != gm->tm_yday) {
    if ((lt->tm_yday > gm->tm_yday && lt->tm_year == gm->tm_year) ||
        (lt->tm_yday < gm->tm_yday && lt->tm_year != gm->tm_year))
      minswest -= 24 * 60;
    else
      minswest += 24 * 60;
  } // if
  
  plus = (minswest > 0) ? '-' : '+';
  if (minswest < 0)
    minswest = -minswest;
  
  snprintf(buf, sizeof(buf), "%s %s %d %d -- %02d:%02d %c%02d:%02d",
      weekdays[lt->tm_wday], months[lt->tm_mon], lt->tm_mday,
      1900 + lt->tm_year, lt->tm_hour, lt->tm_min,
      plus, minswest / 60, minswest % 60);
  
  return buf;
} // openaprs_date

const unsigned int openaprs_string_replace(const string &replaceMe, const string &replaceWith, string &replaceLine) {
  unsigned int i;		// counter
  size_t pos;		// position in string

  // initialize variables
  i = 0;
  pos = 0;

  for(pos = replaceLine.find(replaceMe, pos); pos != string::npos; pos = replaceLine.find(replaceMe, pos)) {
    i++;
    replaceLine.replace(pos, replaceMe.length(), replaceWith);
    pos += replaceWith.length();
  } // for

  return i;
} // openaprs_string_replace

const unsigned int openaprs_string_wordwrap(const string &wrapMe, unsigned int maxChars, list<string> &wrapList) {
  string tempLine;		// temp line to be wraped
  unsigned int i;		// counter
  unsigned int lastSpace;	// last space

  // initialize variables
  i = 0;
  lastSpace = 0;
  tempLine = "";

  for(i = 0; i < wrapMe.length(); i++) {
    if (wrapMe[i] == ' ')
      lastSpace = i;

    if (tempLine.length() == maxChars) {
      // if we can find the last space and cut there
      if (lastSpace > 0) {
        tempLine.erase(tempLine.length() - (i - lastSpace), tempLine.length());
        i = lastSpace;
      } // if

      wrapList.push_back(tempLine);
      tempLine = "";
      lastSpace = 0;
    } // if

    tempLine += wrapMe[i];
  } // for

  if (tempLine.length() > 0)
    wrapList.push_back(tempLine);

  return wrapList.size();
} // openaprs_string_wordwrap

const bool openaprs_string_regex(const string &regexRegex, const string &regexString, map<int, regexMatch> &regexList) {
  regexMatch aMatch;		// regex match
  char errbuf[255];		// error buffer
  regex_t preg;			// preg
  regmatch_t *pmatch = NULL;	// pmatch
  size_t nmatch;		// nmatch
  int errcode;			// error code
  unsigned int i;		// counter

  // compile regex
  if ((errcode = regcomp(&preg, regexRegex.c_str(), REG_EXTENDED)) != 0) {
    // try and snag the text for the error
    regerror(errcode, &preg, errbuf, 255);

    app->writeLog(OPENAPRS_LOG_DEBUG, "regcomp()> #%d %s", errcode, errbuf);
    // debug(HERE, "regcomp(): errno(%d) errmsg(%s)\n", errcode, errbuf);

    return false;
  } // if

  // initialize variables
  nmatch = preg.re_nsub + 1;
  pmatch = new regmatch_t[nmatch];
  regexList.clear();

  if (regexec(&preg, regexString.c_str(), nmatch, pmatch, 0) != 0) {
    // try and snag the text for the error
    regerror(errcode, &preg, errbuf, 255);

    if (errcode != 0)
      app->writeLog(OPENAPRS_LOG_DEBUG, "regexec()> #%d %s", errcode, errbuf);
    //  debug(HERE, "regcomp(): errno(%d) errmsg(%s)\n", errcode, errbuf);

    // cleanup
    delete [] pmatch;
    regfree(&preg);

    return false;
  } // if

  // save the results
  for(i = 0; i < nmatch && pmatch[i].rm_so != -1 && pmatch[i].rm_eo != -1; i++) {
    // cout << i << endl;
    // cout << pmatch[i].rm_so << " " << pmatch[i].rm_eo << endl;
    if (pmatch[i].rm_so == -1 || pmatch[i].rm_eo == -1)
      continue;

    aMatch.matchString = regexString.substr(pmatch[i].rm_so, (pmatch[i].rm_eo - pmatch[i].rm_so));
    aMatch.matchBegin = pmatch[i].rm_so;
    aMatch.matchEnd = pmatch[i].rm_eo;
    aMatch.matchLength = (pmatch[i].rm_eo - pmatch[i].rm_so);

    // cout << aMatch.matchString << endl;

    regexList[i] = aMatch;
    // debug(HERE, "openaprs_string_regex(%s): Number: %d, Start: %d, End: %d, Match: %s\n", regexRegex.c_str(), nmatch, (long) aMatch.matchBegin, (long) aMatch.matchEnd, aMatch.matchString.c_str());
  } // for

  // cleanup
  delete [] pmatch;
  regfree(&preg);

  return true;
} // openaprs_string_regex

const bool ereg(const string &regexRegex, const string &regexString, map<int, regexMatch> &regexList) {
  regexMatch aMatch;		// regex match
  char errbuf[255];		// error buffer
  regex_t preg;			// preg
  regmatch_t *pmatch = NULL;	// pmatch
  size_t nmatch;		// nmatch
  int errcode;			// error code
  unsigned int i;		// counter

  // compile regex
  if ((errcode = regcomp(&preg, regexRegex.c_str(), REG_EXTENDED)) != 0) {
    // try and snag the text for the error
    regerror(errcode, &preg, errbuf, 255);

    app->writeLog(OPENAPRS_LOG_DEBUG, "regcomp()> #%d %s", errcode, errbuf);
    // debug(HERE, "regcomp(): errno(%d) errmsg(%s)\n", errcode, errbuf);

    return false;
  } // if

  // initialize variables
  nmatch = preg.re_nsub + 1;
  pmatch = new regmatch_t[nmatch];
  regexList.clear();

  if (regexec(&preg, regexString.c_str(), nmatch, pmatch, 0) != 0) {
    // try and snag the text for the error
    regerror(errcode, &preg, errbuf, 255);

    if (errcode != 0)
      app->writeLog(OPENAPRS_LOG_DEBUG, "regexec()> #%d %s", errcode, errbuf);
    //  debug(HERE, "regcomp(): errno(%d) errmsg(%s)\n", errcode, errbuf);

    // cleanup
    delete [] pmatch;
    regfree(&preg);

    return false;
  } // if

  // save the results
  for(i = 0; i < nmatch && pmatch[i].rm_so != -1 && pmatch[i].rm_eo != -1; i++) {
    // cout << i << endl;
    // cout << pmatch[i].rm_so << " " << pmatch[i].rm_eo << endl;
    if (pmatch[i].rm_so == -1 || pmatch[i].rm_eo == -1)
      continue;

    aMatch.matchString = regexString.substr(pmatch[i].rm_so, (pmatch[i].rm_eo - pmatch[i].rm_so));
    aMatch.matchBegin = pmatch[i].rm_so;
    aMatch.matchEnd = pmatch[i].rm_eo;
    aMatch.matchLength = (pmatch[i].rm_eo - pmatch[i].rm_so);

    // cout << aMatch.matchString << endl;

    regexList[i] = aMatch;
    // debug(HERE, "openaprs_string_regex(%s): Number: %d, Start: %d, End: %d, Match: %s\n", regexRegex.c_str(), nmatch, (long) aMatch.matchBegin, (long) aMatch.matchEnd, aMatch.matchString.c_str());
  } // for

  // cleanup
  delete [] pmatch;
  regfree(&preg);

  return true;
} // ereg

const unsigned int openaprs_string_create_list(const list<string> &createList, string &createString, const string &createEnd) {
  list<string> localList;		// local list

  // initialize variables
  createString = "";
  localList = createList;

  // create factoid output
  while(!localList.empty()) {
    if (createString.length() == 0)
      createString = localList.front();
    else {
      if (localList.size() > 1)
        createString += string(", ") + localList.front();
      else
        createString += string(" ") + createEnd + string(" ") + localList.front();
    } // else

    localList.pop_front();
  } // while
    
  return createString.length();
} // openaprs_string_create_list

const string strtoupper(const string &upMe) {
  string ret;
  size_t pos;

  for(pos=0; pos < upMe.length(); pos++) {
    if (upMe[pos] >= 97 && upMe[pos] <= 122)
      ret += upMe[pos]-32;
    else
      ret += upMe[pos];
  } // for
    
  return ret;
} // strtoupper

const string strtolower(const string &upMe) {
  string ret;
  size_t pos;

  for(pos=0; pos < upMe.length(); pos++) {
    if (upMe[pos] >= 64 && upMe[pos] <= 90)
      ret += upMe[pos]+32;
    else
      ret += upMe[pos];
  } // for
    
  return ret;
} // strtoupper

}
