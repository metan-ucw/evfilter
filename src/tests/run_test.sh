#!/bin/sh

FAIL=0
PASS=0

function fail_test
{
	FAIL=$((FAIL+1))
	echo
	echo
	echo "$1 failed"
	echo
}

function pass_test
{
	PASS=$((PASS+1))
	echo -n "."
}

for i in *.json; do
	if [ -e "$i.in" ]; then
		IN="$i.in"
	else
		IN=""
	fi

	OUT="$(basename "$i" ".json").out"

	LD_LIBRARY_PATH="../" ./test $@ $i $IN

	if [ -f "$OUT" ]; then
		if ! diff out $OUT 2>&1 > /dev/null; then
			fail_test $i
			diff -u out $OUT
			echo
		else
			pass_test $i
		fi
	fi
done

rm -f out

echo
echo "FAILED $FAIL"
echo "PASSED $PASS"

if [ "$FAIL" -gt 0 ]; then
	exit 1
else
	exit 0
fi
