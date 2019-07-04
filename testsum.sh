#!/bin/bash

exec 3<testout.log
PASSED=(0 0 0 0)
FAILED=(0 0 0 0)
WHOPASSED=()
WHOFAILED=()
while read -r -u 3 -a line; do
	TOT=$((TOT + 1))
	TEST=$(echo "${line[@]}" | cut -d ' ' -f 6)
	WHO="${line[1]}"

	if ! [ "$(echo "${line[@]}" | grep "OK")" == "" ]; then
		PASSED[$TEST]=$((PASSED[TEST] + 1))
		WHOPASSED+=("$WHO")
	else
		FAILED[$TEST]=$((FAILED[TEST] + 1))
		WHOFAILED+=("$WHO")
	fi
done

PASSED[0]=$((PASSED[1] + PASSED[2] + PASSED[3]))
FAILED[0]=$((FAILED[1] + FAILED[2] + FAILED[3]))

printf "Format: Total\tType1\tType2\tType3\n"
echo Passed: "${PASSED[@]}"
echo Failed: "${FAILED[@]}"
echo by: "${WHOFAILED[@]}"


killall -s USR1 server.o
