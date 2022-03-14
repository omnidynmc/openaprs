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
 **************************************************************************/

#include <list>
#include <fstream>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <new>
#include <iostream>
#include <string>
#include <exception>
#include <sstream>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "Command.h"

#include "openframe/StringToken.h"

#include "match.h"
#include "openaprs_string.h"

namespace openaprs {
  using openframe::StringToken;
  using namespace std;

/**************************************************************************
 ** Command Class                                                        **
 **************************************************************************/
  
  /******************************
   ** Constructor / Destructor **
   ******************************/
  
  Command::Command(const string &addName, const string &addPath, const string &addMember,
                   const string &addSyntax, const int addLevel, const unsigned int addFlags, 
                   CommandHandler *addCommandHandler) {
    myName = addName;
    myPath = addPath;
    myMember = addMember;
    mySyntax = addSyntax;
    myLevel = addLevel;
    myCommandHandler = addCommandHandler;
    addFlag(addFlags);
  
    return;
  } // Command::Command
  
  Command::~Command() {
    return;
  } // Command::~Command
  
  /***************
   ** Variables **
   ***************/
  
  const Command::flagType Command::FLAG_SUPPRESS		=	0x0001;
  const Command::flagType Command::FLAG_CLEANUP		=	0x0002;
  
/**************************************************************************
 ** CommandTree Class                                                    **
 **************************************************************************/

  CommandTree::CommandTree() {
    return;
  } // CommandTree::CommandTree

  CommandTree::~CommandTree() {
    commandMapType::iterator ptr;		// pointer to a map

    /*****************************
     ** Cleanup Command Members **
     *****************************/
    for(ptr = commandBegin(); ptr != commandEnd(); ptr++)
      delete ptr->second;

    myCommandList.clear();

    return;
  } // CommandTree::~CommandTree

