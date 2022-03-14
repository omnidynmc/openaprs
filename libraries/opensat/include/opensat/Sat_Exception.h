//
// cSite.h: interface for the cSite class.
//
// Copyright 2002-2003 Michael F. Henry
//
//////////////////////////////////////////////////////////////////////

#include <exception>

#pragma once

namespace opensat {

  class Sat_Exception : public std::exception {
    public:
      Sat_Exception(const std::string message) throw() {
        if (!message.length())
          _message = "An unknown message exception occured.";
        else
          _message = message;
      } // Sat_Exception

      virtual ~Sat_Exception() throw() { }
      virtual const char *what() const throw() { return _message.c_str(); }

      const char *message() const throw() { return _message.c_str(); }

    private:
      std::string _message;                    // Message of the exception error.
  }; // class Sat_Exception

  class SatUnitsNotKm_Exception : public Sat_Exception {
    private:
      typedef Sat_Exception super;

    public:
      SatUnitsNotKm_Exception() : super("units not km") { }
  }; // SatUnitsNotKm_Exception

  class SatTleNameEmpty_Exception : public Sat_Exception {
    private:
      typedef Sat_Exception super;

    public:
      SatTleNameEmpty_Exception() : super("tle name empty") { }
  }; // SatTleNameEmpty_Exception

  class SatTleLine1Empty_Exception : public Sat_Exception {
    private:
      typedef Sat_Exception super;

    public:
      SatTleLine1Empty_Exception() : super("tle line1 empty") { }
  }; // SatTleLine1Empty_Exception

  class SatTleLine2Empty_Exception : public Sat_Exception {
    private:
      typedef Sat_Exception super;

    public:
      SatTleLine2Empty_Exception() : super("tle line2 empty") { }
  }; // SatTleLine2Empty_Exception

} // namespace opensat
