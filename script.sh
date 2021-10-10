#!/bin/sh
echo Enter the port number, on which the server is listening..
read port

for i in 1 2 3 4 5
do
   ./device localhost $port $i $i
done

# running in parallel
./device localhost $port 1 2 & ./device localhost $port 2 3  & ./device localhost $port 3 4  & ./device localhost $port 4 5 & ./device localhost $port 4 6
wait




