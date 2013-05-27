#!/bin/bash

ROTATE="./rotate"

function test() {

	echo "-----------------------------------------------------"
	echo "Testing $1"
	echo "-----------------------------------------------------"
	
	OUTFILE=`basename $1`
	rm -f $OUTFILE.enc $OUTFILE.dec

	
	$ROTATE -v -o $OUTFILE.enc $1
	$ROTATE -v -d -o $OUTFILE.dec $OUTFILE.enc


	ORG_MD5=`md5sum $1 | awk '{print $1}'`
	ENC_MD5=`md5sum $OUTFILE.enc | awk '{print $1}'`
	DEC_MD5=`md5sum $OUTFILE.dec | awk '{print $1}'`

	echo

	if [ "$DEC_MD5" == "$ORG_MD5" ]; then
		echo "SUCCESS :)"
	else
		echo "FAILED :("
	fi

	echo -ne "$1\t= $ORG_MD5\n"
	echo -ne "$OUTFILE.dec\t= $DEC_MD5\n"
	echo -ne "$OUTFILE.enc\t= $ENC_MD5\n"
	
	rm -f $OUTFILE.enc $OUTFILE.dec

	echo
}

function defaultTest() {
	test "binary.dat"
	test "ascii.txt"
}

if [ -e "$1" ]; then
	test "$1"
else
	defaultTest
fi
