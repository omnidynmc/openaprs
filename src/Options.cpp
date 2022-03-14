/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
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
 $Id: Options.cpp,v 1.2 2006/03/21 20:55:12 omni Exp $
 **************************************************************************/

#include <cstdlib>
#include <cstring>
#include <list>
#include <string>
#include <sstream>
#include <iostream>

#include <time.h>
#include <unistd.h>

#include "Options.h"

#include "openframe/StringToken.h"

#include "match.h"

namespace openaprs {
  using openframe::StringToken;
  using namespace std;

/**************************************************************************
 ** Option Class                                                         **
 **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  Option::Option() {
  } // Option

  Option::~Option() {
  } // Option

/**************************************************************************
 ** OptionsList Class                                                    **
 **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  OptionsList::OptionsList() {
  } // OptionsList::OptionsList

  OptionsList::~OptionsList() {
  } // OptionsList::~OptionsList

  /*********************
   ** Options Members **
   *********************/

  /**
   * OptionsList::addOption
   *
   * Adds an option to the option handler.
   *
   * Returns: True if successful, false if failed.
   */
  const bool OptionsList::addOption(const string &addName, 
                                    const int addInt, 
                                    const bool addBool,
                                    const string &addString,
                                    const bool addOverwrite
                                   ) {
    Option aOption;                           // option
    optionMapType::iterator ptr;              // pointer to a map

    mutex.Lock();

    // is there already an option by that name?
    if ((ptr = optionList.find(addName)) != optionList.end()) {
      if (addOverwrite == false) {
         mutex.Unlock();
         return false;
      } // if

      // update that options
      ptr->second.setInteger(addInt);
      ptr->second.setBoolean(addBool);
      ptr->second.setString(addString);

      mutex.Unlock();
      return true;
    } // if

    // initialize variables
    aOption.setInteger(addInt);
    aOption.setBoolean(addBool);
    aOption.setString(addString);

    optionList.insert(pair<string, Option>(addName, aOption));

    mutex.Unlock();

    return true;
  } // OptionsList::addOption

  const bool OptionsList::addOption(const string &addName, const int addInt, const bool addBool, const string &addString) {
    return addOption(addName, addInt, addBool, addString, true);
  } // OptionsList::addOption

  /**
   * OptionsList::addOption
   *
   * Adds an option to the option handler.
   *
   * Returns: True if successful, false if failed.
   */
  const bool OptionsList::addOption(const string &addName, const int addInt) {
    stringstream s;			// String stream to convert ints :)

    mutex.Lock();

    // initialize variables
    s.str("");
    s << addInt;

    mutex.Unlock();

    return addOption(addName, addInt, true, s.str(), true);
  } // OptionsList::addOption

  /**
   * OptionsList::addOption
   *
   * Adds an option to the option handler.
   *
   * Returns: True if successful, false if failed.
   */
  const bool OptionsList::addOption(const string &addName, const long int addInt) {
    stringstream s;			// String stream to convert ints :)

    mutex.Lock();

    // initialize variables
    s.str("");
    s << addInt;

    mutex.Unlock();

    return addOption(addName, (int) addInt, true, s.str(), true);
  } // OptionsList::addOption

  /**
   * OptionsList::addOption
   *
   * Adds an option to the option handler.
   *
   * Returns: True if successful, false if failed.
   */
  const bool OptionsList::addOption(const string &addName, const unsigned int addInt) {
    stringstream s;			// String stream to convert ints :)

    mutex.Lock();

    // initialize variables
    s.str("");
    s << addInt;

    mutex.Unlock();

    return addOption(addName, (signed) addInt, true, s.str(), true);
  } // OptionsList::addOption

  /**
   * OptionsList::addOption
   *
   * Adds an option to the option handler.
   *
   * Returns: True if successful, false if failed.
   */
  const bool OptionsList::addOption(const string &addName, const string &addString) {
    return addOption(addName, 1, true, addString, true);
  } // OptionsList::addOption

