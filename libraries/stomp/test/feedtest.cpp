#include <cassert>
#include <exception>
#include <iostream>
#include <new>
#include <string>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <openframe/openframe.h>
#include <stomp/StompParser.h>
#include <stomp/StompFrame.h>

#include "Feed.h"

stomp::Feed *_feed;
std::string _last_message_id;
size_t _num_messages = 0;

bool try_ack(const bool force) {
  static time_t _last_ack = time(NULL);

  if (!_num_messages) return false;
  bool ok = _num_messages >= 1024 || _last_ack < time(NULL) - 2 || force;
  if (!ok) return false;

  stomp::StompFrame *frame = new stomp::StompFrame("ACK");
  frame->add_header("subscription", _feed->sub_id() );
  frame->add_header("message-id", _last_message_id );
  size_t len = _feed->send_frame(frame);
  frame->release();

  time_t diff = time(NULL) - _last_ack;

  std::cout << "Sending ack for "
            << _num_messages
            << " message" << (_num_messages == 1 ? "" : "s")
            << " to " << _feed->dest_in()
            << " after " << diff << " second" << (diff == 1 ? "" : "s")
            << " " << (force ? "(force)" : "")
            << std::endl;

  _num_messages = 0;
  _last_ack = time(NULL);

  return true;
} // try_ack

int main(int argc, char **argv) {

  _feed = new stomp::Feed("10.0.1.3:61613", "feedtest", "feedtest", "/queue/feeds.*");
  _feed->set_connect_read_timeout(0);
  _feed->start();

  while(1) {
    bool didWork = false;
    size_t process_loops = 0;
    for(size_t i=0; _feed->process(); i++) ++process_loops;

    if (process_loops) std::cout << "Process loops " << process_loops << std::endl;

    size_t num_frames_in = 0;
    size_t num_bytes_in = 0;
    size_t frame_loops = 0;
    for(size_t i=0; i < 1024 || 1; i++) {
      stomp::StompFrame *frame;
      bool ok = _feed->next_frame(frame);
      if (!ok) break;
      if (i == 2) std::cout << "Over 10" << std::endl;

      didWork |= ok;
      if (frame->is_command(stomp::StompFrame::commandMessage) ) {
        _last_message_id = frame->get_header("message-id");
        std::string aprs_created_str = frame->get_header("APRS-Created");
        time_t aprs_created = atoi( aprs_created_str.c_str() );
        ++_num_messages;
      } // if

      ++frame_loops;
      frame->release();
    } // for

    if (frame_loops) std::cout << "Frame loops " << frame_loops << std::endl;

    try_ack(false);
    usleep(1000);
sleep(1);
  } // while

  exit(0);
} // main
