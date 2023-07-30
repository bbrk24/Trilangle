#!/bin/bash

set -e

folder="$(dirname "$0")"

# Testing this has been weird. On some systems this works fine without --pipekill; on others it doesn't. For the
# sake of my sanity I'm not testing the behavior without the flag, so it may become a no-op in the future.
timeout 2s bash -c "trilangle --pipekill '${folder}/yes.trg' | head -n 10 >/dev/null"