  /**
   * OptionsList::addOption
   *
   * Adds an option to the option handler.
   *
   * Returns: True if successful, false if failed.
   */
  const bool OptionsList::addOption(const string &addName, const char *addString) {
    return addOption(addName, 1, true, addString, true);
  } // OptionsList::addOption

  /**
   * OptionsList::removeOption
   *
   * Remove an option from the option handler.
   *
   * Returns: True if successful, false if failed.
   */
  const bool OptionsList::removeOption(const string &removeMe) {
    optionMapType::iterator ptr;              // pointer to a map

    mutex.Lock();

    ptr = optionList.find(removeMe);
    if (ptr == optionList.end()) {
      mutex.Unlock();
      return false;
    } // if

    optionList.erase(ptr);

    mutex.Unlock();

    return true;
  } // OptionsList::removeOption


  /**
   * OptionsList::isOption
   *
   * See if option exists.
   *
   * Returns: True if successful, false if failed.
   */
  const bool OptionsList::isOption(const string &findMe) {
    optionMapType::iterator ptr;              // pointer to a map

    mutex.Lock();

    ptr = optionList.find(findMe);
    if (ptr == optionList.end()) {
      mutex.Unlock();
      return false;
    } // if

    mutex.Unlock();
  
    return true;
  } // OptionsList::isOption

  const bool OptionsList::incOption(const string &addName, const int addInt) {
    return addOption(addName, (int) (getOptionInt(addName) + addInt), true, "", true);
  } // OptionsList::incOption

  const bool OptionsList::toggleOption(const string toggleName) {
    Option aOption;                           // option
    optionMapType::iterator ptr;              // pointer to a map

    mutex.Lock();

    // is there already an option by that name?
    ptr = optionList.find(toggleName);
    if (ptr == optionList.end()) {
      mutex.Unlock();
      return false;
    } // if

    // update that options
    if (ptr->second.getBoolean() == true) {
      ptr->second.setInteger(0);
      ptr->second.setBoolean(false);
      ptr->second.setString("false");
    } // if
    else {
      ptr->second.setInteger(1);
      ptr->second.setBoolean(true);
      ptr->second.setString("true");
    } // else

    mutex.Unlock();

    return true;
  } // OptionsList::toggleOption

  /**
   * OptionsList::removeOptions
   *
   * Remove options from the option handler.
   *
   * Returns: Number of options removed.
   */
  const unsigned int OptionsList::removeOptions(const string &removeMe) {
    list<string> optionList;		// List of options returned that matched.
    string optionName;			// Name of the looped to option.
    unsigned int numRemoved;		// Number of options removed.

    // initialize variables
    numRemoved = 0;

    if (matchOptions(removeMe, optionList) > 0) {
      for(; !optionList.empty(); optionList.pop_front()) {
        // initialize variables
        optionName = optionList.front();

        if (removeOption(optionName) == true)
          numRemoved++;
      } // for
    } // if

    return numRemoved;
  } // OptionsList::removeOptions
  
  /**
   * OptionsList::clearOptions
   *
   * Clears all options from the option handler.
   *
   * Returns: Number of options removed.
   */
  const int OptionsList::clearOptions() {
    unsigned int optionCount;         // number of options cleared

    mutex.Lock();

    optionCount = optionList.size();

    optionList.clear();

    mutex.Unlock();

    return optionCount;
  } // OptionsList::clearOptions

