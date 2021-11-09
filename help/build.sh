#!/bin/bash

## START STANDARD BUILD SCRIPT INCLUDE
# adjust relative paths as necessary
THIS_SCRIPT="$(greadlink -f "${BASH_SOURCE[0]}" 2>/dev/null || readlink -f "${BASH_SOURCE[0]}")"
## END STANDARD BUILD SCRIPT INCLUDE

QUIET=0

set -e
set -u

THIS_DIR="$(dirname "$THIS_SCRIPT")"

function die () {
    echo
    echo "$*"
    echo
    exit 1
}

function display_usage() {
  echo "build.sh [--no-clean]"
  echo "Builds help documentation for Speech Analyzer. Converts Markdown"
  echo "    documentation to html using pandoc and then builds .chm"
  echo "Targets:"
  echo " --no-clean: don't clean target folder before building"
}

# Verify pre-req programs installed
if ! command -v pandoc ; then
  die "Pandoc not installed"
fi

if [ ! -d "/c/program files (x86)/html help workshop/" ]; then
  die "HTML Help workshop not installed"
fi


DO_CHM=true
DO_CLEAN=true

# Debug flags
DO_CHM_CONVERSION=true

#
# Parse args
#

shopt -s nocasematch

while [[ $# -gt 0 ]] ; do
  key="$1"
  case $key in
    --no-clean)
      DO_CLEAN=false
      ;;
    *)
      display_usage
      exit 1
  esac
  shift # past argument
done

#
# Build toc.hhc
#

build_hhc_header() {
  echo '
<HTML>
  <HEAD>
    <meta http-equiv="Content-Type" content="text/html; charset=windows-1252" />
  </HEAD>
  <BODY>
    <OBJECT type="text/site properties">
      <param name="ImageType" value="Folder" />
    </OBJECT>
' > "$DESTCHM/toc.hhc"
}

build_hhc_footer() {
  echo '
</BODY>

</HTML>
' >> "$DESTCHM/toc.hhc"
}

build_hhc_entry() {
  local FILE="$1"
  local TITLE=$(grep '<title>' < "$FILE" | sed -r 's/.*>(.+)<\/.*/\1/' | iconv -f utf-8 -t windows-1252)
  if [ -z "$TITLE" ]; then
    TITLE="$1"
  fi

  echo '
    <LI><OBJECT type="text/sitemap">
        <param name="Name" value="'"$TITLE"'" />
        <param name="Local" value="'"$FILE"'" /></OBJECT></LI>
' >> "$DESTCHM/toc.hhc"
}

function build_hhc() {
  local TARGET_PATH="$1"

  if [ -f "$TARGET_PATH/index.htm" ]; then
    build_hhc_entry "$TARGET_PATH/index.htm"
  fi

  echo '
    <UL>
' >> "$DESTCHM/toc.hhc"

  for file in "$TARGET_PATH"/*; do
    if [ -d "$file" ]; then
      build_hhc "$file"
    elif [[ "$file" == */*.htm && "$file" != */index.htm ]]; then
      build_hhc_entry "$file"
    fi
  done

  echo '
    </UL>
' >> "$DESTCHM/toc.hhc"
}

#
# Compile all .md to .htm
#

MDLUA="./htm-link.lua"
CSS="./offline-help-style-spec.txt"
MD=`find -name "*.md"`
DESTCHM="$THIS_DIR/../chm/"

if $DO_CHM; then
  #
  # Clean existing folder
  #

  if $DO_CLEAN; then
    rm -rf "$DESTCHM" || true # We don't want to die when we clean an empty folder
  fi
  mkdir -p "$DESTCHM"

  #
  # Generate HTML files from Markdown
  #

  if $DO_CHM_CONVERSION; then
    for INFILE in $MD; do
      OUTFILE="$DESTCHM/${INFILE%.md}.htm"
      echo "Processing $INFILE to $(basename "$OUTFILE")"
      mkdir -p "$(dirname "$OUTFILE")"
      pandoc -s -H "$CSS" --lua-filter="$MDLUA" -t html -o "$OUTFILE" $INFILE
    done
  fi

  #
  # Copy Images
  #

  mkdir -p "$DESTCHM/images"
  cp "$THIS_DIR"/images/* "$DESTCHM/images/"

  #
  # Prepare TOC and HHP files
  #

  pushd "$DESTCHM" > /dev/null

  cp "$THIS_DIR/speechanalyzer.hhk" "$DESTCHM/speechanalyzer.hhk"
  cp "$THIS_DIR/speechanalyzer.hhp" "$DESTCHM/speechanalyzer.hhp"
  find -name '*.htm' >> "$DESTCHM/speechanalyzer.hhp"

  build_hhc_header
  build_hhc .
  build_hhc_footer

  # hhc.exe returns 1 on success!
  "/c/program files (x86)/html help workshop/hhc.exe" speechanalyzer.hhp && false || true

  # Copy chm to DistFiles/
  cp Speech_Analyzer_Help.chm "$THIS_DIR/../DistFiles/"

  popd > /dev/null
fi
