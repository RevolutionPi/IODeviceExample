#!/bin/bash
if [ $# -lt 1 ]
then
  echo "Use: "$0" <file_name>"
  echo "Convert files from WINDOWS-1252 to UTF8"
  exit
fi

for i in $*
do
  if [ ! -f $i ]; then # Only convert text files
    continue
  fi
  # Generate temp file to avoid Bus error
  iconv -f WINDOWS-1252 -t UTF-8 $i -o $i.tmp
  mv $i.tmp $i
done


# source:
# https://myotragusbalearicus.wordpress.com/2010/03/10/batch-convert-files-to-utf-8/
