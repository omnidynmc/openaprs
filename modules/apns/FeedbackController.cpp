/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** HAL9000, Internet Relay Chat Bot                                     **
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
 $Id: APNS.cpp,v 1.12 2003/09/05 22:23:41 omni Exp $
 **************************************************************************/

#include <fstream>
#include <string>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <list>
#include <map>
#include <new>
#include <iostream>
#include <fstream>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include "ApnsMessage.h"
#include "Log.h"
#include "PutMySQL.h"
#include "Server.h"
#include "FeedbackController.h"
#include "Vars.h"

#include "apns.h"

namespace apns {

  using namespace openaprs;
  using namespace handler;
  using namespace putmysql;
  using namespace std;

/**************************************************************************
 ** APNS Class                                                           **
 **************************************************************************/

  const int FeedbackController::FEEDBACK_RESPONSE_SIZE	= 38;

  FeedbackController::FeedbackController(const string &host, const int port, const string &certfile, const string &keyfile, const string &capath, const time_t timeout) :
    SslController(host, port, certfile, keyfile, capath), _timeout(timeout) {

    _nextCheckTs = time(NULL) + timeout;

    return;
  } // FeedbackController::FeedbackController

  FeedbackController::~FeedbackController() {
    if (isConnected())
      disconnect();

    return;
  } // FeedbackController::~FeedbackController

  const bool FeedbackController::run() {
    if (time(NULL) < _nextCheckTs)
      return false;

    _nextCheckTs = time(NULL)+_timeout;

    if (!isConnected() && !connect()) {
      _log("WARNING: Could not connect to feedback server, will try again later.");
      return false;
    } // if

    _logf("STATUS: Checking APNS feedback servers after %d seconds.", _timeout);
    _readFeedbackFromApns();

    //_testFeedbackResponse();

    disconnect();

    return true;
  } // PushController::run

  void FeedbackController::_testFeedbackResponse() {
    ApnsFeedbackResponse_t r;
    time_t now = time(NULL);
    uint32_t timestamp = htonl(now);
    uint16_t tokenLen = htons(DEVICE_BINARY_SIZE);
    char binaryDeviceToken[DEVICE_BINARY_SIZE];
    string deviceToken = generateRandomDeviceToken();
    size_t packetLen = sizeof(uint32_t) + sizeof(uint16_t) + DEVICE_BINARY_SIZE;

    _deviceTokenToBinary(binaryDeviceToken, deviceToken, DEVICE_BINARY_SIZE);

    memcpy(&r.timestamp, &timestamp, sizeof(uint32_t));
    memcpy(&r.tokenLen, &tokenLen, sizeof(uint16_t));
    memcpy(&r.deviceToken, &binaryDeviceToken, DEVICE_BINARY_SIZE);

    _logf("INFO: Testing FeedbackResponse system with timestamp(%d) tokenLen(%d) deviceToken(%s) packetLen(%d)", now, DEVICE_BINARY_SIZE, deviceToken.c_str(), packetLen);
    _debugf("INFO: Testing FeedbackResponse system with timestamp(%d) tokenLen(%d) deviceToken(%s) packetLen(%d)", now, DEVICE_BINARY_SIZE, deviceToken.c_str(), packetLen);
    _debugf("INFO: TST |%s|", _safeBinaryOutput((char *) &r, packetLen).c_str());

    _processFeedbackFromApns(&r);
  } // FeedbackController::_testFeedbackResponse

  void FeedbackController::_readFeedbackFromApns() {
    ApnsFeedbackResponse_t r;
    char response[FEEDBACK_RESPONSE_SIZE];
    char *ptr = response;
    int ret;

    ret = read((void *) &response, FEEDBACK_RESPONSE_SIZE);

    if (ret < 1)
      return;

    memcpy(&r.timestamp, ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    memcpy(&r.tokenLen, ptr, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    memcpy(&r.deviceToken, ptr, DEVICE_BINARY_SIZE);

    _debugf("INFO: Received feedback from APNS that was %d bytes.", ret);

    _processFeedbackFromApns(&r);
  } // FeedbackController::_readFeedbackFromApns

  void FeedbackController::_processFeedbackFromApns(const ApnsFeedbackResponse_t *r) {
    FeedbackMessage *aFbMessage;
    time_t timestamp;
    char binaryDeviceToken[DEVICE_BINARY_SIZE];
    string deviceToken;
    stringstream s;
    unsigned int tokenLen;

    memcpy(&timestamp, r->timestamp, sizeof(uint32_t));
    memcpy(&tokenLen, r->tokenLen, sizeof(uint16_t));
    memcpy(&binaryDeviceToken, r->deviceToken, DEVICE_BINARY_SIZE);

    timestamp = ntohl(timestamp);
    tokenLen = ntohs(tokenLen);

    char *ptr = binaryDeviceToken;

    deviceToken = _binaryToDeviceToken(ptr, DEVICE_BINARY_SIZE);

    aFbMessage = new FeedbackMessage(timestamp, tokenLen, deviceToken);
    _messageFeedbackQueue.insert(aFbMessage);

    _debugf("INFO: Feedback response: timestamp(%d) tokenLen(%d) deviceToken(%s)", timestamp, tokenLen, deviceToken.c_str());
    _logf("INFO: Feedback response: timestamp(%d) tokenLen(%d) deviceToken(%s)", timestamp, tokenLen, deviceToken.c_str());

  } // PushController::_processFeedbackFromApns

} // namespace apns

