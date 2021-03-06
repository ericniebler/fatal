#!/bin/sh

set -e

if [ -z "$1" ]; then
  echo "Usage: $0 source_file"
  exit 1
fi

if [ -z "$USE_STD" ]; then
  export USE_STD="c++11"
fi

if [ -z "$USE_CC" ]; then
  export USE_CC="clang++-3.8"
fi

file_name="$1"
shift
base_dir="/tmp/_build/$USE_CC/$USE_STD"
out_dir="$base_dir/`dirname "$file_name"`"
link_dir="/tmp/_bin"

if [ ! -d "$out_dir" ]; then
  mkdir -p "$out_dir"
fi

if [ -e "$link_dir" ]; then
  rm -rf "$link_dir"
fi
ln -s "$base_dir" "$link_dir"

out_binary="$out_dir/`basename "$file_name" .cpp`"

if [ "$NO_CLEAR" != "true" ]; then
  ./lclear.sh >&2
fi

if [ -z "$CC_OPT" ]; then
  CC_OPT="-O2"
fi

CC_ARGS="-o $out_binary $CC_ARGS $CC_OPT -g -pthread"
if [ "$PRE_PROC" = "true" ]; then
  CC_ARGS="-E"
fi

echo -n "started: "; date
set -x
"$USE_CC" $CC_ARGS -Wall -Werror "-std=$USE_STD" -I . "$file_name" 2>&1
set +x
echo -n "finished: "; date

if [ "$DO_RUN" = "true" ]; then
  "$out_binary" "$@"
fi
