#!/bin/sh

echo Compiling
make -j4 app

if [ $? -eq 0 ]
then
  echo "Successfully created file"
else
  echo "Could not create file"
  exit 2
fi


echo Increasing build number
./buildnumber --header "main/HAP/HAPBuildnumber.hpp"
make -j4 app

echo Flash
make flash 

echo Uploading
#scp build/Homekit.bin pi@pi3:/home/pi/dev/HAPUpdateServer/

echo Start monitor
make monitor
