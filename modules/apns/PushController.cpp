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
#include "PushController.h"

#include "apns.h"

namespace apns {

  using namespace openaprs;
  using namespace handler;
  using namespace putmysql;
  using namespace std;

/**************************************************************************
 ** APNS Class                                                           **
 **************************************************************************/
  const time_t PushController::CONNECT_RETRY_TIMEOUT 	= 60;
  const time_t PushController::DEFAULT_STATS_INTERVAL 	= 3600;
  const int PushController::ERROR_RESPONSE_SIZE 	= 6;
  const int PushController::ERROR_RESPONSE_COMMAND 	= 8;

  PushController::PushController(const string &host, const int port, const string &certfile, const string &keyfile, const string &capath, const time_t timeout) :
    SslController(host, port, certfile, keyfile, capath), _timeout(timeout) {

    _lastId = 0;
    _logStatsInterval = DEFAULT_STATS_INTERVAL;			// log stats every hour
    _logStatsTs = time(NULL) + _logStatsInterval;
    _lastActivityTs = time(NULL);
    _connectRetryTimeout = CONNECT_RETRY_TIMEOUT;

    _numStatsError = 0;
    _numStatsSent = 0;
    _numStatsDisconnected = 0;

    return;
  } // PushController::PushController

  PushController::~PushController() {
    _clearMessagesFromQueue(_messageSendQueue);
    _clearMessagesFromQueue(_messageStageQueue);
    _clearMessagesFromQueue(_messageErrorQueue);

    if (isConnected())
      disconnect();

    return;
  } // PushController::~PushController

  const bool PushController::run() {
    unsigned int numRows;

    if (time(NULL) < _connectRetryTs)
      return false;

    if (time(NULL) > _logStatsTs)
      _logStats();

    _processMessageSendQueue();
    _expireIdleConnection();

    if ((numRows = _removeExpiredMessagesFromQueue(_messageStageQueue)) > 0)
      _logf("STATUS: Expired %d message%s from stage queue.", numRows, numRows == 1 ? "" : "s");

    if ((numRows =_removeExpiredMessagesFromQueue(_messageErrorQueue)) > 0)
      _logf("STATUS: Expired %d message%s from error queue.", numRows, numRows == 1 ? "" : "s");

    return true;
  } // PushController::run

  void PushController::_processMessageSendQueue() {
    messageQueueType::iterator ptr;		// message queue iterator
    messageQueueType processQueue;		// local queue storage for processing
    unsigned int id;
    int numBytes;

    if (_messageSendQueue.empty())
      return;

    if (!isConnected() && !connect()) {
      _logf("WARNING: Messages (%d) ready to send but unable connect, will retry in %d seconds.", _messageSendQueue.size(), _connectRetryTimeout);
      _connectRetryTs = time(NULL) + _connectRetryTimeout;
      return;
    } // if

    _logf("INFO: Sending message queue: %d message(s) left in queue.", _messageSendQueue.size());

    // We don't want to edit the real queue so that
    // the sending queue can put failed messages back into
    // the send queue.
    processQueue = _messageSendQueue;
    while(!processQueue.empty() && isConnected()) {
      ptr = processQueue.begin();

      _messageStageQueue.insert(*ptr);
      _messageSendQueue.erase(*ptr);
      processQueue.erase(*ptr);
      id = (*ptr)->id();

      _sendPayload(*ptr);
      if ((numBytes = _readResponseFromApns()) > 0) {
        _logf("STATUS: Detected a response with %d bytes to [custom identifier: %d] deferring %d queued for reconnect.", numBytes, id, _messageSendQueue.size());
        // On error, we will get disconnected
        disconnect();
        _numStatsDisconnected++;
        _numStatsError++;
        //break;
      } // if
    } // while

    return;
  } // _processMessageQueue

  void PushController::_expireIdleConnection() {
    if (!_timeout || !isConnected())
      // Don't expire if timeout is 0
      return;

    if (time(NULL) < (_lastActivityTs+_timeout))
      // not time to expire yet
      return;

    _logf("INFO: Connection expired after %d seconds.", _timeout);

    // connection is expired, disconnect for now.
    disconnect();
  } // PushController::_expireIdleConnection

