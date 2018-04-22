#!/bin/bash

TESTS="test_kos_multi_threaded_all_getAll_fromFile test_kos_multi_threaded_time"

c=0
for i in $TESTS
do
	(( c++ ))
	echo #########################################################
	echo Executing test $c: $i
	echo #########################################################
	./${i}
	echo ---------------------------------------------------------
	echo press a key to continue
	read a
done
