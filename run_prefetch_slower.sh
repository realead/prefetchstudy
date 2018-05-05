set -e

TARGET=prefetch_slower

cd src
g++ prefetch_slower.cpp -O3 -std=c++11 -I../timeitcpp/include -o $TARGET
printf '.99' | ./$TARGET 