  const int PushController::_readResponseFromApns() {
    ApnsResponse_t r;
    char response[ERROR_RESPONSE_SIZE];
    char *ptr = response;
    int ret;

    ret = read((void *) &response, ERROR_RESPONSE_SIZE);

    if (ret < 1)
      return -1;

    _debugf("INFO: Received response from APNS that was %d bytes.", ret);

    memcpy(&r.command, ptr++, sizeof(uint8_t));
    memcpy(&r.status, ptr++, sizeof(uint8_t));
    memcpy(&r.identifier, ptr, sizeof(uint32_t));

    _processResponseFromApns(&r);

    return ret;
  } // PushController::_readResponseFromApns

  void PushController::_processResponseFromApns(const ApnsResponse_t *r) {
    ApnsMessage *aMessage;
    char command;			// Command
    char status;			// Status
    unsigned int identifier;		// Identifier

    memcpy(&identifier, r->identifier, sizeof(uint32_t));

    command = r->command;
    status = r->status;
    identifier = ntohl(identifier);

    string safeBinaryOutput = _safeBinaryOutput((char *) &r, 6);

    _debugf("INFO: RX |%s| bytes(%d)", safeBinaryOutput.c_str(), safeBinaryOutput.length());

    if ((int) command !=  ERROR_RESPONSE_COMMAND) {
      _logf("WARNING: Reponse command unknown: %d for [custom identifier: %d]", (int) command, identifier);
      return;
    } // if

    // move message to the error queue
    aMessage = _findById(identifier);
    if (aMessage != NULL) {
      _removeMessageFromQueue(aMessage, true);
      aMessage->error(status);
    } // if

    switch((int) status) {
      case ERR_NO_ERRORS:
        _logf("INFO: Message reponse [custom identifier: %d]: NO ERROR (%d)", (int) identifier, status);
        return;
        break;
      case ERR_PROCESSING_ERROR:
        _logf("WARNING: Message reponse [custom identifier: %d]: PROCESSING ERROR (%d)", identifier, status);
        break;
      case ERR_MISSING_DEVICE_TOKEN:
        _logf("WARNING: Message response [custom identifier: %d]: MISSING DEVICE TOKEN (%d)", identifier, status);
        break;
      case ERR_MISSING_TOPIC:
        _logf("WARNING: Message response [custom identifier: %d]: MISSING TOPIC (%d)", identifier, status);
        break;
      case ERR_MISSING_PAYLOAD:
        _logf("WARNING: Message response [custom identifier: %d]: MISSING PAYLOAD (%d)", identifier, status);
        break;
      case ERR_INVALID_TOKEN_SIZE:
        _logf("WARNING: Message response [custom identifier: %d]: INVALID TOKEN SIZE (%d)", identifier, status);
        break;
      case ERR_INVALID_TOPIC_SIZE:
        _logf("WARNING: Message response [custom identifier: %d]: INVALID TOPIC SIZE (%d)", identifier, status);
        break;
      case ERR_INVALID_PAYLOAD_SIZE:
        _logf("WARNING: Message response [custom identifier: %d]: INVALID PAYLOAD SIZE (%d)", identifier, status);
        break;
      case ERR_INVALID_TOKEN:
        _logf("WARNING: Message response [custom identifier: %d]: INVALID TOKEN (%d)", identifier, status);
        break;
      case ERR_NONE_UNKNOWN:
        _logf("WARNING: Message response [custom identifier: %d]: NON UNKNOWN (%d)", identifier, status);
        break;
    } // switch

  } // PushController::_processReponseFromApns

  // ### Queue Management ###
  void PushController::add(ApnsMessage *aMessage) {
    _add(aMessage);
  } // PushController::add

  void PushController::_add(ApnsMessage *aMessage) {
    assert(aMessage != NULL);

    // Set id for message, probably never have over 1024 messages
    // in the queue.
    aMessage->id(++_lastId % 1024);

    // update our last activity
    _lastActivityTs = time(NULL);

    _messageSendQueue.insert(aMessage);
  } // PushController::_add

  const bool PushController::remove(ApnsMessage *aMessage) {
    bool ret;

    ret = _remove(aMessage);

    return ret;
  } // PushController::remove

