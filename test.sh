#!/bin/bash    

inext=".in"
outext=".out"
compareext=".compare"

for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22
do
    infile=$i$inext
    outfile=$i$outext
    compfile=$i$compareext
    ./dash test/xtratestcases/$infile > out 2>&1
    diff out test/xtratestcases/$outfile > $compfile
done