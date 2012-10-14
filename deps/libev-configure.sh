#!/bin/sh
set -e
cd "$(dirname "$0")/libev"
#LIBEV_DIR=$(pwd)
./configure --enable-shared=no --enable-static=yes
# cd ../../  # now in ./
# mkdir -p src/ev
# for srcfile in \
#   ev.h ev.c ev_vars.h ev_wrap.h \
#   ev_select.c ev_poll.c ev_kqueue.c ev_epoll.c ev_port.c ev_win32.c
# do
#   cp -f "$LIBEV_DIR/$srcfile" src/ev/$srcfile
# done
