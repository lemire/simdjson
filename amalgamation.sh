#!/bin/bash
########################################################################
# Generates an "amalgamation build" for roaring. Inspired by similar
# script used by whefs.
########################################################################
SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

echo "We are about to amalgamate all simdjson files into one source file. "
echo "See https://www.sqlite.org/amalgamation.html and https://en.wikipedia.org/wiki/Single_Compilation_Unit for rationale. "

AMAL_H="simdjson.h"
AMAL_C="simdjson.cpp"

# order does not matter
ALLCFILES="
$SCRIPTPATH/src/simdjson.cpp
$SCRIPTPATH/src/jsonioutil.cpp
$SCRIPTPATH/src/jsonminifier.cpp
$SCRIPTPATH/src/jsonparser.cpp
$SCRIPTPATH/src/stage1_find_marks.cpp
$SCRIPTPATH/src/stage2_build_tape.cpp
$SCRIPTPATH/src/parsedjson.cpp
$SCRIPTPATH/src/parsedjsoniterator.cpp
"

# order matters
ALLCHEADERS="
$SCRIPTPATH/include/simdjson/simdjson_version.h
$SCRIPTPATH/include/simdjson/portability.h
$SCRIPTPATH/include/simdjson/isadetection.h
$SCRIPTPATH/include/simdjson/simdjson.h
$SCRIPTPATH/include/simdjson/common_defs.h
$SCRIPTPATH/include/simdjson/padded_string.h
$SCRIPTPATH/include/simdjson/jsoncharutils.h
$SCRIPTPATH/include/simdjson/jsonformatutils.h
$SCRIPTPATH/include/simdjson/jsonioutil.h
$SCRIPTPATH/include/simdjson/simdprune_tables.h
$SCRIPTPATH/include/simdjson/simdutf8check_haswell.h
$SCRIPTPATH/include/simdjson/simdutf8check_westmere.h
$SCRIPTPATH/include/simdjson/simdutf8check_arm64.h
$SCRIPTPATH/include/simdjson/jsonminifier.h
$SCRIPTPATH/include/simdjson/parsedjson.h
$SCRIPTPATH/include/simdjson/stage1_find_marks.h
$SCRIPTPATH/include/simdjson/stage1_find_marks_flatten.h
$SCRIPTPATH/include/simdjson/stage1_find_marks_flatten_haswell.h
$SCRIPTPATH/include/simdjson/stage1_find_marks_macros.h
$SCRIPTPATH/include/simdjson/stage1_find_marks_westmere.h
$SCRIPTPATH/include/simdjson/stage1_find_marks_haswell.h
$SCRIPTPATH/include/simdjson/stage1_find_marks_arm64.h
$SCRIPTPATH/include/simdjson/stringparsing.h
$SCRIPTPATH/include/simdjson/stringparsing_westmere.h
$SCRIPTPATH/include/simdjson/stringparsing_haswell.h
$SCRIPTPATH/include/simdjson/stringparsing_arm64.h
$SCRIPTPATH/include/simdjson/numberparsing.h
$SCRIPTPATH/include/simdjson/stage2_build_tape.h
$SCRIPTPATH/include/simdjson/jsonparser.h
"

for i in ${ALLCHEADERS} ${ALLCFILES}; do
    test -e $i && continue
    echo "FATAL: source file [$i] not found."
    exit 127
done

function dofile()
{
    # Last lines are always ignored. Files should end by an empty lines.
    RELFILE=${1#"$SCRIPTPATH/"}
    echo "/* begin file $RELFILE */"
    # echo "#line 8 \"$1\"" ## redefining the line/file is not nearly as useful as it sounds for debugging. It breaks IDEs.
    while IFS= read -r line
    do
        if [[ "${line}" == '#include "simdjson'* ]]; then
            # we paste the contents of simdjson header files with names ending by _common.h
            # we ignore every other simdjson headers
            if [[ "${line}" == '#include "simdjson/'*'_common.h"'* ]]; then
              file=$(echo $line| cut -d'"' -f 2)
              echo "$(<include/$file)" # we assume those files are always in include/
            fi
        else
            # Otherwise we simply copy the line
            echo "$line"
        fi;
    done < "$1"
    echo "/* end file $RELFILE */"
}
timestamp=$(date)
echo "Creating ${AMAL_H}..."
echo "/* auto-generated on ${timestamp}. Do not edit! */" > "${AMAL_H}"
{
    for h in ${ALLCHEADERS}; do
        dofile $h
    done
} >> "${AMAL_H}"


echo "Creating ${AMAL_C}..."
echo "/* auto-generated on ${timestamp}. Do not edit! */" > "${AMAL_C}"
{
    echo "#include \"${AMAL_H}\""

    echo ""
    echo "/* used for http://dmalloc.com/ Dmalloc - Debug Malloc Library */"
    echo "#ifdef DMALLOC"
    echo "#include \"dmalloc.h\""
    echo "#endif"
    echo ""

    for h in ${ALLCFILES}; do
        dofile $h
    done
} >> "${AMAL_C}"



DEMOCPP="amalgamation_demo.cpp"
echo "Creating ${DEMOCPP}..."
echo "/* auto-generated on ${timestamp}. Do not edit! */" > "${DEMOCPP}"
cat <<< '
#include <iostream>
#include "simdjson.h"
#include "simdjson.cpp"
int main(int argc, char *argv[]) {
  if(argc < 2) {
    std::cerr << "Please specify a filename " << std::endl;
  }
  const char * filename = argv[1];
  simdjson::padded_string p = simdjson::get_corpus(filename);
  simdjson::ParsedJson pj = simdjson::build_parsed_json(p); // do the parsing
  if( ! pj.is_valid() ) {
    std::cout << "not valid" << std::endl;
  } else {
    std::cout << "valid" << std::endl;
  }
  return EXIT_SUCCESS;
}
' >>  "${DEMOCPP}"

echo "Done with all files generation. "

echo "Files have been written to  directory: $PWD "
ls -la ${AMAL_C} ${AMAL_H}  ${DEMOCPP}

echo "Giving final instructions:"


CPPBIN=${DEMOCPP%%.*}

echo "Try :"
echo "c++ -O3 -std=c++17 -o ${CPPBIN} ${DEMOCPP}  && ./${CPPBIN} ../jsonexamples/twitter.json "

SINGLEHDR=$SCRIPTPATH/singleheader
echo "Copying files to $SCRIPTPATH/singleheader "
mkdir -p $SINGLEHDR
echo "c++ -O3 -std=c++17 -o ${CPPBIN} ${DEMOCPP}  && ./${CPPBIN} ../jsonexamples/twitter.json " > $SINGLEHDR/README.md
cp ${AMAL_C} ${AMAL_H}  ${DEMOCPP} $SINGLEHDR
ls $SINGLEHDR

cd $SINGLEHDR && c++ -O3 -std=c++17 -o ${CPPBIN} ${DEMOCPP}  && ./${CPPBIN} ../jsonexamples/twitter.json

lowercase(){
    echo "$1" | tr 'A-Z' 'a-z'
}

OS=`lowercase \`uname\``
