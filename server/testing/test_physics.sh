#!/bin/bash

cp ../physics.h .
g++ -Wall -Wextra -std=c++11 test_physics.cpp && ./a.out

if [ $? -ne 0 ]; then
  echo "failure"
  exit 1
else
  echo "success"
  rm physics.h a.out
fi
  
