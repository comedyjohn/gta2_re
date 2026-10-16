#!/bin/sh
# GTA2Hax (a submodule) includes its own headers with Windows path separators and case, e.g.
# "..\Common\Types.hpp". GCC takes the backslashes literally, so this writes one forwarding header
# per such name into $1, which goes on the include path. Nothing in the submodule changes.
set -e
out=${1:?usage: gen_forwarders.sh <outdir>}
hax="$(cd "$(dirname "$0")/../../3rdParty/GTA2Hax" && pwd)"
mkdir -p "$out"
fwd() { printf '#pragma once\n#include "%s"\n' "$hax/$2" > "$out/$1"; }
fwd '..\Common\Types.hpp' Common/types.hpp
# ddraw_vc6.h tests _MSC_VER and WINVER, which are not defined here (-Wundef)
printf '#pragma once\n#pragma GCC diagnostic push\n#pragma GCC diagnostic ignored "-Wundef"\n#pragma GCC diagnostic ignored "-Wextra-semi"\n#include "%s"\n#pragma GCC diagnostic pop\n' \
    "$hax/Common/ddraw_vc6.h" > "$out/..\\Common\\ddraw_vc6.h"
fwd '..\d3ddll\d3ddll.hpp' d3ddll/d3ddll.hpp
# GTA2Hax's structure size checks count 4-byte pointers; a 64-bit build skips them
cat >> "$out/..\\Common\\Types.hpp" <<'END'
#if __SIZEOF_POINTER__ == 8
    #undef STATIC_ASSERT
    #define STATIC_ASSERT(x, msg)
#endif
END
