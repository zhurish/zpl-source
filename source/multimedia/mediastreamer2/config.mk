#############################################################################
# DEFINE
###########################################################################
vpath %.c 		src/base
vpath %.c 		src/otherfilters
vpath %.c 		src/voip
vpath %.cpp 	src/voip
vpath %.c 		src/crypto
vpath %.c 		src/utils
vpath %.cpp 	src/utils/filter-wrapper
vpath %.c 		src/audiofilters
vpath %.c 		src/videofilters
vpath %.cpp 	src/videofilters
vpath %.c 		src/upnp
vpath %.cpp 	src/voip/h26x
vpath %.c	 	tools



#base
OBJS =	mscommon.o \
					msfilter.o \
					msqueue.o \
					msticker.o \
					eventqueue.o \
					mssndcard.o \
					msfactory.o \
					mswebcam.o \
					msvideopresets.o \
					mtu.o \
					msasync.o \

OBJS += 		msrtp.o \
					msudp.o \
					void.o \
					itc.o \
					tee.o \
					join.o \
					rfc4103_source.o \
					rfc4103_sink.o 

#ifeq ($(strip $(ZPL_MEDIASTREAM_SRTP)),true)
OBJS += ms_srtp.o
#endif

#ifeq ($(strip $(ZPL_MEDIASTREAM_ZRTP)),true)
OBJS += zrtp.o
#endif
#ifeq ($(strip $(ZPL_MEDIASTREAM_DTLS_SRTP)),true)
OBJS += dtls_srtp.o
#endif


OBJS+= alaw.o \
	audiomixer.o \
	chanadapt.o \
	devices.o \
	dtmfgen.o \
	equalizer.o \
	flowcontrol.o \
	g711.o \
	msfileplayer.o \
	msfilerec.o \
	asyncrw.o \
	msg722.o \
	msvaddtx.o \
	msvolume.o \
	tonedetector.o \
	ulaw.o \
	genericplc.o \
	msgenericplc.o
					

#OBJS+=	msspeex.o \ 
#OBJS+=    speexec.o \
#OBJS+=	gsm.o \
#OBJS+=	g726.o \
#OBJS+=	g729.o \
#OBJS+=	msresample.o 
ifeq ($(strip $(ZPL_MEDIASTREAM_ALSA)),true)
OBJS +=	alsa.o 
endif
#OBJS+=	qsa.o 
#OBJS+=	arts.o 
#OBJS+=	pasnd.o \
#OBJS+=	pulseaudio.o \


OBJS += bits_rw.o \
	audiodiff.o \
	box-plot.o \
	dsptools.o \
	g722_decode.o \
	g722_encode.o \
	kiss_fft.o \
	kiss_fftr.o \
	stream_regulator.o \
	decoding-filter-wrapper.o \
	encoding-filter-wrapper.o 

ifeq ($(strip $(ZPL_MEDIASTREAM_FFMPEG)),true)
OBJS += jpgloader-ffmpeg.o 
OBJS += ffmpeg-priv.o 
endif




OBJS +=	extdisplay.o \
		msanalysedisplay.o \
		mire.o \
		nowebcam.o \
		pixconv.o \
		sizeconv.o \
		videoswitcher.o \
		videorouter.o 	

ifeq ($(strip $(ZPL_MEDIASTREAM_V4L2)),true)
OBJS  += msv4l2.o 
endif

ifeq ($(strip $(ZPL_MEDIASTREAM_FFMPEG)),true)
OBJS += videodec.o \
		videoenc.o 
OBJS += ffmpegjpegwriter.o
endif


OBJS +=	h26x-encoder-filter.o 
OBJS +=	h26x-decoder-filter.o 
OBJS +=	h26x-utils.o 
OBJS +=	h264-nal-packer.o 
OBJS +=	h264-nal-unpacker.o 
OBJS +=	h264-utils.o 
OBJS +=	h265-nal-packer.o 
OBJS +=	h265-nal-unpacker.o 
OBJS +=	h265-utils.o 
OBJS +=	nal-packer.o 
OBJS +=	nal-unpacker.o 
OBJS +=	rfc3984.o 
OBJS +=	h264dec.o 



OBJS +=	 \
	audioconference.o \
	audiostream.o \
	bandwidthcontroller.o \
	bitratecontrol.o \
	bitratedriver.o \
	ice.o \
	mediastream.o \
	msiframerequestslimiter.o \
	msmediaplayer.o \
	msmediarecorder.o \
	msvoip.o \
	qosanalyzer.o \
	qualityindicator.o \
	rfc4103_textstream.o \
	ringstream.o \
	stun.o \
	offeranswer.o \
	audiostreamvolumes.o \
	turn_tcp.o \
	video-conference.o \
	video-conference-all-to-all.o \
	video-endpoint.o \
	layouts.o \
	msvideo.o \
	msvideoqualitycontroller.o \
	video_preset_high_fps.o \
	videostarter.o \
	videostream.o 

ifeq ($(strip $(ZPL_MEDIASTREAM_OPENH264)),true)
vpath %.cpp 	src/openh264
OBJS += msopenh264.o \
		msopenh264dec.o 
OBJS += msopenh264enc.o
endif

ifeq ($(strip $(ZPL_MEDIASTREAM_UPNP)),true)
OBJS+=	upnp_igd.o \
					upnp_igd_cmd.o \
					upnp_igd_utils.o 
endif


OBJS += common.o mediastreamtest.o mkvstream.o   test1.o
#videoh264file.o test1.o
#$(upnp_OBJS)

basedescs.h:	Makefile $(libmediastreamer_base_la_SOURCES)
	cd $(srcdir) && \
	awk 'BEGIN { FS="[()]" ; }; /^\t*MS_FILTER_DESC_EXPORT/{ printf("%s\n", $$2) } '  > $(abs_builddir)/basedescs.txt $(libmediastreamer_base_la_SOURCES) && \
	awk 'BEGIN { print("#include \"mediastreamer2/msfilter.h\"\n") } { printf("extern MSFilterDesc %s;\n",$$1) } ' $(abs_builddir)/basedescs.txt > $(abs_builddir)/$@ && \
	awk 'BEGIN { print("MSFilterDesc * ms_base_filter_descs[]={") } { printf("&%s,\n",$$1) } END{ print("NULL\n};\n") } ' $(abs_builddir)/basedescs.txt >> $(abs_builddir)/$@

voipdescs.h:	Makefile $(libmediastreamer_voip_la_SOURCES) $(libqtcapture_cocoa_la_SOURCES)
	cd $(srcdir) && \
	awk 'BEGIN { FS="[()]" ; }; /^\t*MS_FILTER_DESC_EXPORT/{ printf("%s\n", $$2) } '  > $(abs_builddir)/voipdescs.txt $(libmediastreamer_voip_la_SOURCES) $(libqtcapture_cocoa_la_SOURCES) && \
	awk 'BEGIN { print("#include \"mediastreamer2/msfilter.h\"\n") } { printf("extern MSFilterDesc %s;\n",$$1) } ' $(abs_builddir)/voipdescs.txt > $(abs_builddir)/$@ && \
	awk 'BEGIN { print("MSFilterDesc * ms_voip_filter_descs[]={") } { printf("&%s,\n",$$1) } END{ print("NULL\n};\n") } ' $(abs_builddir)/voipdescs.txt >> $(abs_builddir)/$@


#############################################################################
# LIB 		
###########################################################################
LIBS = libmediastream.a
