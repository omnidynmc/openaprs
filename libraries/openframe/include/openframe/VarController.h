/**************************************************************************
 ** OpenFrame Library                                                    **
 **************************************************************************
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

#ifndef LIBOPENFRAME_VARCONTROLLER_H
#define LIBOPENFRAME_VARCONTROLLER_H

#include <string>
#include <list>
#include <map>
#include <cassert>
#include <vector>

#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#include "OpenFrame_Abstract.h"
#include "OFLock.h"
#include "noCaseCompare.h"
#include "Refcount.h"
#include "stringify.h"

namespace openframe {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class VarController_Exception : public OpenFrame_Exception {
  public:
    VarController_Exception(const std::string message) throw() : OpenFrame_Exception(message) {
    } // VarController_Exception

  private:
}; // class VarController_Exception

class VcUintDeincOutOfRange_Exception : public VarController_Exception {
  private:
    typedef VarController_Exception super;
  public:

  VcUintDeincOutOfRange_Exception(const std::string message) throw() : super(message) { }
  VcUintDeincOutOfRange_Exception() throw() : super("uint deinc out of range") { }
}; // class VcUintDeincOutOfRange_Exception

class VarController;
class Var : public std::vector<Var *> {
  public:
    enum varType {
      VARTYPE_BOOL,
      VARTYPE_DOUBLE,
      VARTYPE_INT,
      VARTYPE_UINT,
      VARTYPE_STRING,
      VARTYPE_VOID,
      VARTYPE_ARRAY,
      VARTYPE_OBJECT
    };

    Var(varType vtype) : _type(vtype) { }
    Var(const Var &n) : _var(n._var), _string(n._string), _type(n._type) { }
    virtual ~Var();

    friend class VarController;

    const varType type() const { return _type; }

    const double doubly() {
      assert(_type == VARTYPE_DOUBLE);	// bug
      return _var.v_double;
    } // doub

    void *voidy() {
      assert(_type == VARTYPE_VOID);	// bug
      return _var.v_void;
    } // voidy

    VarController *objecty() {
      assert(_type == VARTYPE_OBJECT);	// bug
      return _var.v_object;
    } // objecty

    const int integer() {
      assert(_type == VARTYPE_INT);	// bug
      return _var.v_int;
    } // integer

    const unsigned int uinteger() {
      assert(_type == VARTYPE_UINT);	// bug
      return _var.v_uint;
    } // uinteger

    const bool boolean() {
      assert(_type == VARTYPE_BOOL);	// bug
      return _var.v_bool;
    } // boolean

    const std::string str() {
      std::string ret;

      switch(_type) {
        case VARTYPE_BOOL:
          ret = stringify<bool>(_var.v_bool);
          break;
        case VARTYPE_INT:
          ret = stringify<int>(_var.v_int);
          break;
        case VARTYPE_UINT:
          ret = stringify<unsigned int>(_var.v_uint);
          break;
        case VARTYPE_DOUBLE:
          ret = stringify<double>(_var.v_double);
          break;
        case VARTYPE_STRING:
          ret = _string;
          break;
        case VARTYPE_VOID:
          ret = "(void *) " + stringify(_var.v_void);
          break;
        case VARTYPE_ARRAY:
          ret = display();
          break;
        default:
          assert(false);		// bug
          break;
      } // switch

      return ret;
    } // str

    const std::string display() {
      std::string ret;
      std::vector<Var *>::iterator ptr;

      switch(_type) {
        case VARTYPE_BOOL:
          ret = stringify<bool>(_var.v_bool);
          break;
        case VARTYPE_INT:
          ret = stringify<int>(_var.v_int);
          break;
        case VARTYPE_UINT:
          ret = stringify<unsigned int>(_var.v_uint);
          break;
        case VARTYPE_DOUBLE:
          ret = stringify<double>(_var.v_double);
          break;
        case VARTYPE_STRING:
          ret = "\"" + _string + "\"";
          break;
        case VARTYPE_VOID:
          ret = "(void *) " + stringify(_var.v_void);
          break;
        case VARTYPE_ARRAY:
          ret = "[ ";
          for(ptr = this->begin(); ptr != this->end(); ptr++) {
            if (ptr != this->begin())
              ret += ", ";
            ret += (*ptr)->display();
          } // for
          ret += " ]";
          break;
        case VARTYPE_OBJECT:
          ret = "(Object *) " + stringify(_var.v_object);
          break;
        default:
          assert(false);		// bug
          break;
      } // switch

      return ret;
    } // display

    const std::string display(const int sigdigits) {
      std::string ret;

      switch(_type) {
        case VARTYPE_BOOL:
          ret = stringify<bool>(_var.v_bool);
          break;
        case VARTYPE_INT:
          ret = stringify<int>(_var.v_int);
          break;
        case VARTYPE_UINT:
          ret = stringify<unsigned int>(_var.v_uint);
          break;
        case VARTYPE_DOUBLE:
          ret = stringify<double>(_var.v_double, sigdigits);
          break;
        case VARTYPE_STRING:
          ret = "\"" + _string + "\"";
          break;
        case VARTYPE_VOID:
          ret = "(void *) " + stringify(_var.v_void);
          break;
        case VARTYPE_ARRAY:
          ret = display();
          break;
        case VARTYPE_OBJECT:
          ret = display();
          break;
        default:
          assert(false);		// bug
          break;
      } // switch

      return ret;
    } // display

  protected:
    union {
      bool v_bool;
      double v_double;
      int v_int;
      unsigned int v_uint;
      void * v_void;
      VarController *v_object;
    } _var;
    std::string _string;
    varType _type;	// type of stored variable
};

class VarController : public OFLock, public Refcount {
  public:
    VarController();
    VarController(const VarController &);
    virtual ~VarController();

    static const string			ERR_ARRAY_BOUNDS;
    static const std::string 		ERR_NOT_ARRAY;
    static const std::string 		ERR_NOT_BOOL;
    static const std::string 		ERR_NOT_DOUBLE;
    static const std::string 		ERR_NOT_INT;
    static const std::string 		ERR_NOT_STRING;
    static const std::string 		ERR_NOT_UINT;
    static const std::string 		ERR_NOT_VOID;
    static const std::string 		ERR_NOT_OBJECT;

    // ### Typedefs ###
    typedef Var::size_type arraySizeType;
    typedef map<string, Var *, noCaseCompare> varMapType;
    typedef list<string> matchListType;

    typedef list<string> arrayStringList_t;
    typedef arrayStringList_t::iterator arrayStringList_itr;
    typedef arrayStringList_t::const_iterator arrayStringList_citr;
    typedef arrayStringList_t::size_type arrayStringList_s;

    // ### Members ###
    const arraySizeType array_size(const std::string &);
    const bool array_bool(const std::string &, const arraySizeType);
    const int array_int(const std::string &, const arraySizeType);
    const unsigned int array_uint(const std::string &, const arraySizeType);
    const std::string array_string(const std::string &, const arraySizeType);
    const arrayStringList_s array_string_list(const std::string &, arrayStringList_t &ret);
    const std::string array_string(const std::string &, const arraySizeType, const std::string &def);
    const double array_double(const std::string &, const arraySizeType);
    const double array_double(const std::string &, const arraySizeType, const double def);
    void *array_void(const std::string &, arraySizeType);
    VarController *array_object(const std::string &, arraySizeType);

    const bool push_bool(const std::string &, const bool);
    const bool push_int(const std::string &, const int);
    const bool push_uint(const std::string &, const unsigned int);
    const bool push_string(const std::string &, const std::string &);
    const bool push_double(const std::string &, const double);
    const bool push_void(const std::string &, void *);
    const bool push_object(const std::string &, VarController *);

    const bool is_array(const std::string &);
    const bool is_array_string(const std::string &name, const arraySizeType i);
    const bool is_bool(const std::string &);
    const bool is_int(const std::string &);
    const bool is_uint(const std::string &);
    const bool is_string(const std::string &);
    const bool is_double(const std::string &);
    const bool is_array_double(const std::string &name, const arraySizeType i);
    const bool is_void(const std::string &);
    const bool is_object(const std::string &);

    const bool replace_bool(const std::string &, const bool);
    const bool replace_int(const std::string &, const int);
    const bool replace_uint(const std::string &, const unsigned int);
    const bool replace_string(const std::string &, const std::string &);
    const bool replace_double(const std::string &, const double);
    const bool replace_void(const std::string &, void *);
    const bool replace_object(const std::string &, VarController *);

    const bool get_bool(const std::string &);
    const bool get_bool(const std::string &, const bool);
    const int get_int(const std::string &);
    const int get_int(const std::string &, const int);
    const unsigned int get_uint(const std::string &);
    const unsigned int get_uint(const std::string &, const unsigned int);
    const std::string get_string(const std::string &);
    const std::string get_string(const std::string &, const std::string &);
    const double get_double(const std::string &);
    const double get_double(const std::string &, const double);
    void *get_void(const std::string &);
    void *get_void(const std::string &, void *);
    VarController *get_object(const std::string &);

    void inc(const std::string &, const int inc=1);
    void deinc(const std::string &, const int deinc=1);
    const bool toggle(const std::string &);

    const bool pop(const std::string &);
    const unsigned int pop_match(const std::string &);

    const bool exists(const std::string &);

    const varMapType::size_type clear();
    const varMapType::size_type size();
    const unsigned int load(const std::string &);

    void tree(const std::string &, matchListType &);
    const matchListType::size_type match(const std::string &, matchListType &);

  protected:
    const bool _push_bool(const std::string &, const bool, const bool);
    const bool _push_int(const std::string &, const int, const bool);
    const bool _push_uint(const std::string &, const unsigned int, const bool);
    const bool _push_string(const std::string &, const std::string &, const bool);
    const bool _push_double(const std::string &, const double, const bool);
    const bool _push_void(const std::string &, void *, const bool);
    const bool _push_object(const std::string &, VarController *, const bool);
    const bool _pop(const std::string &);
    const unsigned int _pop_match(const std::string &);
    Var *_find(const std::string &);
    const bool _exists(const std::string &);
    const bool _is(const std::string &, Var::varType);
    const bool _array_is(const std::string &name, const arraySizeType i, Var::varType vtype);

    const arraySizeType _array_size(const std::string &);
    const bool _array_bool(const std::string &, const arraySizeType);
    const int _array_int(const std::string &, const arraySizeType);
    const unsigned int _array_uint(const std::string &, const arraySizeType);
    const std::string _array_string(const std::string &, const arraySizeType);
    const std::string _array_string(const std::string &, const arraySizeType, const std::string &def);
    const arrayStringList_s _array_string_list(const std::string &, arrayStringList_t &ret);
    const double _array_double(const std::string &, const arraySizeType);
    const double _array_double(const std::string &, const arraySizeType, const double def);
    void *_array_void(const std::string &, arraySizeType);
    VarController *_array_object(const std::string &, arraySizeType);

    const bool _get_bool(const std::string &);
    const bool _get_bool(const std::string &, const bool);
    const int _get_int(const std::string &);
    const int _get_int(const std::string &, const int);
    const unsigned int _get_uint(const std::string &);
    const unsigned int _get_uint(const std::string &, const unsigned int);
    const std::string _get_string(const std::string &);
    const std::string _get_string(const std::string &, const std::string &);
    const double _get_double(const std::string &);
    const double _get_double(const std::string &, const double);
    void *_get_void(const std::string &);
    void *_get_void(const std::string &, void *);
    VarController *_get_object(const std::string &);

    void _inc(const std::string &, const int inc=1);
    void _deinc(const std::string &, const int deinc=1);
    const bool _toggle(const std::string &);
    virtual const unsigned int _load(const std::string &);
    void _valParse(const unsigned int, const std::string &, const std::string &);

    void _tree(const std::string &, unsigned int, const unsigned int, string, list<string> &);
    const matchListType::size_type _match(const std::string &, matchListType &);

  private:
    // ### Members ###
    const bool _push(const std::string &, Var *, const bool);

    // ### Variables ###
    varMapType _varMap;		// variable map
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openframe
#endif