  const bool PushController::_remove(ApnsMessage *aMessage) {
    assert(aMessage != NULL);

    if (_messageSendQueue.find(aMessage) == _messageSendQueue.end())
      return false;

    _messageSendQueue.erase(aMessage);

    delete aMessage;

    return true;
  } // PushController::_remove

  ApnsMessage *PushController::_findById(const unsigned int id) {
    messageQueueType::iterator ptr;

    ptr = _messageStageQueue.begin();
    while(ptr != _messageStageQueue.end()) {
      if ((*ptr)->id() == id)
        return *ptr;
      ptr++;
    } // while

    return NULL;
  } // PushController::_findById

  void PushController::_removeMessageFromQueueById(const unsigned int id, const bool error) {
    ApnsMessage *aMessage;

    aMessage = _findById(id);
    if (aMessage == NULL)
      throw PushController_Exception("Unable to find ApnsMessage by id");

    _messageStageQueue.erase(aMessage);

    if (error)
      _messageErrorQueue.insert(aMessage);
    else
      delete aMessage;

  } // PushController::_removeMessageFromQueueById

  const unsigned int PushController::_clearMessagesFromQueue(messageQueueType &messageQueue) {
    const unsigned int numRows = messageQueue.size();
    messageQueueType::iterator ptr;

    while(!messageQueue.empty()) {
      ptr = messageQueue.begin();
      messageQueue.erase(ptr);
      delete (*ptr);
    } // while

    return numRows;
  } // _clearMessagesFromQueue

  const unsigned int PushController::_removeExpiredMessagesFromQueue(messageQueueType &messageQueue) {
    messageQueueType::iterator ptr;
    messageQueueType removeMe;
    string queueName = "";
    unsigned int numRows = 0;

    ptr = messageQueue.begin();
    while(ptr != messageQueue.end()) {
      if (time(NULL) > (*ptr)->expiry())
        removeMe.insert((*ptr));

      ptr++;
    } // while

    while(!removeMe.empty()) {
      ptr = removeMe.begin();
      messageQueue.erase((*ptr));
      removeMe.erase((*ptr));

      // we're expire, remove it
      //_logf("STATUS: Expired message [custom identifier: %d]: Removed from queue.", (*ptr)->id());

      delete (*ptr);
      numRows++;
    } // while

    return numRows;
  } // PushController::_removeExpiredMessagesFromQueue

  void PushController::_removeMessageFromQueue(ApnsMessage *aMessage, const bool error) {
    messageQueueType::iterator ptr;

    ptr = _messageStageQueue.find(aMessage);
    if (ptr == _messageStageQueue.end())
      throw PushController_Exception("Unable to find ApnsMessage");

    _messageStageQueue.erase(*ptr);

    if (error)
      _messageErrorQueue.insert(*ptr);
    else
      delete *ptr;

  } // PushController::_removeMessageFromQueue

