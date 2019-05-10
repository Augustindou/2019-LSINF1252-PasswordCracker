#!/bin/bash

touch results.txt
for i in 1 2 .. 40
do
  start=`date +%s`
  ./cracker -t $i -c -o out.txt tests/test-input/01_4c_1k.bin
  end=`date +%s`
  runtime=$((end-start))
  echo "$i : $runtime [s]" >> results.txt
done