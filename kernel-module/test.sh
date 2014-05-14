#!/bin/bash

make clean
make

SUM=0;
P=0

if [ -e ./sarlkm.ko ]; then

cp ./sarlkm.ko /dev/shm

sudo rmmod sarlkm
sudo insmod /dev/shm/sarlkm.ko

echo -n "Test 1 (LKM): "
if [ -f /dev/shm/sarlkm.ko ]; then
  let SUM=SUM+1
  echo "OK."
else
  echo "failed."
  echo "No LKM -> 0 points"
  exit 0;
fi

echo -n "Test 2 (Procfile): "
if [ -f /proc/sarlkm ]; then
  let SUM=SUM+1
  let P=P+2
  echo "OK."
else
  echo "failed."
fi

echo -n "Test 3 (sysfsfile): "
if [ -f /proc/sys/kernel/prompt ]; then
  let SUM=SUM+1
  let P=P+2
  echo "OK."
else
  echo "failed."
fi

sysctl -w kernel.prompt=test4 > /dev/null
T1=`cat /proc/sarlkm | sed "s#,# #g" | awk '{print $1}'`

echo -n "Test 4 (Write Sysctl, Read Proc): "
if [ "x$T1" = "xtest4" ]; then
  let SUM=SUM+1
  let P=P+2
  echo "OK."
else
  echo "failed."
fi

echo "test5" > /proc/sarlkm
T1=`sysctl kernel.prompt | awk '{print $3}'`

echo -n "Test 5 (Write Proc, Read Sysctl): "
if [ "x$T1" = "xtest5" ]; then
  let SUM=SUM+1
  let P=P+2
  echo "OK."
else
  echo "failed."
fi

sudo rmmod sarlkm
sudo insmod /dev/shm/sarlkm.ko prompt_param=test6

echo -n "Test 6 (LKM params): "
T1=`sysctl kernel.prompt | awk '{print $3}'`
if [ "x$T1" = "xtest6" ]; then
  let SUM=SUM+1
  let P=P+1
  echo "OK."
else
  echo "failed."
fi

fi

make tools

if [ -e ./hostnamesysctl ]; then

cp ./hostnamesysctl /dev/shm

HN=`cat /proc/sys/kernel/hostname`
HN_SYS=`/dev/shm/hostnamesysctl`
echo -n "Test 7 (sysctl hostname(read)): "
if [ "x$HN" = "x$HN_SYS" ]; then
  let SUM=SUM+1
  READHN=1
  echo "OK."
else
  READHN=0
  echo "failed."
fi

HNOLD=`cat /proc/sys/kernel/hostname`
HN_SYS=`sudo /dev/shm/hostnamesysctl foobar`
HN=`cat /proc/sys/kernel/hostname`
echo -n "Test 8 (sysctl hostname(write)): "
if [ "x$HN" = "xfoobar" ]; then
  let SUM=SUM+1
  if [ $READHN -eq 1 ]; then
    let P=P+1
  fi
  echo "OK."
else
  echo "failed."
fi

sudo sysctl -w kernel.hostname=$HNOLD > /dev/null 2>&1

fi

echo ""
echo "*******************  RESULT  ***************************"
echo "$SUM of 8 Tests are OK. Points: $P of 10"

