#!/bin/bash
LASTMEM=""

while [ 1 == 1 ] ; do
  PID=`pgrep openaprs`
  CMD="ps u -p $PID --no-headers"

  RET=`$CMD`
  #echo $RET
  MEM=`echo $RET | cut -f6 -d ' '`
  MEMPER=`echo $RET | cut -f4 -d ' '`
  DATE=`date`

  if [ "$MEM"  != "$LASTMEM" ]; then
    echo "$DATE: !!!WARNING!!! Memory changed: $MEMPER $MEM"
  fi

  LASTMEM="$MEM"
  sleep 2
done