  /**
   * CommandTree::addCommand
   *
   * Adds a command to the command handler.
   *
   * Returns: True on success, false on failure.
   */
  const bool CommandTree::addCommand(const string &addName, const string &addMember, 
                                     const string &addSyntax, const int addLevel,
                                     const unsigned int addFlags,
                                     CommandHandler *addCommandHandler) {
    Command *aCommand;				// new command to add

    assert(addCommandHandler != NULL);

    // create a new command
    try {
      aCommand = new Command(addName, addMember + string("/") + addName, 
                             addMember, addSyntax, addLevel, addFlags, addCommandHandler);
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    myCommandList.insert(pair<string, Command *>(aCommand->getPath(), aCommand));

    return true;
  } // CommandTree::addCommand

  /**
   * CommandTree::removeCommand
   *
   * Removes a command tree from the command handler.
   *
   * Returns: Number of commands removed.
   */
  const unsigned int CommandTree::removeCommand(const string &removePath) {
    Command *tCommand;				// Pointer to a target Command.
    CommandHandler *tCommandHandler;		// Pointer to a target CommandHandler.
    commandMapType::iterator commandPtr;		// Iterator to a Command map.
    list<string> returnList;			// List returned from removeCommandTree.
    unsigned int numRemoved;			// Number of commands removed.

    removeCommandTree(removePath, 0, returnList);

    // initialize variables
    numRemoved = 0;

    for(; !returnList.empty(); returnList.pop_front()) {
      if ((commandPtr = myCommandList.find(returnList.front())) == myCommandList.end())
        continue;

      // initialize variables
      tCommand = commandPtr->second;
      tCommandHandler = tCommand->getCommandHandler();

  // cout << "Remove: " << tCommand->getPath() << endl;

      // cleanup
      if (tCommand->findFlag(Command::FLAG_CLEANUP) == true)
        delete tCommandHandler;
      delete tCommand;

      myCommandList.erase(commandPtr);
      numRemoved++;
    } // while

    return numRemoved;
  } // CommandTree::removeCommand

  /**
   * CommandTree::removeCommandTree
   *
   * Assembles a tree of commands based on the treeMember given.
   *
   * Returns: Nothing.
   */
  void CommandTree::removeCommandTree(const string &treeMember, 
                                      unsigned int treeLevel, 
                                      list<string> &treeList) {
    Command *lCommand;					// pointer to a command we've looped to
    StringToken treeToken;				// Tokenize the tree path.
    StringToken treeMemberToken;				// Tokenize the tree member path.
    commandMapType::iterator treePtr;			// pointer to a list
    string lastName;					// Last name spanned.
  
    // initialize variables
    treeToken.setDelimiter('/');
    treeMemberToken.setDelimiter('/');
    treeMemberToken = treeMember;
    if (treeLevel == 0) {
      treeList.push_back(treeMember);
      treeLevel = treeMemberToken.size();
    } // if
  
    for(treePtr = commandBegin(); treePtr != commandEnd(); treePtr++) {
      // initialize variables
      lCommand = treePtr->second;
      treeToken = lCommand->getPath();
  
      if ((treeLevel + 1) > treeToken.size() ||
          treeLevel > treeMemberToken.size())
        continue;
        
      if (treeToken.getRange(0, treeLevel) == treeMemberToken.getRange(0, treeLevel) &&
          treeToken.getRange(0, treeLevel + 1) != lastName) {
  // cout << treeToken.getRange(0, treeLevel) << " == " << treeMemberToken.getRange(0, treeLevel) << endl;
  // cout << "Remove: " << lCommand->getPath() << endl;
        treeList.push_back(lCommand->getPath());
        removeCommandTree(treeToken.getRange(0, treeLevel), ++treeLevel, 
                          treeList);
        treeLevel--;
        lastName = treeToken.getRange(0, treeLevel + 1);
      } // if
    } // for
  
    return;
  } // CommandTree::showTree
  
  /**
   * CommandTree::findCommand
   *
   * Attempt to find a commands in the command tree.
   *
   * Returns: Pointer to Command or NULL on failure.
   */
  Command *CommandTree::findCommand(const string commandMember, const StringToken aToken, unsigned int &commandPosition, Command *currentCommand) {
    Command *lCommand;					// pointer to a Command we've looped to
    commandMapType::iterator commandPtr;			// pointer to a map
  
    if (commandPosition == aToken.size())
      return currentCommand;
  
    for(commandPtr = commandBegin(); commandPtr != commandEnd(); commandPtr++) {
      // initialize variables
      lCommand = commandPtr->second;
  
      if (!cmp(lCommand->getMember().c_str(), commandMember.c_str()) &&
          !cmp(aToken[commandPosition].c_str(), lCommand->getName().c_str()))
        return findCommand(lCommand->getPath(), aToken, ++commandPosition, lCommand);
    }  // for
  
    return currentCommand;
  } // CommandTree::findCommand
  
  /**
   * CommandTree::findCommands
   *
   * Attempt to find a list of commands in the command tree.
   *
   * Returns: Pointer to Command or NULL on failure.
   */
  const unsigned int CommandTree::findCommands(const string commandMember, const StringToken aToken, unsigned int &commandPosition, Command *currentCommand, list<Command *> &findList) {
    Command *lCommand;					// pointer to a command we've looped to
    commandMapType::iterator commandPtr;			// pointer to a map
  
    if (commandPosition == aToken.size())
      return findList.size();
  
    for(commandPtr = commandBegin(); commandPtr != commandEnd(); commandPtr++) {
      // initialize variables
      lCommand = commandPtr->second;
  
      if (!cmp(lCommand->getMember().c_str(), commandMember.c_str()) &&
          match(aToken[commandPosition].c_str(), lCommand->getName().c_str())) {
        findList.push_back(lCommand);
        findCommands(lCommand->getPath(), aToken, ++commandPosition, lCommand, findList);
      }
    }  // for
  
    return findList.size();
  } // CommandTree::findCommands
  
  /**
   * CommandTree::showTree
   *
   * Attempts to create a displayable command tree.
   *
   * Returns: Nothing.
   */
  void CommandTree::showTree(const string treeMember, int treeLevel, string treeSpacer,  list<string> &treeList) {
    Command *lCommand;					// pointer to a command we've looped to
    commandMapType::iterator treePtr;			// pointer to a list
    int numMatches;					// number of matches
    int maxMatches;					// max matches
    stringstream s;					// stringstream
  
    if (treeLevel == 1) {
      s << treeSpacer << treeMember << " (0)";
      treeList.push_back(s.str());
    } // if
  
    // count matches
    maxMatches = 0;
    for(treePtr = commandBegin(); treePtr != commandEnd(); treePtr++) {
      // initialize variables
      lCommand = treePtr->second;
  
      if (!cmp(lCommand->getMember().c_str(), treeMember.c_str()))
        maxMatches++;
    } // for
  
    numMatches = 0;
    for(treePtr = commandBegin(); treePtr != commandEnd(); treePtr++) {
      // initialize variables
      lCommand = treePtr->second;
  
      if (!cmp(lCommand->getMember().c_str(), treeMember.c_str())) {
        numMatches++;
        treeSpacer += (numMatches == maxMatches) ? " " : " |";
        s.str("");
        s << treeSpacer << ((numMatches == maxMatches) ? "`" : "") << "-" 
          << lCommand->getName() << " (" << treeLevel << ") " << lCommand->getLevel();
        treeList.push_back(s.str());
  //      send_to_server(PRIORITY_QUEUE_MEDIUM, "%s %s :%s%s-\003%.2d%s (%d) %d\003\n", IRC_PRIVATE, "OmniDynmc", commandSpacer.c_str(), (numMatches == maxMatches) ? "`" : "", localColor, ptr->second->getName().c_str(), commandLevel, ptr->second->getLevel());
        treeSpacer += (numMatches == maxMatches) ? " " : "";
        showTree(lCommand->getPath(), ++treeLevel, treeSpacer, treeList);
        treeSpacer.replace((treeSpacer.length() - 2), 2, "");
        treeLevel--;
      } // if
    }  // for

    return;
  } // CommandTree::showTree

/**************************************************************************
 ** Packet Class                                                         **
 **************************************************************************/

  Packet::Packet() {
  } // Packet::Packet

  Packet::~Packet() {
  } // Packet::~Packet

  const bool Packet::addString(const string &stringName, const string &addMe) {
    stringMapType::iterator ptr;                  // pointer to a string list

    if ((ptr = myStringList.find(stringName)) != myStringList.end()) {
      ptr->second = addMe;
      return true;
    } // if

    myStringList.insert(pair<string, string>(stringName, addMe));

    return true;
  } // CommandPacket::addString

  const bool Packet::removeString(const string &stringName) {
    stringMapType::iterator ptr;                  // pointer to a string list
    if ((ptr = myStringList.find(stringName)) == myStringList.end())
      return false;
  
    myStringList.erase(ptr);
  
    return true;
  } // Packet::removeString
  
  const string Packet::getString(const string &stringName) {
    stringMapType::iterator ptr;
  
    if ((ptr = myStringList.find(stringName)) == myStringList.end())
      return "";
  
    return ptr->second;
  } // Packet::getString

  const bool Packet::isString(const string &stringName) {
    stringMapType::iterator ptr;
  
    if ((ptr = myStringList.find(stringName)) == myStringList.end())
      return false;
  
    return true;
  } // Packet::isString
  
  void Packet::showStringTree(const string treeMember, unsigned int treeLevel, 
                              string treeSpacer, list<string> &treeList) {
    StringToken treeToken;                        // Tokenize a stringName.
    StringToken treeMemberToken;                  // Tokenise a treeMember name.
    stringMapType::iterator treePtr;              // iterator to a string map
    int numMatches;                               // number of matches
    int maxMatches;                               // max matches
    string stringName;                            // Name of the string.
    string stringValue;                           // Value of the string.
    string lastName;                              // Last name matched.
    stringstream s;                               // stringstream
  
    // initialize variables
    treeToken.setDelimiter('.');
    treeMemberToken.setDelimiter('.');
    treeMemberToken = treeMember;

    if (treeLevel == 1) {
      s << treeSpacer << treeMember << " (0)";
      treeList.push_back(s.str());
      // cout << endl;
    } // if
    // cout << "treeMember: " << treeMember << endl;
  
    // count matches
    maxMatches = 0;
    treeLevel = treeMemberToken.size();

    for(treePtr = stringBegin(); treePtr != stringEnd(); treePtr++) {
      // initialize variables
      treeToken = stringName = string("root.") + treePtr->first;
      if ((treeLevel + 1) > treeToken.size() ||
          treeLevel > treeMemberToken.size())
        continue;
  
      // cout << "[" << stringName << "] " << treeToken.getRange(0, (treeLevel + 1)) << " != " << lastName << endl;
      if (treeToken.getRange(0, treeLevel)  == treeMemberToken.getRange(0, treeLevel) &&
          treeToken.getRange(0, (treeLevel + 1)) != lastName) {
        // cout << "[" << stringName << "] " << treeToken.getRange(0, treeLevel) << " == " << treeMember << endl;
        maxMatches++;
        lastName = treeToken.getRange(0, (treeLevel + 1));
      } // if
    } // for
  
    // initialize variables
    numMatches = 0;
    lastName = "";
  
    for(treePtr = stringBegin(); treePtr != stringEnd(); treePtr++) {  
      // initialize variables
      treeToken = stringName = string("root.") + treePtr->first;
      stringValue = treePtr->second;
  
      if ((treeLevel + 1) > treeToken.size() ||
          treeLevel > treeMemberToken.size())
        continue;
  
      if (treeToken.getRange(0, treeLevel) == treeMemberToken.getRange(0, treeLevel) &&
          treeToken.getRange(0, (treeLevel + 1)) != lastName) {
        numMatches++;
        treeSpacer += (numMatches == maxMatches) ? " " : " |";
        s.str("");
        s << treeSpacer << ((numMatches == maxMatches) ? "`" : "") << "-"
          << treeToken[treeLevel];
        if ((treeLevel + 2) > treeToken.size())
          s << " = \"" << stringValue << "\"";
        treeList.push_back(s.str());
        treeSpacer += (numMatches == maxMatches) ? " " : "";
        showStringTree(treeToken.getRange(0, treeLevel), ++treeLevel, treeSpacer, treeList);
        treeSpacer.replace((treeSpacer.length() - 2), 2, "");
        treeLevel--;
        lastName = treeToken.getRange(0, (treeLevel + 1));
      } // if
    }  // for
  
    return;
  } // Packet::showStringTree
  
  /**********************
   ** Variable Members **
   **********************/
  
  const bool Packet::addVariable(const string variableName, void *variablePointer) {
    variableMapType::iterator ptr;			// pointer to a variable list
  
    if ((ptr = myVariableList.find(variableName)) != myVariableList.end())
      return false;
  
    myVariableList.insert(pair<string, void *>(variableName, variablePointer));
  
    return true;
  } // CommandPacket::addVariable
  
  const bool Packet::removeVariable(const string variableName) {
    variableMapType::iterator ptr;			// pointer to a variable list
  
    if ((ptr = myVariableList.find(variableName)) == myVariableList.end())
      return false;
  
    myVariableList.erase(ptr);
  
    return true;
  } // Packet::removeVariable
  
  void *Packet::getVariable(const string variableName) {
    variableMapType::iterator ptr;
  
    if ((ptr = myVariableList.find(variableName)) == myVariableList.end())
      return NULL;
  
    return ptr->second;
  } // Packet::getVariable
} // namespace handler