  /**
   * OptionsList::showOptionTree
   *
   * Returns a formatted list of all options matched in a tree format.
   *
   * Returns: Nothing.
   */
  void OptionsList::showOptionTree(const string treeMember, unsigned int treeLevel, 
                                   const unsigned int treeColor, string treeSpacer, 
                                   list<string> &treeList) {
    mutex.Lock();

    _showOptionTree(treeMember, treeLevel, 
                   treeColor, treeSpacer, 
                   treeList);

    mutex.Unlock();
  } // OptionsList::showOptionTree
  /**
   * OptionsList::showOptionTree
   *
   * Returns a formatted list of all options matched in a tree format.
   *
   * Returns: Nothing.
   */
  void OptionsList::_showOptionTree(const string treeMember, unsigned int treeLevel, 
                                   const unsigned int treeColor, string treeSpacer, 
                                   list<string> &treeList) {
    StringToken treeToken;                        // Tokenize a stringName.
    StringToken treeMemberToken;                  // Tokenise a treeMember name.
    optionMapType::iterator treePtr;              // iterator to a string map
    int numMatches;                               // number of matches
    int maxMatches;                               // max matches
    int intValue;				// Integer value.
    string stringName;                            // Name of the string.
    string stringLast;				// Last part of the name.
    string stringValue;                           // Value of the string.
    string lastName;                              // Last name matched.
    stringstream s;                               // stringstream
    unsigned int localColor;			// Local color.
    unsigned int i;				// Counter for lower case.

    treeToken.setDelimiter('.');
    treeMemberToken.setDelimiter('.');
    treeMemberToken = ((treeLevel == 0) ? string("root.") : string("")) + treeMember;

    localColor = treeColor;
    localColor++;
    if (localColor == 8)
      localColor = 2;

    if (treeLevel == 0) {
      s << treeSpacer << treeMemberToken.getTrailing(0) << " (0)";
      treeList.push_back(s.str());
      treeLevel = treeMemberToken.size();
    } // if
    // cout << "treeMember: " << treeMember << endl;
  
    // count matches
    maxMatches = 0;

    for(treePtr = optionBegin(); treePtr != optionEnd(); treePtr++) {
      // initialize variables
      treeToken = stringName = string("root.") + treePtr->first;

      if ((treeLevel + 1) > treeToken.size() ||
          treeLevel > treeMemberToken.size())
        continue;

      // cout << "[" << stringName << "] " << treeToken.getRange(0, treeLevel + 1) << " != " << lastName << endl;
      if (treeToken.getRange(0, treeLevel) == treeMemberToken.getRange(0, treeLevel) &&
          treeToken.getRange(0, (treeLevel + 1)) != lastName) {
        // cout << "[" << stringName << "] " << treeToken.getRange(0, treeLevel) << " == " << treeMember << endl;
        maxMatches++;
        lastName = treeToken.getRange(0, (treeLevel + 1));
      } // if
    } // for
  
    // initialize variables
    numMatches = 0;
    lastName = "";
  
    for(treePtr = optionBegin(); treePtr != optionEnd(); treePtr++) {  
      // initialize variables
      treeToken = stringName = string("root.") + treePtr->first;
  
      stringValue = treePtr->second.getString();
      intValue = treePtr->second.getInteger();
  
      if ((treeLevel + 1) > treeToken.size() ||
          treeLevel > treeMemberToken.size())
        continue;
  
      stringLast = treeToken[treeLevel];
  
      for(i = 0; i < stringLast.length(); i++) {
        if ((int) stringLast[i] >= 97 && (int) stringLast[i] <= 122)
        stringLast[i] = (char) ((int) stringLast[i] - (int) 32);
      } // for
  
      if (treeToken.getRange(0, treeLevel) == treeMemberToken.getRange(0, treeLevel) &&
          treeToken.getRange(0, (treeLevel + 1)) != lastName) {
        numMatches++;
        treeSpacer += (numMatches == maxMatches) ? " " : " |";
        s.str("");
        s << treeSpacer << ((numMatches == maxMatches) ? "`" : "") << "-"
          << stringLast;

        if ((treeLevel + 2) > treeToken.size()) {
          if (stringValue.length() == 0)
            s << " = " << intValue;
          else
            s << " = \"" << stringValue << "\"";
        } // if

        treeList.push_back(s.str());
        treeSpacer += (numMatches == maxMatches) ? " " : "";

        //if (maxMatches < 3) {
          _showOptionTree(treeToken.getRange(0, treeLevel), ++treeLevel, localColor, 
                         treeSpacer, treeList);
          treeLevel--;
        //} // if

        treeSpacer.replace((treeSpacer.length() - 2), 2, "");

        lastName = treeToken.getRange(0, (treeLevel + 1));
      } // if
    }  // for
  
    return;
  } // OptionsList::showOptionTree
  
