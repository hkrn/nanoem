#!/bin/bash

mkdir -p dependencies/ffmpeg/out/windows/vs2017/x86_64/release
cd dependencies/ffmpeg/out/windows/vs2017/x86_64/release
../../../../../configure \
        --prefix=$PWD/install-root \
        --arch=amd64 \
        --target-os=mingw32 \
        --cross-prefix="x86_64-w64-mingw32.static-" \
        --extra-cflags="-MD" \
        --extra-cxxflags="-MD" \
        --disable-debug \
        --disable-asm \
        --disable-static \
        --disable-doc \
        --disable-htmlpages \
        --disable-manpages \
        --disable-podpages \
        --disable-txtpages \
        --disable-bzlib \
        --disable-iconv \
        --disable-lzo \
        --disable-sdl2 \
        --disable-network \
        --disable-schannel \
        --disable-symver \
        --disable-xlib \
        --disable-zlib \
        --disable-securetransport \
        --disable-avdevice \
        --disable-avfilter \
        --disable-postproc \
        --disable-demuxers \
        --disable-muxers \
        --disable-decoders \
        --disable-encoders \
        --disable-bsfs \
        --disable-parsers \
        --disable-programs \
        --disable-hwaccels \
        --disable-filters \
        --disable-devices \
        --disable-protocols \
        --enable-shared \
        --enable-rpath \
        --enable-small \
        --enable-swresample \
        --enable-swscale \
        --enable-demuxer=avi,matroska \
        --enable-muxer=avi,matroska \
        --enable-decoder=flac,pcm_s16le,rawvideo,utvideo \
        --enable-encoder=flac,pcm_s16le,rawvideo,utvideo \
        --enable-protocol=file \
        --enable-cross-compile \

make
make install
