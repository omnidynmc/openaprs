#!/bin/bash
svn update &&
cd openframe &&
./configure &&
make install
