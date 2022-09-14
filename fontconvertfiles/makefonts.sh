#!/bin/bash

# Ugly little Bash script, generates a set of .h files for GFX using
# GNU FreeFont sources.  There are three fonts: 'Mono' (Courier-like),
# 'Sans' (Helvetica-like) and 'Serif' (Times-like); four styles: regular,
# bold, oblique or italic, and bold+oblique or bold+italic; and four
# sizes: 9, 12, 18 and 24 point.  No real error checking or anything,
# this just powers through all the combinations, calling the fontconvert
# utility and redirecting the output to a .h file for each combo.

# Adafruit_GFX repository does not include the source outline fonts
# (huge zipfile, different license) but they're easily acquired:
# http://savannah.gnu.org/projects/freefont/

convert=./fontconvert
inpath=freefont-ttf/sfd/
outpath=Fonts/
fonts=(FreeMono FreeSans FreeSerif)
styles=("" Bold Italic BoldItalic Oblique BoldOblique)
sizes=(9 12 14 16 18 24 28 30 32 36 42 44 46 48 64)

for f in ${fonts[*]}
do
	for index in ${!styles[*]}
	do
		st=${styles[$index]}
		for si in ${sizes[*]}
		do
			infile=$inpath$f$st".ttf"
			outfile=$outpath$f$st$si"pt7b.h"
			if [ -f $infile ] # Does source combination exist?
			  then
				printf "Convert %s %s %s > %s\n" $convert $infile $si $outfile
				$convert $infile $si > $outfile
			  else
				printf "Skipping: %s %s  %s\n" $f $st $si
			fi
		done
	done
done
