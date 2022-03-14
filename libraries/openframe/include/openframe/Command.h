#ifndef __OPENFRAME_COMMAND_H
#define __OPENFRAME_COMMAND_H

#include <map>
#include <string>
#include <list>

#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>

#include <openframe/openframe.h>

#include "Flag.h"

namespace openframe {
  using std::string;
  using std::map;
  using std::list;
  using std::pair;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define COMMAND_ARGUMENTS	Packet *ePacket
#define COMMAND_PROTOTYPE	Packet *

#define CMDERR_ACCESS  		0		// functions return 0 if user access too low
#define CMDERR_SUCCESS 		1		// successfully executed.
#define CMDERR_SYNTAX  		-1		// syntax error

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class CommandHandler;

class Packet {
  public:
    typedef map<string, void *> variableMapType;
    typedef map<string, string> stringMapType;

    Packet();
    ~Packet();

    /***********************
     ** Arguments Members **
     ***********************/

    StringToken getArguments() { return myArguments; }
    StringToken getArgs() { return myArguments; }
    StringToken args() { return myArguments; }
    void setArguments(const string setMe) { myArguments = setMe; }
    const string getPath() const { return myPath; }
    void setPath(const string &path) { myPath = path; }
    const string getSource() const { return mySource; }
    void setSource(const string &source) { mySource = source; }
    const string getName() const { return myName; }
    void setName(const string &name) { myName = name; }

    /********************
     ** String Members **
     ********************/

    const bool addString(const string &, const string &);
    const bool removeString(const string &);
    const bool isString(const string &);
    const string getString(const string &);

    void showStringTree(const string, unsigned int,
                        string, list<string> &);

    // iterators
    inline stringMapType::iterator stringBegin() { return myStringList.begin(); }
    inline stringMapType::iterator stringEnd() { return myStringList.end(); }
    const unsigned int stringSize() { return myStringList.size(); }
    const unsigned int clearStrings() {
      unsigned int numStrings = stringSize();

      myStringList.clear();

      return numStrings;
    } // clearStrings

    /**********************
     ** Variable Members **
     **********************/

    const bool addVariable(const string, void *);
    const bool removeVariable(const string);
    void *getVariable(const string);

    // iterators
    inline variableMapType::iterator variableBegin() { return myVariableList.begin(); }
    inline variableMapType::iterator variableEnd() { return myVariableList.end(); }
    const unsigned int variableSize();

  private:
    StringToken myArguments;		// arguments
    variableMapType myVariableList;	// list of variables
    stringMapType myStringList;		// String list.
    string myPath;			// path
    string mySource;			// source
    string myName;			// name

  protected:
}; // CommandPacket

class Command : public Flag {
  public:
    Command(const string &, const string &, const string &, const string &, const int, const unsigned int, CommandHandler *);
    virtual ~Command();

    /**********************
     ** Type Definitions **
     **********************/
    typedef unsigned int flagType;

    /***************
     ** Variables **
     ***************/

    static const flagType FLAG_SUPPRESS;
    static const flagType FLAG_CLEANUP;

    /*******************
     ** Level Members **
     *******************/

    const int getLevel() const
      { return myLevel; }

    /******************
     ** Name Members **
     ******************/

    const string getName() const
      { return myName; }

    /********************
     ** Syntax Members **
     ********************/

    const string getSyntax() const
      { return mySyntax; }

    /******************
     ** Path Members **
     ******************/

    const string getPath() const
      { return myPath; }

    /****************************
     ** CommandHandler Members **
     ****************************/

    // return function type
    CommandHandler *getCommandHandler()
      { return myCommandHandler; }

    /***********************
     ** Member Of Members **
     ***********************/

    const string getMember() const
      { return myMember; }

  protected:
    CommandHandler *myCommandHandler;		// command handler
    bool mySuppress;				// suppress the debug output of command
    string myName;				// command name
    string myPath;				// command path
    string mySyntax;				// command syntax
    string myMember;				// command I am a member of
    int myLevel;
};

class CommandHandler {
  public:
    CommandHandler() {}
    virtual ~CommandHandler() {}

    virtual const int Execute(COMMAND_PROTOTYPE) = 0;
  protected:
  private:
}; // CommandHandler

class CommandTree {
  public:
    CommandTree();
    virtual ~CommandTree();

    typedef map<string, Command *> commandMapType;

    /*********************
     ** Command Members **
     *********************/

    virtual const bool addCommand(const string &, const string &, const string &,
                                  const int, const unsigned int, CommandHandler *);
    virtual const unsigned int removeCommand(const string &);
    virtual void removeCommandTree(const string &, 
                                   unsigned int, list<string> &);

    virtual Command *findCommand(const string, const StringToken, unsigned int &, Command *);
    virtual const unsigned int findCommands(const string, const StringToken, unsigned int &, Command *, list<Command *> &);

    virtual void showTree(const string, const int, const string, list<string> &);

    const unsigned int commandSize() const
      { return myCommandList.size(); }
 
    // iterators!
    virtual inline commandMapType::iterator commandBegin()
      { return myCommandList.begin(); }

    virtual inline commandMapType::iterator commandEnd()
      { return myCommandList.end(); }

  private:
    commandMapType myCommandList;	 // recursive command list

  protected:
};

#define CREATE_COMMAND(name) \
class name : public CommandHandler { \
  public: \
    name() {} \
    virtual ~name() {} \
    const int Execute(COMMAND_PROTOTYPE); \
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

#define COMMAND_DEBUG_STRINGS(x, y, z) \
list<string> y; \
x->showStringTree(z, 1, "", y);

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openframe
#endif
