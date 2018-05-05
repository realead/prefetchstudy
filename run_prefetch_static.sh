set -e

TARGET=prefetch_static

cd src
g++ prefetch_static.cpp -O3 -std=c++11 -I../timeitcpp/include -o $TARGET
printf '.99' | ./$TARGET 

