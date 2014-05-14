#!/bin/sh

make clean
make

if [ -e ./sarlkm.ko ]; then
  cp ./sarlkm.ko /dev/shm
  sudo rmmod sarlkm
  sudo insmod /dev/shm/sarlkm.ko
fi

#tail -n 5 /var/log/messages