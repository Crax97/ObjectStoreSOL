#! /bin/bash
FAILED_COUNT_1=$(cat testout.log | grep "FAIL" | grep "type 1" | wc -l)
FAILED_COUNT_2=$(cat testout.log | grep "FAIL" | grep "type 2" | wc -l)
FAILED_COUNT_3=$(cat testout.log | grep "FAIL" | grep "type 3" | wc -l)
TYPE1_COUNT=$(cat testout.log | grep "type 1" | wc -l)
TYPE2_COUNT=$(cat testout.log | grep "type 2" | wc -l)
TYPE3_COUNT=$(cat testout.log | grep "type 3" | wc -l)

echo "Tests run in total: $(($FAILED_COUNT_1 + $TYPE1_COUNT + $TYPE2_COUNT + $TYPE3_COUNT))"
echo "Tests failed: $FAILED_COUNT_1"
echo "Type 1 tests: $TYPE1_COUNT"
echo "Type 2 tests: $TYPE2_COUNT"
echo "Type 3 tests: $TYPE3_COUNT"
