#!/bin/bash
# make
# for i in {1..100}
# do
#     sleep 0.01
#    ./test &
# done

for i in {1..100}
do
    sleep 0.01
   ./test &
done