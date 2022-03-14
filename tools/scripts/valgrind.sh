#!/bin/bash
make install && valgrind --tool=memcheck --leak-check=yes --undef-value-errors=no --log-file=valgrind.log ./openaprs -d -c etc/debug.conf
