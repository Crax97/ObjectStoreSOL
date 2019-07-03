#!/bin/bash

exec 3<testout.log
PASSED=(0 0 0 0)
FAILED=(0 0 0 0)
WHOPASSED=()
WHOFAILED=()
while read -u 3 -a line; do
	TOT=$(($TOT + 1))
	TEST=$(echo ${line[@]} | cut -d ' ' -f 6)
	WHO="${line[1]}"
	RESULT=$(echo ${line[@]} | grep "OK")

	if [ $? -eq 0 ]; then
		PASSED[$TEST]=$((PASSED[$TEST] + 1))
		WHOPASSED+=($WHO)
	else
		FAILED[$TEST]=$((FAILED[$TEST] + 1))
		WHOFAILED+=($WHO)
	fi
done

PASSED[0]=$((PASSED[1] + PASSED[2] + PASSED[3]))
FAILED[0]=$((FAILED[1] + FAILED[2] + FAILED[3]))

printf "Format: Total\tType1\tType2\tType3\n"
echo Passed: "${PASSED[@]}"
echo Failed: "${FAILED[@]}"
echo by: ${WHOFAILED[@]}


kill -s USR1 $(pidof server.o)
