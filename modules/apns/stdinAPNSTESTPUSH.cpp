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
#include <new>
#include <iostream>
#include <string>
#include <exception>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "APNS.h"
#include "ApnsMessage.h"
#include "ApnsLog_OpenAPRS.h"
#include "FeedbackController.h"
#include "StringToken.h"
#include "Command.h"
#include "Server.h"
#include "Vars.h"

#include "apnsCommands.h"

#include "apns.h"
#include "match.h"
#include "openaprs_string.h"

namespace apns {

using namespace std;
using namespace openaprs;

/**************************************************************************
 ** stdinAPNSTESTPUSH Class                                              **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

const int stdinAPNSTESTPUSH::Execute(COMMAND_ARGUMENTS) {
  Vars v;
  ApnsMessage *aMessage;
  stringstream s;
  string validDeviceToken;
  string invalidDeviceToken;
  string deviceToken;
  string message;
  int r;
  int testNum;

  app->writeLog(OPENAPRS_LOG_DEBUG, "stdinAPNSPUSH::Execute> /APNS TEST PUSH received.");

  if (ePacket->getArguments().size() < 1)
    return CMDERR_SYNTAX;

  validDeviceToken = ePacket->getArguments().getTrailing(0);

  // #1 test message -> valid token
  // #2 test message -> invalid token
  // #3 test a long message -> valid token
  // #4 test a utf8 message -> valid token

  for(int i=0; i < 64/2; i++) {
    r = rand() % 255;
    invalidDeviceToken += strtolower(v.char2hex(r));
  } // for

  // ### BEGIN: TEST #1 ###
  // message -> valid token
  message = "This is a valid text message";
  deviceToken = validDeviceToken;
  testNum = 1;

  try {
    aMessage = new ApnsMessage(deviceToken);
  } // try
  catch (ApnsMessage_Exception e) {
    printf("*** Test #%d - Failed to create new APNS message: %s\n", testNum, e.message());
    app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Test #%d - Failed to create new APNS message: %s", testNum, e.message());
    return CMDERR_SUCCESS;
  } // catch

  aMessage->text(message);
  aMessage->actionKeyCaption("View");
  aMessage->badgeNumber(1);

  aAPNS->Push(aMessage);

  printf("*** Test #%d - Queuing push: message -> valid token\n", testNum);
  app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Test #%d - Queuing push: message -> valid token", testNum);
  // ### END: TEST #1

  // --------------------------------------------------------------------------------------

  // ### BEGIN: TEST #2 ###
  // message -> invalid token
  message = "This is a valid text message";
  deviceToken = invalidDeviceToken;
  testNum++;

  try {
    aMessage = new ApnsMessage(deviceToken);
  } // try
  catch (ApnsMessage_Exception e) {
    printf("*** Test #%d - Failed to create new APNS message: %s\n", testNum, e.message());
    app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Test #%d - Failed to create new APNS message: %s", testNum, e.message());
    return CMDERR_SUCCESS;
  } // catch

  aMessage->text(message);
  aMessage->actionKeyCaption("View");
  aMessage->badgeNumber(1);

  aAPNS->Push(aMessage);

  printf("*** Test #%d - Queuing push: message -> invalid token\n", testNum);
  app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Test #%d - Queuing push: message -> invalid token", testNum);
  // ### END: TEST #2 ###

  // --------------------------------------------------------------------------------------

  // ### BEGIN: TEST #3 ###
  // long message -> valid token

  // create a random text message
  s.str("");
  for(int i=0; i < 260; i++) {
    r = (rand() % 94) + 32;
    s << (char) r;
  } // for

  message = s.str();
  deviceToken = validDeviceToken;
  testNum++;

  try {
    aMessage = new ApnsMessage(deviceToken);
  } // try
  catch (ApnsMessage_Exception e) {
    printf("*** Test #%d - Failed to create new APNS message: %s\n", testNum, e.message());
    app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Test #%d - Failed to create new APNS message: %s", testNum, e.message());
    return CMDERR_SUCCESS;
  } // catch

  aMessage->text(message);
  aMessage->actionKeyCaption("View");
  aMessage->badgeNumber(1);

  aAPNS->Push(aMessage);

  printf("*** Test #%d - Queuing push: long message -> valid token\n", testNum);
  app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Test #%d - Queuing push: long message -> valid token", testNum);
  // ### END: TEST #3 ###

  // --------------------------------------------------------------------------------------

  // ### BEGIN: TEST #4 ###
  // utf8 message -> valid token

  message = "This is a test message with valid utf8 \xc3\xb4\xc3\xb9 let's see if it works";
  deviceToken = validDeviceToken;
  testNum++;

  try {
    aMessage = new ApnsMessage(deviceToken);
  } // try
  catch (ApnsMessage_Exception e) {
    printf("*** Test #%d - Failed to create new APNS message: %s\n", testNum, e.message());
    app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Test #%d - Failed to create new APNS message: %s", testNum, e.message());
    return CMDERR_SUCCESS;
  } // catch

  aMessage->text(message);
  aMessage->actionKeyCaption("View");
  aMessage->badgeNumber(1);

  aAPNS->Push(aMessage);

  printf("*** Test #%d - Queuing push: utf8 -> valid token\n", testNum);
  app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Test #%d - Queuing push: utf8 -> valid token", testNum);
  // ### END: TEST #4 ###

  // --------------------------------------------------------------------------------------

  // ### BEGIN: TEST #5 ###
  // test feedback response
  testNum++;
  FeedbackController *feedback = new FeedbackController(DEVEL_APPLE_FEEDBACK_HOST, DEVEL_APPLE_FEEDBACK_PORT, DEVEL_RSA_CLIENT_CERT, DEVEL_RSA_CLIENT_KEY, DEVEL_CA_CERT_PATH, DEVEL_APPLE_FEEDBACK_TIMEOUT);

  feedback->logger(new ApnsLog_OpenAPRS("TestFeedback"));

  feedback->testFeedbackResponse();

  printf("*** Test #%d - Testing: feedback controller response (check logs)\n", testNum);
  app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Test #%d - Testing: feedback controller response", testNum);
  // ### END: TEST #5 ###

  // ### BEGIN: TEST #6 ###
  // test feedback response
  testNum++;

  bool isOK = feedback->testDeviceTokenTools();

  printf("*** Test #%d - Testing: device token tools (check logs) [%s]\n", testNum, isOK ? "PASS" : "FAIL");
  app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Test #%d - Testing: device token tools [%s]", testNum, isOK ? "PASS" : "FAIL");

  delete feedback;

  return CMDERR_SUCCESS;
} // stdinAPNSTESTPUSH::Execute

}
