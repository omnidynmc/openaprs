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

#include <openframe/config.h>

#include <cstdlib>
#include <cstring>
#include <list>
#include <string>
#include <sstream>
#include <iostream>

#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <openframe/LineBuffer.h>
#include <openframe/StringToken.h>
#include <openframe/StringTool.h>
#include <openframe/VarController.h>
#include <openframe/scoped_lock.h>

namespace openframe {
/**************************************************************************
 ** VarController Class                                                  **
 **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  Var::~Var() {
    // free up array
    std::vector<Var *>::iterator ptr;
    for(ptr = this->begin(); ptr != this->end(); ptr++) delete *ptr;
    if (_type == VARTYPE_OBJECT) _var.v_object->release();

  } // ~Var::Var


  const std::string VarController::ERR_ARRAY_BOUNDS		= "array element position out of bounds";
  const std::string VarController::ERR_NOT_ARRAY		= "var not an array";
  const std::string VarController::ERR_NOT_BOOL		= "var not a bool";
  const std::string VarController::ERR_NOT_DOUBLE		= "var not a double";
  const std::string VarController::ERR_NOT_INT		= "var not an int";
  const std::string VarController::ERR_NOT_STRING		= "var not a string";
  const std::string VarController::ERR_NOT_UINT		= "var not an unsigned int";
  const std::string VarController::ERR_NOT_VOID		= "var not a void";
  const std::string VarController::ERR_NOT_OBJECT		= "var not an object";

  // ### Public Members ###
  VarController::VarController() {
  } // VarController::VarController

  VarController::VarController(const VarController &n) {
    Var *v;
    varMapType::const_iterator ptr;

    for(ptr = n._varMap.begin(); ptr != n._varMap.end(); ptr++) {
      v = new Var(*ptr->second);
      _varMap.insert( make_pair(ptr->first, v) );
    } // for
  } // VarController::VarController

  VarController::~VarController() {
    Lock();
    _pop_match("*");
    Unlock();
  } // VarController::~VarController

  void VarController::tree(const std::string &match, matchListType &matchList) {
    Lock();
    _tree(match, 0, 1, "", matchList);
    Unlock();
  } // VarController::tree

  const VarController::matchListType::size_type VarController::match(const std::string &match, matchListType &matchList) {
    matchListType::size_type ret;

    Lock();
    ret = _match(match, matchList);
    Unlock();

    return ret;
  } // VarController::tree

  const bool VarController::exists(const std::string &name) {
    bool ret;

    Lock();
    ret = _exists(name);
    Unlock();

    return ret;
  } // VarController::exists

  const bool VarController::is_int(const std::string &name) {
    bool ret;

    Lock();
    ret = _is(name, Var::VARTYPE_INT);
    Unlock();

    return ret;
  } // VarController::is_int

  const bool VarController::is_uint(const std::string &name) {
    bool ret;

    Lock();
    ret = _is(name, Var::VARTYPE_UINT);
    Unlock();

    return ret;
  } // VarController::is_uint

  const bool VarController::is_double(const std::string &name) {
    bool ret;

    Lock();
    ret = _is(name, Var::VARTYPE_DOUBLE);
    Unlock();

    return ret;
  } // VarController::is_double

  const bool VarController::is_array_double(const std::string &name, const arraySizeType i) {
    bool ret;

    Lock();
    ret = _array_is(name, i, Var::VARTYPE_DOUBLE);
    Unlock();

    return ret;
  } // VarController::is_array_double

  const bool VarController::is_void(const std::string &name) {
    bool ret;

    Lock();
    ret = _is(name, Var::VARTYPE_VOID);
    Unlock();

    return ret;
  } // VarController::is_void

  const bool VarController::is_object(const std::string &name) {
    bool ret;

    Lock();
    ret = _is(name, Var::VARTYPE_OBJECT);
    Unlock();

    return ret;
  } // VarController::is_object

  const bool VarController::is_array(const std::string &name) {
    bool ret;

    Lock();
    ret = _is(name, Var::VARTYPE_ARRAY);
    Unlock();

    return ret;
  } // VarController::is_array

  const bool VarController::is_string(const std::string &name) {
    bool ret;

    Lock();
    ret = _is(name, Var::VARTYPE_STRING);
    Unlock();

    return ret;
  } // VarController::is_string

  const bool VarController::is_bool(const std::string &name) {
    bool ret;

    Lock();
    ret = _is(name, Var::VARTYPE_BOOL);
    Unlock();

    return ret;
  } // VarController::is_bool

  // Push Bool
  const bool VarController::push_bool(const std::string &name, const bool value) {
    bool ret;

    Lock();
    ret = _push_bool(name, value, false);
    Unlock();

    return ret;
  } // VarController::push_bool

  const bool VarController::replace_bool(const std::string &name, const bool value) {
    bool ret;

    Lock();
    ret = _push_bool(name, value, true);
    Unlock();

    return ret;
  } // VarController::replace_bool

  // Push Int
  const bool VarController::push_int(const std::string &name, const int value) {
    bool ret;

    Lock();
    ret = _push_int(name, value, false);
    Unlock();

    return ret;
  } // VarController::push_int

  const bool VarController::replace_int(const std::string &name, const int value) {
    bool ret;

    Lock();
    ret = _push_int(name, value, true);
    Unlock();

    return ret;
  } // VarController::replace_int

  // Push Unsigned Int
  const bool VarController::push_uint(const std::string &name, const unsigned int value) {
    bool ret;

    Lock();
    ret = _push_uint(name, value, false);
    Unlock();

    return ret;
  } // VarController::push_uint

  const bool VarController::replace_uint(const std::string &name, const unsigned int value) {
    bool ret;

    Lock();
    ret = _push_uint(name, value, true);
    Unlock();

    return ret;
  } // VarController::replace_uint

  // Push Double
  const bool VarController::push_double(const std::string &name, const double value) {
    bool ret;

    Lock();
    ret = _push_double(name, value, false);
    Unlock();

    return ret;
  } // VarController::push_double

  // Push Double
  const bool VarController::push_void(const std::string &name, void *value) {
    bool ret;

    Lock();
    ret = _push_void(name, value, false);
    Unlock();

    return ret;
  } // VarController::push_void

  const bool VarController::push_object(const std::string &name, VarController *value) {
    bool ret;

    Lock();
    ret = _push_object(name, value, false);
    Unlock();

    return ret;
  } // VarController::push_object

  const bool VarController::replace_double(const std::string &name, const double value) {
    bool ret;

    Lock();
    ret = _push_double(name, value, true);
    Unlock();

    return ret;
  } // VarController::replace_double

  const bool VarController::replace_void(const std::string &name, void *value) {
    bool ret;

    Lock();
    ret = _push_void(name, value, true);
    Unlock();

    return ret;
  } // VarController::replace_void

  const bool VarController::replace_object(const std::string &name, VarController *value) {
    bool ret;

    Lock();
    ret = _push_object(name, value, true);
    Unlock();

    return ret;
  } // VarController::replace_object

  // Push String
  const bool VarController::push_string(const std::string &name, const std::string &value) {
    bool ret;

    Lock();
    ret = _push_string(name, value, false);
    Unlock();

    return ret;
  } // VarController::push_string

  const bool VarController::replace_string(const std::string &name, const std::string &value) {
    bool ret;

    Lock();
    ret = _push_string(name, value, true);
    Unlock();

    return ret;
  } // VarController::replace_string

  const bool VarController::pop(const std::string &name) {
    bool ret;

    Lock();
    ret = _pop(name);
    Unlock();

    return ret;
  } // VarController::pop

  const unsigned int VarController::pop_match(const std::string &name) {
    unsigned int ret;

    Lock();
    ret = _pop_match(name);
    Unlock();

    return ret;
  } // VarController::pop_match

  void VarController::inc(const std::string &name, const int increment) {
    //int ret;

    Lock();
    _inc(name, increment);
    Unlock();

    //return ret;
  } // VarController::inc

  void VarController::deinc(const std::string &name, const int increment) {
    //int ret;

    Lock();
    _deinc(name, increment);
    Unlock();

    //return ret;
  } // VarController::inc

  const bool VarController::toggle(const std::string &name) {
    bool ret;

    Lock();
    ret = _toggle(name);
    Unlock();

    return ret;
  } // VarController::toggle

  // Public std::string Members
  const VarController::arraySizeType VarController::array_size(const std::string &name) {
    arraySizeType ret;

    Lock();
    ret = _array_size(name);
    Unlock();

    return ret;
  } // VarController::array_size

  // Public Bool Members
  const bool VarController::array_bool(const std::string &name, const arraySizeType i) {
    bool ret;

    Lock();
    ret = _array_bool(name, i);
    Unlock();

    return ret;
  } // VarController::array_bool

  const bool VarController::get_bool(const std::string &name) {
    bool ret;

    Lock();
    ret = _get_bool(name);
    Unlock();

    return ret;
  } // VarController::get_bool

  const bool VarController::get_bool(const std::string &name, const bool def) {
    bool ret;

    Lock();
    ret = _get_bool(name, def);
    Unlock();

    return ret;
  } // VarController::get_bool

  // Public Int Members
  const int VarController::array_int(const std::string &name, const arraySizeType i) {
    int ret;

    Lock();
    ret = _array_int(name, i);
    Unlock();

    return ret;
  } // VarController::array_int

  const int VarController::get_int(const std::string &name) {
    int ret;

    Lock();
    ret = _get_int(name);
    Unlock();

    return ret;
  } // VarController::get_int

  const int VarController::get_int(const std::string &name, const int def) {
    int ret;

    Lock();
    ret = _get_int(name, def);
    Unlock();

    return ret;
  } // VarController::get_int

  // Public Double Members
  const double VarController::array_double(const std::string &name, const arraySizeType i) {
    double ret;

    Lock();
    ret = _array_double(name, i);
    Unlock();

    return ret;
  } // VarController::array_double

  const double VarController::array_double(const std::string &name, const arraySizeType i, const double def) {
    double ret;

    Lock();
    ret = _array_double(name, i, def);
    Unlock();

    return ret;
  } // VarController::array_double

  const double VarController::get_double(const std::string &name) {
    double ret;

    Lock();
    ret = _get_double(name);
    Unlock();

    return ret;
  } // VarController::get_double

  const double VarController::get_double(const std::string &name, const double def) {
    double ret;

    Lock();
    ret = _get_double(name, def);
    Unlock();

    return ret;
  } // VarController::get_double

  // Public Void Members
  void *VarController::array_void(const std::string &name, const arraySizeType i) {
    void *ret;

    Lock();
    ret = _array_void(name, i);
    Unlock();

    return ret;
  } // VarController::array_void

  VarController *VarController::array_object(const std::string &name, const arraySizeType i) {
    VarController *ret;

    Lock();
    ret = _array_object(name, i);
    Unlock();

    return ret;
  } // VarController::array_object

  void *VarController::get_void(const std::string &name) {
    void *ret;

    Lock();
    ret = _get_void(name);
    Unlock();

    return ret;
  } // VarController::get_void

  void *VarController::get_void(const std::string &name, void *def) {
    void *ret;

    Lock();
    ret = _get_void(name, def);
    Unlock();

    return ret;
  } // VarController::get_void

  VarController *VarController::get_object(const std::string &name) {
    VarController *ret;

    Lock();
    ret = _get_object(name);
    Unlock();

    return ret;
  } // VarController::get_object

  // Public Unsigned Int Members
  const unsigned int VarController::array_uint(const std::string &name, const arraySizeType i) {
    unsigned int ret;

    Lock();
    ret = _array_uint(name, i);
    Unlock();

    return ret;
  } // VarController::array_uint

  const unsigned int VarController::get_uint(const std::string &name) {
    unsigned int ret;

    Lock();
    ret = _get_uint(name);
    Unlock();

    return ret;
  } // VarController::get_uint

  const unsigned int VarController::get_uint(const std::string &name, const unsigned int def) {
    unsigned int ret;

    Lock();
    ret = _get_uint(name, def);
    Unlock();

    return ret;
  } // VarController::get_uint

  // Public std::string Members
  const std::string VarController::array_string(const std::string &name, const arraySizeType i) {
    std::string ret;

    Lock();
    ret = _array_string(name, i);
    Unlock();

    return ret;
  } // VarController::array_string

  const VarController::arrayStringList_s VarController::array_string_list(const std::string &name, arrayStringList_t &ret) {
    scoped_lock slock(this);
    return _array_string_list(name, ret);
  } // VarController::array_string_list

  const std::string VarController::array_string(const std::string &name, const arraySizeType i, const std::string &def) {
    std::string ret;

    Lock();
    ret = _array_string(name, i, def);
    Unlock();

    return ret;
  } // VarController::array_string

  const bool VarController::is_array_string(const std::string &name, const arraySizeType i) {
    bool ret;

    Lock();
    ret = _array_is(name, i, Var::VARTYPE_STRING);
    Unlock();

    return ret;
  } // VarController::is_array_string

  // Public std::string Members
  const std::string VarController::get_string(const std::string &name) {
    std::string ret;

    Lock();
    ret = _get_string(name);
    Unlock();

    return ret;
  } // VarController::get_string

  const std::string VarController::get_string(const std::string &name, const std::string &def) {
    std::string ret;

    Lock();
    ret = _get_string(name, def);
    Unlock();

    return ret;
  } // VarController::get_string

  const VarController::varMapType::size_type VarController::clear() {
    varMapType::size_type num;
    varMapType::iterator ptr;

    Lock();

    num = _varMap.size();

    while(!_varMap.empty()) {
      ptr = _varMap.begin();
      delete ptr->second;
      _varMap.erase(ptr);
    } // while

    Unlock();

    return num;
  } // VarController::clear

  const VarController::varMapType::size_type VarController::size() {
    varMapType::size_type num;

    Lock();

    num = _varMap.size();

    Unlock();

    return num;
  } // VarController::size

  const unsigned int VarController::load(const std::string &filename) {
    unsigned int ret;

    Lock();
    ret = _load(filename);
    Unlock();

    return ret;
  } // VarController::load

  // ### Protected Members ###
  const bool VarController::_exists(const std::string &name) {
    varMapType::iterator ptr;

    ptr = _varMap.find(name);
    if (ptr == _varMap.end())
      return false;

    return true;
  } // VarController::_exists

  const bool VarController::_is(const std::string &name, Var::varType vtype) {
    varMapType::iterator ptr;

    ptr = _varMap.find(name);
    if (ptr == _varMap.end())
      return false;

    return (ptr->second->_type == vtype);
  } // VarController::_is

  const bool VarController::_array_is(const std::string &name, const arraySizeType i, Var::varType vtype) {
    Var *var;

    var = _find(name);
    if (var == NULL)
      return false;

    if ( !(i < var->size()) )
      return var->type() == vtype;

    return (*var)[i]->type() == vtype;
  } // VarController::_array_is

  Var *VarController::_find(const std::string &name) {
    varMapType::iterator ptr;

    ptr = _varMap.find(name);
    if (ptr == _varMap.end())
      return NULL;

    return ptr->second;
  } // VarController::_find

  // Double Members
  const double VarController::_array_double(const std::string &name, const arraySizeType i) {
    Var *var;

    var = _find(name);
    if (var == NULL) {
      Unlock();
      throw VarController_Exception(ERR_NOT_DOUBLE + "; " + name);
    } // if

    if ( !(i < var->size()) )
      return var->doubly();

    return (*var)[i]->doubly();
  } // VarController::_array_double

  const double VarController::_array_double(const std::string &name, const arraySizeType i, const double def) {
    Var *var;

    var = _find(name);
    if (var == NULL)
      return def;

    if ( !(i < var->size()) )
      return var->doubly();

    return (*var)[i]->doubly();
  } // VarController::_array_double

  const double VarController::_get_double(const std::string &name) {
    Var *var;

    var = _find(name);
    if (var == NULL || var->_type != Var::VARTYPE_DOUBLE) {
      Unlock();
      throw VarController_Exception(ERR_NOT_DOUBLE + "; " + name);
    } // if

    return var->doubly();
  } // VarController::_get_double

  const double VarController::_get_double(const std::string &name, double def) {
    Var *var;

    var = _find(name);
    if (var == NULL || var->_type != Var::VARTYPE_DOUBLE)
      return def;

    return var->doubly();
  } // VarController::_get_double

  void *VarController::_array_void(const std::string &name, const arraySizeType i) {
    Var *var;

    var = _find(name);
    if (var == NULL) {
      Unlock();
      throw VarController_Exception(ERR_NOT_INT + "; " + name);
    } // if

    if ( !(i < var->size()) ) {
      Unlock();
      throw VarController_Exception(ERR_ARRAY_BOUNDS + "; "+name+"["+stringify<size_t>(i)+"]");
    } // if

    return (*var)[i]->voidy();
  } // VarController::_array_void

  VarController *VarController::_array_object(const std::string &name, const arraySizeType i) {
    Var *var;

    var = _find(name);
    if (var == NULL) {
      Unlock();
      throw VarController_Exception(ERR_NOT_OBJECT + "; " + name);
    } // if

    if ( !(i < var->size()) ) {
//      Unlock();
//      throw VarController_Exception(ERR_ARRAY_BOUNDS + "; "+name+"["+stringify<size_t>(i)+"]");
      return var->objecty();
    } // if

    return (*var)[i]->objecty();
  } // VarController::_array_object

  void *VarController::_get_void(const std::string &name) {
    Var *var;

    var = _find(name);
    if (var == NULL || var->_type != Var::VARTYPE_VOID) {
      Unlock();
      throw VarController_Exception(ERR_NOT_VOID + "; " + name);
    } // if

    return var->voidy();
  } // VarController::_get_void

  void *VarController::_get_void(const std::string &name, void *def) {
    Var *var;

    var = _find(name);
    if (var == NULL || var->_type != Var::VARTYPE_VOID)
      return def;

    return var->voidy();
  } // VarController::_get_void

  VarController *VarController::_get_object(const std::string &name) {
    Var *var;

    var = _find(name);
    if (var == NULL || var->_type != Var::VARTYPE_OBJECT) {
      Unlock();
      throw VarController_Exception(ERR_NOT_OBJECT + "; " + name);
    } // if

    return var->objecty();
  } // VarController::_get_obect

  // # Private Int

  // Int Members
  const int VarController::_array_int(const std::string &name, const arraySizeType i) {
    Var *var;

    var = _find(name);
    if (var == NULL) {
      Unlock();
      throw VarController_Exception(ERR_NOT_INT + "; " + name);
    } // if

    if ( !(i < var->size()) ) {
      Unlock();
      throw VarController_Exception(ERR_ARRAY_BOUNDS + "; "+name+"["+stringify<size_t>(i)+"]");
    } // if

    return (*var)[i]->integer();
  } // VarController::_array_int

  const int VarController::_get_int(const std::string &name) {
    Var *var;

    var = _find(name);
    if (var == NULL || var->_type != Var::VARTYPE_INT) {
      Unlock();
      throw VarController_Exception(ERR_NOT_INT + "; " + name);
    } // if

    return var->integer();
  } // VarController::_get_int

  const int VarController::_get_int(const std::string &name, const int def) {
    Var *var;

    var = _find(name);
    if (var == NULL || var->_type != Var::VARTYPE_INT)
      return def;

    return var->integer();
  } // VarController::_get_int

  void VarController::_inc(const std::string &name, const int increment) {
    Var *var;

    var = _find(name);
    if (var == NULL) {
      // if she no exist, create her
      var = new Var(Var::VARTYPE_UINT);
      var->_var.v_uint = increment;
      _varMap.insert( make_pair(name, var) );
      return;
    } // if

    switch(var->_type) {
      case Var::VARTYPE_DOUBLE:
        var->_var.v_double += double(increment);
        break;
      case Var::VARTYPE_INT:
        var->_var.v_int += increment;
        break;
      case Var::VARTYPE_UINT:
        var->_var.v_uint += (unsigned int) increment;
        break;
      default:
        Unlock();
        throw VarController_Exception(ERR_NOT_INT + "; " + name);
        break;
    } // switch

  } // VarController::_inc

  void VarController::_deinc(const std::string &name, const int increment) {
    Var *var;

    var = _find(name);
    if (var == NULL) {
      // if she no exist, create her
      var = new Var(Var::VARTYPE_UINT);
      var->_var.v_uint = increment;
      _varMap.insert( make_pair(name, var) );
      return;
    } // if

    switch(var->_type) {
      case Var::VARTYPE_DOUBLE:
        var->_var.v_double -= double(increment);
        break;
      case Var::VARTYPE_INT:
        var->_var.v_int -= increment;
        break;
      case Var::VARTYPE_UINT:
        if (var->_var.v_uint == 0) throw VcUintDeincOutOfRange_Exception();
        var->_var.v_uint -= (unsigned int) increment;
        break;
      default:
        Unlock();
        throw VarController_Exception(ERR_NOT_INT + "; " + name);
        break;
    } // switch

  } // VarController::_deinc

  // # Private Bool

  const bool VarController::_array_bool(const std::string &name, const arraySizeType i) {
    Var *var;

    var = _find(name);
    if (var == NULL) {
      Unlock();
      throw VarController_Exception(ERR_NOT_BOOL + "; " + name);
    } // if

    if ( !(i < var->size()) ) {
      Unlock();
      throw VarController_Exception(ERR_ARRAY_BOUNDS + "; "+name+"["+stringify<size_t>(i)+"]");
    } // if

    return (*var)[i]->boolean();
  } // VarController::_array_bool

  const bool VarController::_get_bool(const std::string &name) {
    Var *var;

    var = _find(name);
    if (var == NULL || var->_type != Var::VARTYPE_BOOL) {
      Unlock();
      throw VarController_Exception(ERR_NOT_BOOL + "; " + name);
    } // if

    return var->boolean();
  } // VarController::_get_bool

  const bool VarController::_get_bool(const std::string &name, const bool def) {
    Var *var;

    var = _find(name);
    if (var == NULL || var->_type != Var::VARTYPE_BOOL)
      return def;

    return var->boolean();
  } // VarController::_get_bool

  const bool VarController::_toggle(const std::string &name) {
    Var *var;

    var = _find(name);
    if (var == NULL || var->_type != Var::VARTYPE_BOOL) {
      Unlock();
      throw VarController_Exception(ERR_NOT_BOOL + "; " + name);
    } // if

    var->_var.v_bool = !var->_var.v_bool;

    return var->boolean();
  } // VarController::_toggle

  // std::string Members
  const VarController::arraySizeType VarController::_array_size(const std::string &name) {
    Var *var;

    var = _find(name);
    if (var == NULL) {
      Unlock();
      return 0;
      //throw VarController_Exception(ERR_NOT_ARRAY + "; " + name);
    } // if

    if (var->type() != Var::VARTYPE_ARRAY)
      return 1;

    return var->size();
  } // VarController::_array_size

  // std::string Members
  const std::string VarController::_array_string(const std::string &name, const arraySizeType i) {
    Var *var;

    var = _find(name);
    if (var == NULL) {
      Unlock();
      throw VarController_Exception(ERR_NOT_STRING + "; " + name);
    } // if

    if ( !(i < var->size()) ) {
//      Unlock();
      return var->str();
//      throw VarController_Exception(ERR_ARRAY_BOUNDS + "; "+name+"["+stringify<size_t>(i)+"]");
    } // if

    return (*var)[i]->str();
  } // VarController::_array_string

  // std::string Members
  const VarController::arrayStringList_s VarController::_array_string_list(const std::string &name, arrayStringList_t &ret) {
    ret.clear();

    for(arraySizeType i=0; i < _array_size(name); i++)
      ret.push_back( _array_string(name, i) );

    return ret.size();
  } // VarController::_array_string

  const std::string VarController::_array_string(const std::string &name, const arraySizeType i, const std::string &def) {
    Var *var;

    var = _find(name);
    if (var == NULL)
      return def;

    if ( !(i < var->size()) )
      return var->str();

    return (*var)[i]->str();
  } // VarController::_array_string

  // std::string Members
  const std::string VarController::_get_string(const std::string &name) {
    Var *var;

    var = _find(name);
    if (var == NULL) {
      Unlock();
      throw VarController_Exception(ERR_NOT_STRING + "; " + name);
    } // if

    return var->str();
  } // VarController::_get_string

  const std::string VarController::_get_string(const std::string &name, const std::string &def) {
    Var *var;

    var = _find(name);
    if (var == NULL)
      return def;

    return var->str();
  } // VarController::_get_string

  // Unsigned int Members

  const unsigned int VarController::_array_uint(const std::string &name, const arraySizeType i) {
    Var *var;

    var = _find(name);
    if (var == NULL) {
      Unlock();
      throw VarController_Exception(ERR_NOT_UINT + "; " + name);
    } // if

    if ( !(i < var->size()) ) {
      Unlock();
      throw VarController_Exception(ERR_ARRAY_BOUNDS + "; "+name+"["+stringify<size_t>(i)+"]");
    } // if

    return (*var)[i]->uinteger();
  } // VarController::_array_uint

  const unsigned int VarController::_get_uint(const std::string &name) {
    Var *var;

    var = _find(name);
    if (var == NULL) {
      Unlock();
      throw VarController_Exception(ERR_NOT_UINT + "; " + name);
    } // if

    return var->uinteger();
  } // VarController::_get_uint

  const unsigned int VarController::_get_uint(const std::string &name, const unsigned int def) {
    Var *var;

    var = _find(name);
    if (var == NULL)
      return def;

    return var->uinteger();
  } // VarController::_get_uint

  const bool VarController::_push_string(const std::string &name, const std::string &str, const bool replace) {
    Var *var = new Var(Var::VARTYPE_STRING);
    bool isOK;

    var->_string = str;

    isOK = _push(name, var, replace);

    if (!isOK)
      delete var;

    return isOK;
  } // VarController::_push_string

  const bool VarController::_push_bool(const std::string &name, const bool boolean, const bool replace) {
    Var *var = new Var(Var::VARTYPE_BOOL);
    bool isOK;

    var->_var.v_bool = boolean;

    isOK = _push(name, var, replace);

    if (!isOK)
      delete var;

    return isOK;
  } // VarController::_push_bool

  const bool VarController::_push_int(const std::string &name, const int integer, const bool replace) {
    Var *var = new Var(Var::VARTYPE_INT);
    bool isOK;

    var->_var.v_int = integer;

    isOK = _push(name, var, replace);

    if (!isOK)
      delete var;

    return isOK;
  } // VarController::_push_int

  const bool VarController::_push_uint(const std::string &name, const unsigned int integer, const bool replace) {
    Var *var = new Var(Var::VARTYPE_UINT);
    bool isOK;

    var->_var.v_uint = integer;

    isOK = _push(name, var, replace);

    if (!isOK)
      delete var;

    return isOK;
  } // VarController::_push_uint

  const bool VarController::_push_double(const std::string &name, const double d, const bool replace) {
    Var *var = new Var(Var::VARTYPE_DOUBLE);
    bool isOK;

    var->_var.v_double = d;

    isOK = _push(name, var, replace);

    if (!isOK)
      delete var;

    return isOK;
  } // VarController::_push_double

  const bool VarController::_push_void(const std::string &name, void *v, const bool replace) {
    Var *var = new Var(Var::VARTYPE_VOID);
    bool isOK;

    var->_var.v_void = v;

    isOK = _push(name, var, replace);

    if (!isOK)
      delete var;

    return isOK;
  } // VarController::_push_void

  const bool VarController::_push_object(const std::string &name, VarController *v, const bool replace) {
    Var *var = new Var(Var::VARTYPE_OBJECT);
    bool isOK;

    v->retain();
    var->_var.v_object = v;

    isOK = _push(name, var, replace);

    if (!isOK) delete var;

    return isOK;
  } // VarController::_push_object

  const bool VarController::_pop(const std::string &name) {
    varMapType::iterator ptr;

    ptr = _varMap.find(name);
    if (ptr == _varMap.end()) return false;

    delete ptr->second;
    _varMap.erase(ptr);

    return true;
  } // VarController::_pop

  const unsigned int VarController::_pop_match(const std::string &name) {
    matchListType matchList;
    varMapType::iterator ptr;
    unsigned int ret = 0;

    if (_match(name, matchList) > 0) {
      while(!matchList.empty()) {
        _pop(matchList.front());
        ret++;
        matchList.pop_front();
      } // while
    } // if

    return ret;
  } // VarController::_pop_match

  // ### Private Members ###
  const bool VarController::_push(const std::string &name, Var *var, const bool replace) {
    varMapType::iterator ptr;

    ptr = _varMap.find(name);
    if (ptr != _varMap.end()) {
      if (!replace) {
//        delete var;
//        Unlock();
//        throw VarController_Exception("tried to overwrite an existing var; "+name);
        // if we aren't to replace and it already exists then we add to the array
        if (ptr->second->type() != Var::VARTYPE_ARRAY) {
          Var *var_array = new Var(Var::VARTYPE_ARRAY);
          Var *var_old = ptr->second;
          _varMap.erase(ptr);
          var_array->push_back(var_old);
          var_array->push_back(var);

          _varMap.insert( make_pair(name, var_array) );
        } // if
        else
          ptr->second->push_back(var);

        return true;
      } // if

      delete ptr->second;
      _varMap.erase(ptr);
    } // if

    _varMap.insert( make_pair(name, var) );

    return true;
  } // VarController::_push

  /**
   * VarController::_load
   *
   * Attempts to load the processes config from the config file specified.
   *
   * Returns: Number of bytes read from the file.
   */
  const unsigned int VarController::_load(const std::string &filename) {
    StringToken aToken;			// tokenize a string
    LineBuffer aLine;			// line buffer
    bool innerVariable;			// Is the variable inside a scope?
    char fileBuf[512 + 1];		// file buffer
    int fd;				// pointer to a file
    int confLevel;			// Level we're at in the conf.
    std::string tempBuf;			// temp buffer
    std::string confValue;			// Value of the conf variable.
    std::string confName;			// Conf variable name.
    list<string> confList;		// List of conf variables.
    list<string> tempList;		// List of conf variables.
    unsigned int bytesRead;		// bytes read;
    unsigned int numLine;		// Line number we're processing.
    std::stringstream err;

    // initialize variables
    bytesRead = 0;
    err.str("");

    if ((fd = open(filename.c_str(), O_RDONLY)) == -1) {
      err << "could not load " << filename;
      Unlock();
      throw VarController_Exception(err.str());
    } // if

    while((bytesRead = read(fd, fileBuf, 512)) > 0) {
      fileBuf[bytesRead] = '\0';
      aLine.add(fileBuf);
    } // while

      // initialize variables
    confLevel = 0;

    for(numLine = 1; aLine.readLine(tempBuf); numLine++) {
      StringTool::stripcrlf(tempBuf);
      tempBuf = StringTool::trim(tempBuf);
      StringTool::replace("\t", " ", tempBuf);

      // is it a comment?  Yes?  Skip!
      if (tempBuf.size() < 1 || tempBuf[0] == '#')
        continue;

      // initialize variables
      aToken = tempBuf;
      confValue = "";
      innerVariable = false;

      /*
       * 09/19/2003: Check to see if we've jumped a level. -GCARTER
       */
      if (aToken[0] == "}") {
        confLevel--;
        if (confLevel < 0) {
          err << "unbalanced braces on line #" << numLine;
          Unlock();
          throw VarController_Exception(err.str());
        } // if

        confList.pop_back();
      } // if
      else if (aToken[aToken.size() - 1] == "{") {
        if (aToken.size() < 2) {
          err << "syntax error on line #" << numLine;
          Unlock();
          throw VarController_Exception(err.str());
        } // if

        confValue = aToken.getRange(1, (aToken.size() - 1));
        confList.push_back(aToken[0]);
        confLevel++;
      } // else if
      else if (aToken.size() > 1) {
        confValue = aToken.getTrailing(1);

        if (confValue[confValue.size() - 1] != ';') {
          err << "unterminated statement on line #" << numLine;
          Unlock();
          throw VarController_Exception(err.str());
        } // if

        confValue.replace(confValue.size() - 1, 1, "");
        confList.push_back(aToken[0]);
        innerVariable = true;
      } // else if
      else {
        err << "syntax error statement on line #" << numLine;
        Unlock();
        throw VarController_Exception(err.str());
      } // else

      // initialize variables;
      tempList = confList;
      confName = "";

      while(!tempList.empty()) {
        confName += tempList.front() + (tempList.size() > 1 ? "." : "");
        tempList.pop_front();
      } // while

      //cout << confName << (confValue.size() > 0 ? (" = " + confValue) : "") << endl;

      _valParse(numLine, confName, confValue);

      if (innerVariable == true)
        confList.pop_back();
    } // for

    if (confLevel != 0) {
      err << "unbalanced braces on line #" << numLine;
      Unlock();
      throw VarController_Exception(err.str());
    } // if

    close(fd);

    return bytesRead;
  } // VarController::_load

