#!/bin/bash

rm results.txt
touch results.txt
for j in {1..3}
do
  echo "---------------------------------------------------" >> results.txt
  echo "Test : $j" >> results.txt
  echo "---------------------------------------------------" >> results.txt
  for i in {1..40}
  do
    start=`date +%s`
    ./cracker -t $i -c -o out.txt tests/test-input/01_4c_1k.bin
    end=`date +%s`
    runtime=$((end-start))
    echo "$i threads : time = $runtime" >> results.txt
  done
done