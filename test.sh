#!/bin/bash


declare -a programs=( "rust/target/release/mandelbrot"
                      "c/mandelbrot"
                      "python/mandelbrot.py" )
ARGS="2000x2000 -0.5,0.5 -0.3,0.7"

if [ $# -eq 3 ]; then
  ARGS="$1 $2 $3"
elif [ $# -ne 0 ]; then
  echo "Usage: $0 2000x2000 -2.0,2.0 2.0,-2.0"
  exit
fi

MAX_CORES=32
FILENAME="test_results.csv"

echo -n "Threads," > $FILENAME
CORES=1

while [ $CORES -le $MAX_CORES ]; do
  echo -n "$CORES," >>$FILENAME
  let CORES=CORES*2
done
echo >> $FILENAME

for p in "${programs[@]}"
do
  echo -n "$p,">> $FILENAME
  CORES=1
  while [ $CORES -le $MAX_CORES ]; do
    (/usr/bin/time -f "%e," "$p" `expr match "$p" '\(^[a-zA-Z0-9]*\)'`_mandel.png $ARGS "$CORES" NO) 2>&1 | tr -d '\n' >> $FILENAME
    let CORES=CORES*2
  done
  echo >> $FILENAME
done