  void VarController::_valParse(const unsigned int numLine, const std::string &name, const std::string &value) {
    char acceptChars[] = "0123456789";
    std::string confValue = StringTool::trim(value);
    std::string confName = name;
    std::stringstream err;

    if (confValue.size() < 1)
      return;

//cout << confName << (confValue.size() > 0 ? (" = " + confValue) : "") << endl;

    if (StringTool::acceptable(acceptChars, confValue) == true)
      _push_int(confName, atoi(confValue.c_str()), true);
    else if (confValue == "true" || confValue == "on")
      _push_bool(confName, true, true);
    else if (confValue == "false" || confValue == "off")
      _push_bool(confName, false, true);
    else if (confValue[0] == '[') {
      // this is an array
      confValue.replace(0, 1, "");
      if (confValue[confValue.size()-1] != ']') {
        err << "unbalanced array on line #" << numLine;
        Unlock();
        throw VarController_Exception(err.str());
      } // if

      confValue.replace(confValue.size() - 1, 1, "");
      StringToken st;
      st.setDelimiter(',');
      st.quote('"');
      st = confValue;
      if (st.size() < 1) {
        err << "cannot parse array on line #" << numLine;
        Unlock();
        throw VarController_Exception(err.str());
      } // if

      for(size_t pos=0; pos < st.size(); pos++) {
        std::string nm = confName + stringify<size_t>(pos);
        _valParse(numLine, confName, st[pos]);
      } // for
    } // else if
    else {
      if (confValue[0] == '\"')
        confValue.replace(0, 1, "");

      if (confValue[confValue.size() - 1] == '"')
        confValue.replace(confValue.size() - 1, 1, "");

      _push_string(confName, confValue, false);
    } // else
  } // VarController::_valParse

