#!/bin/bash
#
# Run this script to generate keys.h from keyparser.in
#
# You should check key names and fix some errors by hand
# (for example Key_Www -> Key_WWW, Key_Leftctrl -> Key_LeftCtrl)
#

capitalize()
{
	len=`echo $1 | wc -c`
	word="$1"
	begin=1
	for((i=0;i<len;i++)) {
		if [ "${word:$i:1}" == "_" ]; then
			echo -n "_"
			begin=1
		else
			if [ $begin -eq 1 ]; then
				begin=0
				echo -n ${word:$i:1}
			else
				echo -n `echo -n ${word:$i:1} | tr [A-Z] [a-z]`
			fi
		fi
	}
}

KEYS=`cat keyparser.in | sort`

echo -en "#ifndef EVF_KEYS_H__\n#define EVF_KEYS_H__\n\n"
echo -en "#include <linux/input.h>\n\n"

echo "#define NO_KEY 0"
echo "static int key_values[] = {"

for i in $KEYS; do
	echo -e "\t$i,"
done
echo -e "};\n"

echo -n "static char *key_names[] = {"
for i in $KEYS; do
	echo -ne "\t\""
	echo -n `capitalize $i`
	echo "\","
done
echo -e "};\n"

echo -n "static const int key_count = "
echo -n `cat keyparser.in | wc -l`
echo ";"

echo -e "\n#endif /* EVF_KEYS_H__ */"
