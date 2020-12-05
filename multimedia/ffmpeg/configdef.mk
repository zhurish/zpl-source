CONFIG_OPTS =--prefix=$(DSTROOTFSDIR) \
--enable-static --enable-shared --disable-all --disable-programs --disable-doc --disable-htmlpages \
--disable-manpages --disable-podpages --disable-txtpages \
--enable-avdevice --enable-avcodec --enable-avformat --enable-swresample \
--enable-swscale --enable-postproc --enable-avfilter --enable-avresample \
--disable-pthreads --enable-network --enable-dct --enable-dwt \
--enable-lsp --disable-lzo --enable-mdct --enable-rdft --enable-fft \
--enable-faan --enable-pixelutils --disable-everything --disable-encoders \
--disable-decoders --disable-hwaccels --disable-muxers --disable-demuxers \
--disable-parsers --disable-bsfs --disable-protocols --disable-indevs \
--disable-outdevs --disable-filters
ifeq ($(PL_LIBX264_MODULE),true)
CONFIG_OPTS += -L/home/zhurish/workspace/SWPlatform/source/rootfs_install/lib -lx264
endif
ifeq ($(PL_OPENH264_MODULE),true)
EXTRALIBS-avcodec += -L/home/zhurish/workspace/SWPlatform/source/rootfs_install/lib -lopenh264
endif
--enable-encoder="flv,g723_1,h261,h263,h264_v4l2m2m,hevc_v4l2m2m,libgsm,libopencore_amrnb,libopenh264,\
libopenjpeg,libspeex,libx264,libx264rgb,libx265,mpeg4,pcm_alaw,yuv4,zlib" \
--enable-decoder="amrnb,amrwb,flv,g723_1,g729,gsm,h261,h263,h264,hevc,ilbc,libgsm,libopencore_amrnb,libopencore_amrwb,libopenh264,\
libopenjpeg,libspeex,libvpx_vp8,libvpx_vp9,mpeg4,mpegvideo,pcm_alaw,vp8,vp9,yuv4,zlib" \
--enable-hwaccel=
--enable-muxer="flv,g723_1,gsm,h261,h263,h264,mp4,mpeg1video,mpeg2video,pcm_alaw,rtp,rtsp" \
--enable-demuxer="flv,g723_1,gsm,h261,h263,h264,mp3,mpegvideo,pcm_alaw,rtp,rtsp" \
--enable-parser="flac,g723_1,gsm,h261,h263,h264,hevc,mpeg4video,mpegvideo,vp8,vp9" \
--enable-bsf="av1_frame_merge,av1_frame_split,av1_metadata,filter_units,h264_metadata,h264_mp4toannexb,\
h264_redundant_pps,hevc_metadata,hevc_mp4toannexb,mjpeg2jpeg,mpeg2_metadata,mpeg4_unpack_bframes,pcm_rechunk" \
--enable-protocol="hls,http,https,pipe,rtmp,rtp,srtp,unix,tcp,tls,udp" \
--enable-indev="alsa,dshow,fbdev,v4l2" \
--enable-outdev="alsa,opengl,fbdev,v4l2" \
--enable-filter="alsa,opengl,fbdev,v4l2" \
--enable-alsa \
--enable-gnutls \
--enable-libdav1d \
--enable-libdavs2 \
--enable-libgsm \
--enable-libilbc \
--enable-libopencore-amrnb \
--enable-libopencore-amrwb \
--enable-libopenh264 \
--enable-libopenjpeg \
--enable-libspeex \
--enable-libv4l2 \
--enable-libvpx \
--enable-libx264 \
--enable-libx265 \
--enable-opengl \
--enable-openssl \
--enable-zlib \
--enable-v4l2-m2m \
--arch=ARCH \      
--cpu=CPU  \                    
--cross-prefix=PREFIX \
--progs-suffix=SUFFIX \
--enable-cross-compile \   
--enable-asm \
--enable-power8 \
--enable-mmx \
--enable-mmxext \
--enable-sse \
--enable-sse2 \
--enable-sse3 \
--enable-ssse3 \
--enable-sse4 \
--enable-sse42 \
--enable-avx \
--enable-xop \
--enable-fma3 \
--enable-fma4 \
--enable-avx2 \
--enable-avx512 \
--enable-armv5te \
--enable-armv6 \
--enable-armv6t2 \
--enable-vfp  \
--enable-neon \
--enable-inline-asm \
--enable-x86asm \
--enable-mipsdsp \
--enable-mipsdspr2 \
--enable-mipsfpu 