  /**
   * VarController::_tree
   *
   * Returns a formatted list of all options matched in a tree format.
   *
   * Returns: Nothing.
   */
  void VarController::_tree(const std::string &treeMember, unsigned int treeLevel,
                                   const unsigned int treeColor, std::string treeSpacer,
                                   list<string> &treeList) {
    StringToken treeToken;                        // Tokenize a stringName.
    StringToken treeMemberToken;                  // Tokenise a treeMember name.
    varMapType::iterator treePtr;              // iterator to a std::string map
    int numMatches;                               // number of matches
    int maxMatches;                               // max matches
    std::string stringName;                            // Name of the string.
    std::string stringLast;				// Last part of the name.
    std::string stringValue;                           // Value of the string.
    std::string lastName;                              // Last name matched.
    std::stringstream s;                               // stringstream
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

    for(treePtr = _varMap.begin(); treePtr != _varMap.end(); treePtr++) {
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

    for(treePtr = _varMap.begin(); treePtr != _varMap.end(); treePtr++) {
      // initialize variables
      treeToken = stringName = string("root.") + treePtr->first;

      stringValue = treePtr->second->display(5);

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

        if ((treeLevel + 2) > treeToken.size())
          s << " = " << stringValue;

        treeList.push_back(s.str());
        treeSpacer += (numMatches == maxMatches) ? " " : "";

        _tree(treeToken.getRange(0, treeLevel), ++treeLevel, localColor,
                       treeSpacer, treeList);
        treeLevel--;

        treeSpacer.replace((treeSpacer.length() - 2), 2, "");

        lastName = treeToken.getRange(0, (treeLevel + 1));
      } // if
    }  // for

    return;
  } // VarController::_tree

  /**
   * VarController::_match
   *
   * Assembles a list of options that match the given mask.
   *
   * Returns: Number of options matched.
   */
  const VarController::matchListType::size_type VarController::_match(const std::string &matchName, matchListType &matchList) {
    varMapType::iterator ptr;		// iterator to varmap
    unsigned int size;

    // initialize variables
    matchList.clear();

    for(ptr = _varMap.begin(); ptr != _varMap.end(); ptr++) {
      if (StringTool::match(matchName.c_str(), ptr->first.c_str()))
        matchList.push_back(ptr->first);
    } // for

    size = matchList.size();

    return size;
  } // VarController::_match

} // namespace opentest
