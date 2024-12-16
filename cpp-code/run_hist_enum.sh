g++ -I ~/Downloads/eigen-3.4.0/eigen-3.4.0/ -O3 -std=c++17 hist_enum.cpp -o hist_enum
TIMEFORMAT=$'\nExecution time: %Rs'
time ./hist_enum