  /**
   * OptionsList::matchOptions
   *
   * Assembles a list of options that match the given mask.
   *
   * Returns: Number of options matched.
   */
  const unsigned int OptionsList::matchOptions(const string &matchName,
                                               list<string> &matchList) {
    optionMapType::iterator optionPtr;		// Iterator to an Option map.
    unsigned int size;

    mutex.Lock();

    // initialize variables
    matchList.clear();

    for(optionPtr = optionBegin(); optionPtr != optionEnd(); optionPtr++) {
      if (match(matchName.c_str(), optionPtr->first.c_str()))
        matchList.push_back(optionPtr->first);
    } // for

    size = matchList.size();

    mutex.Unlock();

    return size;
  } // OptionsList::matchOptions

  /**
   * OptionsList::getOptionBool
   *
   * Finds an option and returns it's bool value.
   *
   * Returns: Bool value of the option.
   */
  const bool OptionsList::getOptionBool(const string &findMe) {
    optionMapType::iterator ptr;              // pointer to a map
    bool ret;

    mutex.Lock();

    if ((ptr = optionList.find(findMe)) == optionList.end()) {
      mutex.Unlock();
      return LIBHANDLER_OPTION_DEFAULT_BOOL;
    } // if

    ret = ptr->second.getBoolean();

    mutex.Unlock();

    return ret;
  } // OptionsList::getOptionBool

  /**
   * OptionsList::getOptionInt
   *
   * Finds an option and returns it's integer value.
   *
   * Returns: Integer value of the option.
   */
  const int OptionsList::getOptionInt(const string &findMe) {
    optionMapType::iterator ptr;              // pointer to a map
    int ret;

    mutex.Lock();

    if ((ptr = optionList.find(findMe)) == optionList.end()) {
      mutex.Unlock();
      return LIBHANDLER_OPTION_DEFAULT_INT;
    } // if

    ret = ptr->second.getInteger();

    mutex.Unlock();

    return ret;
  } // OptionsList::getOptionInt

  /**
   * OptionsList::getOptionInt
   *
   * Finds an option and returns it's string value.
   *
   * Returns: String value of the option.
   */
  const string OptionsList::getOptionString(const string &findMe) {
    optionMapType::iterator ptr;              // pointer to a map
    string ret;

    mutex.Lock();

    if ((ptr = optionList.find(findMe)) == optionList.end()) {
      mutex.Unlock();
      return LIBHANDLER_OPTION_DEFAULT_STRING;
    } // if

    ret = ptr->second.getString();

    mutex.Unlock();

    return ret;
  } // OptionsList::getOption

  /**
   * OptionsList::defaultOptionBool
   *
   * Attempts to look up an option, if failed returns the defaultValue.
   *
   * Returns: Options value if found or defaultValue if not.
   */
  const bool OptionsList::defaultOptionBool(const string &defaultName, 
                                            const bool defaultValue) {
    if (isOption(defaultName))
      return getOptionBool(defaultName);

    return defaultValue;
  } // OptionsList::defaultOptionBool

  /**
   * OptionsList::defaultOptionInt
   *
   * Attempts to look up an option, if failed returns the defaultValue.
   *
   * Returns: Options value if found or defaultValue if not.
   */  const int OptionsList::defaultOptionInt(const string &defaultName, 
                                          const int defaultValue) {
    if (isOption(defaultName))
      return getOptionInt(defaultName);

    return defaultValue;
  } // OptionsList::defaultOptionInt

  /**
   * OptionsList::defaultOptionString
   *
   * Attempts to look up an option, if failed returns the defaultValue.
   *
   * Returns: Options value if found or defaultValue if not.
   */
  const string OptionsList::defaultOptionString(const string &defaultName, 
                                                 const string &defaultValue) {
    if (isOption(defaultName))
      return getOptionString(defaultName);

    return defaultValue;
  } // OptionsList::defaultOptionString
} // namespace handler
