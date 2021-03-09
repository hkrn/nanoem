#!/bin/bash

CLANG_FORMAT_PATH=${CLANG_FORMAT_PATH:=clang-format}

# nanoem
fd --glob "*.{cc,h,m,mm}" \
  -E "dependencies" \
  -E "emapp/bundle" \
  -E "emapp/include/GL" \
  -E "emapp/include/emapp/private/shaders" \
  -E "emapp/plugins/obsolete/mojoshader" \
  -E "emapp/plugins/lsmash/thread.h" \
  -E "emapp/resources" \
  -E "emapp/src/ini.h" \
  -E "emapp/src/sha1.h" \
  -E "emapp/src/wildcardcmp.h" \
  -E "win32/ffmpeg" \
  -E "glfw/include/GL" \
  -E "nanoem" \
  -E "scripts" \
  -E "khash.h" \
  -E "*.pb-c.*" \
  $PWD \
  -x ${CLANG_FORMAT_PATH} -i

# fx9
fd --glob "*.{cc,h}" \
  -E "tokens.h" \
  -E "thirdparty/GL" \
  $PWD/dependencies/fx9 \
  -x ${CLANG_FORMAT_PATH} -i
