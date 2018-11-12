#!/bin/sh

#define parameters which are passed to template
FILENAME=$1
ENGINEER=$2
DATE=$(date '+%d/%m/%Y %H:%M:%S');

#define the template
cat << EOF >$FILENAME
//=============================================================================
//== FILENAME:          "$FILENAME"
//== WRITTEN BY:        $ENGINEER
//== LAST MODIFIED:     $DATE
//=============================================================================

//===========================
//== SUBSECTION
//===========================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EOF
