#!/bin/sh
make
rm -f tmp
for i in test??
do echo -n "Running $i: "
   ./$i > tmp
   cmp -s tmp out/$i
   if [ "$?" -eq 0 ]
   then echo OK
   else echo FAIL
   fi
done
rm -f tmp
exit 0   