  const bool PushController::_sendPayload(ApnsMessage *aMessage) {
    ApnsPacket_Enhanced_t p;
    string payloadString;
    char deviceTokenHex[aMessage->deviceToken().length()+1];
    char payload[MAXPAYLOAD_SIZE];
    size_t payloadLen;
    int ret;

    // Should never happen, we are only called by _buildPacket which
    // will set this when done.
    assert(aMessage != NULL);

    // Should we retry?
    if (!aMessage->retry()) {
      _logf("Giving up on message [custom identifier: %d] after retry (%d) count expired.", aMessage->id(), aMessage->retries());
      _removeMessageFromQueue(aMessage, false);
      return false;
    } // if

    try {
      payloadString = aMessage->getPayload();
    } // try
    catch(ApnsMessage_Exception e) {
      _logf("WARNING: Message removed [custom identifier: %d]: %s", aMessage->id(), e.message());
      aMessage->error(ERR_INVALID_PAYLOAD_SIZE);
      _removeMessageFromQueue(aMessage, true);
      return false;
    } // catch

    strncpy(deviceTokenHex, aMessage->deviceToken().c_str(), sizeof(deviceTokenHex));
    strncpy(payload, payloadString.c_str(), sizeof(payload));
    payloadLen = payloadString.length();

    bzero(&p, sizeof(p));

    _debugf("INFO: Sending[%s] of (%s) %d bytes", deviceTokenHex, payload, payloadLen);

    p.command = (char) COMMAND_PUSH_ENHANCED;
    //p.command = COMMAND_PUSH_ENHANCED;

    // message format is, |COMMAND|TOKENLEN|TOKEN|PAYLOADLEN|PAYLOAD|
    //char binaryMessageBuff[sizeof(uint8_t) + sizeof(uint16_t) + DEVICE_BINARY_SIZE + sizeof(uint16_t) + MAXPAYLOAD_SIZE];

    uint16_t networkOrderTokenLength = htons(DEVICE_BINARY_SIZE);
    uint16_t networkOrderPayloadLength = htons(payloadLen);
    uint32_t networkOrderIdentifier = htonl(aMessage->id());
    uint32_t networkOrderExpiry = htonl(time(NULL)+300);
    memcpy(&p.tokenLen, &networkOrderTokenLength, sizeof(uint16_t));
    memcpy(&p.payloadLen, &networkOrderPayloadLength, sizeof(uint16_t));
    memcpy(&p.identifier, &networkOrderIdentifier, sizeof(uint32_t));
    memcpy(&p.expiry, &networkOrderExpiry, sizeof(uint32_t));

    //p.tokenLen = htons(DEVICE_BINARY_SIZE);
    //p.payloadLen = htons(_packetLen);
    //p.identifier = htonl(1);
    //p.expiry = htonl(time(NULL)+300);

    // Convert the Device Token
    unsigned int i = 0;
    int j = 0;
    int tmpi;
    char tmp[3];
    while(i < strlen(deviceTokenHex)) {
      if (deviceTokenHex[i] == ' ') {
        i++;
      } // if
      else {
        tmp[0] = deviceTokenHex[i];
        tmp[1] = deviceTokenHex[i + 1];
        tmp[2] = '\0';

        sscanf(tmp, "%x", &tmpi);
        p.deviceToken[j] = tmpi;

        i += 2;
        j++;
      } // else
    } // while

    // payload
    memcpy(p.payload, payload, payloadLen);

    //int payloadOffset = sizeof(uint8_t) + sizeof(uint16_t) + DEVICE_BINARY_SIZE + sizeof(uint16_t);
    int payloadOffset = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + DEVICE_BINARY_SIZE + sizeof(uint16_t);
    int packetLen = payloadOffset+payloadLen;

    char packet[packetLen];
    char *ptr = packet;

    memcpy(ptr++, &p.command, sizeof(uint8_t));
    memcpy(ptr, &p.identifier, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    memcpy(ptr, &p.expiry, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    memcpy(ptr, &p.tokenLen, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    memcpy(ptr, &p.deviceToken, DEVICE_BINARY_SIZE);
    ptr += DEVICE_BINARY_SIZE;
    memcpy(ptr, &p.payloadLen, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    memcpy(ptr, &p.payload, payloadLen);

    ret = write((char *) &packet, packetLen);

    _debugf("INFO: Write returned: %d", ret);

    if (ret < 1 || ret != packetLen) {
      // If we failed to write this packet, we need to put it back
      // into the queue
      _messageStageQueue.erase(aMessage);
      _messageSendQueue.insert(aMessage);
      _logf("WARNING: Unable to send message [customer identifier: %d].  Wrote %d of %d bytes, pushing back to send queue.", aMessage->id(), ret, packetLen);
    } // if

    _debugf("INFO: TX |%s| payloadOffset(%d) packetLen(%d) bytes(%d)", _safeBinaryOutput(packet, packetLen).c_str(), payloadOffset, payloadLen, packetLen);
    _logf("INFO: Sending message [custom identifier: %d]: %d bytes, try #%d", aMessage->id(), packetLen, aMessage->retries());

    _numStatsSent++;

    return ret ? true : false;
  } // PushController::_sendPayload

  void PushController::_logStats() {
    _logStatsTs = time(NULL) + _logStatsInterval;

    _logf("INFO: Statistics Sent(%d) Errors(%d) Disconnects(%d) next in %d seconds", _numStatsSent, _numStatsError, _numStatsDisconnected, _logStatsInterval);
  } // PushController::_logStats

} // namespace apns

