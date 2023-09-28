#!/bin/bash

set -eu

folder=$(dirname "$0")

# https://stackoverflow.com/a/35512328/6253337
# Because macOS doesn't have timeout for some reason
if ! which timeout &>/dev/null
then
    timeout () {
        perl -e 'alarm shift; exec @ARGV' "$@";
    }
fi

# Testing this has been weird. On some systems this works fine without --pipekill; on others it doesn't. For the
# sake of my sanity I'm not testing the behavior without the flag, so it may become a no-op in the future.
timeout 2s bash -c "$TRILANGLE -af '${folder}/yes.trg' | head -n 10 >/dev/null"
