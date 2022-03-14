#ifndef __LIBOPENFRAME_BYTEDATA_H
#define __LIBOPENFRAME_BYTEDATA_H

#include <arpa/inet.h>

#include <string>
#include <exception>
#include <stdexcept>

#include <stdint.h>

namespace openframe {
  using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class ByteData : public string {
    public:
      typedef string super;
      ByteData() : bytedata_pos_(0) { }
      ByteData(const char *ch) : super(ch), bytedata_pos_(0) { }
      ByteData(const string &str) : super(str), bytedata_pos_(0) { }
      virtual ~ByteData() { }

      ByteData &operator=(const char *ch) {
        bytedata_pos_ = 0;
        return (ByteData &)string::assign(ch);
      } // ByteData

      ByteData &operator=(const string &str) {
        bytedata_pos_ = 0;
        return (ByteData &)string::assign(str);
      } // ByteData

      ByteData &assign(const char *s, size_t n) {
        bytedata_pos_ = 0;
        return (ByteData &)string::assign(s, n);
      } // assign

      void eraseToNext() {
        erase(0, bytedata_pos_);
        bytedata_pos_ = 0;
      } // eraseToNext

      const char *nextData() const {
        return substr(bytedata_pos_, length()).data();
      } // nextData

      const string nextStr() const {
        return substr(bytedata_pos_, length());
      } // nextStr

      const ByteData::size_type nextLength() const {
        return length() - bytedata_pos_;
      } // dataLength

      const string nextCString() {
        size_type pos = find('\0', bytedata_pos_);
        if (pos == npos) throw std::out_of_range("cannot find nul char");

        size_t len = pos - bytedata_pos_;

        string ret = substr(bytedata_pos_, len);
        bytedata_pos_ += len+1;
        return ret;
      } // nextCString

      const bool nextCString(std::string &ret) {
        size_type pos = find('\0', bytedata_pos_);
        if (pos == npos) return false;

        size_t len = pos - bytedata_pos_;

        ret = substr(bytedata_pos_, len);
        bytedata_pos_ += len+1;
        return true;
      } // nextCString

      const string nextCString(const size_t limit) {
        size_type pos = find('\0', bytedata_pos_);
        if (pos == npos) throw std::out_of_range("cannot find nul char");
        if ((pos+1) - bytedata_pos_ > limit) throw std::out_of_range("cstring size over limit");

        size_t len = pos - bytedata_pos_;

        string ret = substr(bytedata_pos_, len);
        bytedata_pos_ += len+1;
        return ret;
      } // nextCString

      const string nextLine() {
        size_type pos = find('\n', bytedata_pos_);
        if (pos == npos) throw std::out_of_range("cannot find newline char");

        size_t len = pos - bytedata_pos_;

        string ret = substr(bytedata_pos_, len);
        bytedata_pos_ += len+1;
        return ret;
      } // nextLine

      const bool nextLine(std::string &ret) {
        size_type pos = find('\n', bytedata_pos_);
        if (pos == npos) return false;

        size_t len = pos - bytedata_pos_;

        ret = substr(bytedata_pos_, len);
        bytedata_pos_ += len+1;
        return true;
      } // nextLine

      const string nextLine(const size_t limit) {
        size_type pos = find('\n', bytedata_pos_);
        if (pos == npos) throw std::out_of_range("cannot find newline char");
        if ((pos+1) - bytedata_pos_ > limit) throw std::out_of_range("line size over limit");

        size_t len = pos - bytedata_pos_;

        string ret = substr(bytedata_pos_, len);
        bytedata_pos_ += len+1;
        return ret;
      } // nextLine

      void jumpBytes(const size_t n) {
        size_type remaining = length() - bytedata_pos_;
        if (n > remaining) throw std::out_of_range("oversize jump");
        bytedata_pos_ += n;
      } // jumpBytes

      const char nextByte() {
        if (length() == 0) throw std::out_of_range("empty byte sequence");
        char ret = at(bytedata_pos_);
        bytedata_pos_++;
        return ret;
      } // nextByte

      size_t nextBytes(char *s, size_t n) {
        size_t ret = copy(s, bytedata_pos_, n);
        bytedata_pos_ += n;
        return ret;
      } // nextBytes

      const string nextBytes(size_t n) {
        string ret = substr(bytedata_pos_, n);
        bytedata_pos_ += n;
        return ret;
      } // nextBytes

      const uint16_t nextShort() {
        string bytes = nextBytes( sizeof(uint16_t) );
        uint16_t ret;
        bytes.copy( reinterpret_cast<char *>(&ret), sizeof(uint16_t) );
        //ret = htons(ret);
        return ret;
      } // nextShort

      const uint16_t nextNetworkShort() {
        string bytes = nextBytes( sizeof(uint16_t) );
        uint16_t ret;
        bytes.copy( reinterpret_cast<char *>(&ret), sizeof(uint16_t) );
        ret = ntohs(ret);
        return ret;
      } // nextNetworkShort

      const uint32_t nextLong() {
        string bytes = nextBytes( sizeof(uint32_t) );
        uint32_t ret;
        bytes.copy( reinterpret_cast<char *>(&ret), sizeof(uint32_t) );
        //ret = htonl(ret);
        return ret;
      } // nextLong

      const uint32_t nextNetworkLong() {
        string bytes = nextBytes( sizeof(uint32_t) );
        uint32_t ret;
        bytes.copy( reinterpret_cast<char *>(&ret), sizeof(uint32_t) );
        ret = ntohl(ret);
        return ret;
      } // nextNetworkLong

      // ### Remove Members ###
      const string removeCString(const size_t limit) {
        size_type pos = find('\0');
        if (pos == npos) throw std::out_of_range("cannot find nul char");
        if (pos+1 > limit) throw std::out_of_range("cstring size over limit");
        bytedata_pos_ = 0;

        string ret = substr(0, pos);
        erase(0, pos+1);
        return ret;
      } // removeCString

      const string removeCString() {
        size_type pos = find('\0');
        if (pos == npos) throw std::out_of_range("cannot find nul char");
        bytedata_pos_ = 0;

        string ret = substr(0, pos);
        erase(0, pos+1);
        return ret;
      } // removeCString

      const bool removeCString(string &ret) {
        size_type pos = find('\0');
        if (pos == npos) return false;
        bytedata_pos_ = 0;

        ret = substr(0, pos);
        erase(0, pos+1);
        return true;
      } // removeCString

      const string removeLine(const size_t limit) {
        size_type pos = find('\n');
        if (pos == npos) throw std::out_of_range("cannot find newline char");
        if (pos+1 > limit) throw std::out_of_range("line size over limit");
        bytedata_pos_ = 0;

        string ret = substr(0, pos);
        erase(0, pos+1);
        return ret;
      } // removeLine

      const string removeLine() {
        size_type pos = find('\n');
        if (pos == npos) throw std::out_of_range("cannot find newline char");
        bytedata_pos_ = 0;

        string ret = substr(0, pos);
        erase(0, pos+1);
        return ret;
      } // removeLine

      const bool removeLine(string &ret) {
        size_type pos = find('\n');
        if (pos == npos) return false;
        bytedata_pos_ = 0;

        ret = substr(0, pos);
        erase(0, pos+1);
        return true;
      } // removeLine

      const char removeByte() {
        if (length() == 0) throw std::out_of_range("empty byte sequence");
        bytedata_pos_ = 0;
        char ret = at(0);
        erase(0, 1);
        return ret;
      } // removeByte

      size_t removeBytes(char *s, size_t n, size_t pos = 0) {
        size_t ret = copy(s, pos, n);
        bytedata_pos_ = 0;
        erase(pos, n);
        return ret;
      } // removeBytes

      const string removeBytes(size_t n, size_t pos = 0) {
        string ret = substr(pos, n);
        bytedata_pos_ = 0;
        erase(pos, n);
        return ret;
      } // removeBytes

      const uint16_t removeShort() {
        string bytes = removeBytes( sizeof(uint16_t) );
        uint16_t ret;
        bytes.copy(reinterpret_cast<char *>(&ret), sizeof(uint16_t) );
        //ret = htons(ret);
        return ret;
      } // removeShort

      const uint16_t removeNetworkShort() {
        string bytes = removeBytes( sizeof(uint16_t) );
        uint16_t ret;
        bytes.copy(reinterpret_cast<char *>(&ret), sizeof(uint16_t) );
        ret = ntohs(ret);
        return ret;
      } // removeNetworkShort

      const uint32_t removeLong() {
        string bytes = removeBytes( sizeof(uint32_t) );
        uint32_t ret;
        bytes.copy(reinterpret_cast<char *>(&ret), sizeof(uint32_t) );
        //ret = htonl(ret);
        return ret;
      } // removeLong

      const uint32_t removeNetworkLong() {
        string bytes = removeBytes( sizeof(uint32_t) );
        uint32_t ret;
        bytes.copy(reinterpret_cast<char *>(&ret), sizeof(uint32_t) );
        ret = ntohl(ret);
        return ret;
      } // removeNetworkLong

    protected:
      size_type bytedata_pos_;
    private:
  }; // class ByteData

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

  extern "C" {
  } // extern

} // namespace openframe
#endif
