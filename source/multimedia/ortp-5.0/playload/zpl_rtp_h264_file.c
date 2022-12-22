/*
 * zpl_rtp_h264_channel.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */



#include "zpl_rtsp.h"
#include "zpl_rtsp_util.h"
#include "zpl_rtsp_transport.h"
#include "zpl_rtsp_sdp.h"
#include "zpl_rtsp_sdpfmtp.h"
#include "zpl_rtsp_base64.h"
#include "zpl_rtsp_auth.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_client.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_adap.h"
#include "zpl_rtsp_rtp.h"
#include "zpl_rtp_h264_file.h"

#ifndef ZPL_BUILD_LINUX
static unsigned rtsp_rtp_h264_get_profileLevelId(uint8_t *to, unsigned toMaxSize,
                                uint8_t const *from, unsigned fromSize)
{
	unsigned toSize = 0;
	unsigned i = 0;
	while (i < fromSize && toSize + 1 < toMaxSize)
	{
		if (i + 2 < fromSize && from[i] == 0 && from[i + 1] == 0 && from[i + 2] == 3)
		{
			to[toSize] = to[toSize + 1] = 0;
			toSize += 2;
			i += 3;
		}
		else
		{
			to[toSize] = from[i];
			toSize += 1;
			i += 1;
		}
	}
	return toSize;
}
#endif

zpl_rtp_h264_file_t *zpl_rtp_h264_file_create()
{
	zpl_rtp_h264_file_t *fil = malloc(sizeof(zpl_rtp_h264_file_t));
	if (fil)
	{
		memset(fil, 0, sizeof(zpl_rtp_h264_file_t));
	}
	return fil;
}

int zpl_rtp_h264_file_destroy(zpl_rtp_h264_file_t *filemedia)
{
	if (filemedia)
	{
		if (filemedia->fp)
		{
			fclose(filemedia->fp);
			filemedia->fp = NULL;
		}
	}
	return 0;
}

static int zpl_rtp_h264_file_find_nalu(FILE *fp, int *nalsize)
{
	uint8_t c = 0;
	int nal = 0;
	int off = 0;
	while (!feof(fp))
	{
		if (fread(&c, 1, 1, fp) == 1)
		{
			off++;
			if (c == 0)
				nal++;
			else if (c == 1)
			{
				if (nal == 2)
					break;
				else if (nal == 3)
					break;
			}
			else
			{
				nal = 0;
			}
		}
	}
	if (nalsize)
		*nalsize = nal;
	return off - nal;
}

static int zpl_rtp_h264_file_read_nalu_size(FILE *fp)
{
	int nalsize = 0;
	int s = zpl_rtp_h264_file_find_nalu(fp, &nalsize);
	if (s > 0)
		return s;
	s += zpl_rtp_h264_file_find_nalu(fp, &nalsize);
	return s;
}

int zpl_rtp_h264_file_get_nalu(char *filename, uint8_t *vps, uint8_t *pps, uint8_t *sps, 
		uint8_t *sei, uint32_t *profileid)
{
	fprintf(stdout, "===================zpl_rtp_h264_file_get_nalu======================\n");
	fflush(stdout);
	FILE *fp = fopen(filename, "r");
	if (fp)
	{
		zpl_rtp_h264_file_t file;
        H264_NALU_T nalu;
        uint8_t *bufdata = NULL;
		int mosiz = 0;
		memset(&file, 0, sizeof(zpl_rtp_h264_file_t));
		while (!feof(fp))
		{
			fseek(fp, file.file_ofsset, SEEK_SET);
			int rc = zpl_rtp_h264_file_read_nalu_size(fp);
			if (rc)
			{
				if(bufdata == NULL)
                    bufdata = (uint8_t*)malloc(rc);
				else
				{
					if(rc > mosiz && mosiz)
                        bufdata = (uint8_t*)realloc(bufdata, rc);
					mosiz = mosiz > rc ? mosiz:rc;
				}
					
				if (bufdata != NULL)
				{
					fseek(fp, file.file_ofsset, SEEK_SET);
					int len = fread(bufdata, 1, rc, fp);
					file.file_ofsset += len;

					file.file_len -= file.file_ofsset;
                    memset(&nalu, 0, sizeof(H264_NALU_T));

					if (bufdata[0] == 0 && bufdata[1] == 0 && bufdata[2] == 0 && bufdata[3] == 1)
					{
						nalu.hdr_len = 4;
						nalu.buf = bufdata;
						nalu.len = len - nalu.hdr_len;
						//memcpy (nalu.buf, &Buf[nalu.startcodeprefix_len], nalu.len);
						nalu.forbidden_bit = bufdata[nalu.hdr_len] & 0x80;	 //1 bit
						nalu.nal_idc = bufdata[nalu.hdr_len] & 0x60;		 // 2 bit
						nalu.nal_unit_type = (bufdata[nalu.hdr_len]) & 0x1f; // 5 bit
					}
					else if (bufdata[0] == 0 && bufdata[1] == 0 && bufdata[2] == 1)
					{
						nalu.hdr_len = 3;
						nalu.buf = bufdata;
						nalu.len = len - nalu.hdr_len;
						//memcpy (nalu.buf, &Buf[nalu.startcodeprefix_len], nalu.len);
						nalu.forbidden_bit = bufdata[nalu.hdr_len] & 0x80;	 //1 bit
						nalu.nal_idc = bufdata[nalu.hdr_len] & 0x60;		 // 2 bit
						nalu.nal_unit_type = (bufdata[nalu.hdr_len]) & 0x1f; // 5 bit
					}
					if (nalu.nal_unit_type)
					{
						char type_str[20] = {0};
						switch (nalu.nal_unit_type)
						{
                        case NALU_TYPE_SLICE:
							sprintf(type_str, "SLICE");
							break;
                        case NALU_TYPE_DPA:
							sprintf(type_str, "DPA");
							break;
                        case NALU_TYPE_DPB:
							sprintf(type_str, "DPB");
							break;
                        case NALU_TYPE_DPC:
							sprintf(type_str, "DPC");
							break;
                        case NALU_TYPE_IDR:
							sprintf(type_str, "IDR");
							break;
                        case NALU_TYPE_SEI:
							sprintf(type_str, "SEI");
							if(sei)
                            ;//	memcpy(sei, nalu.buf + nalu.hdr_len, nalu.len);
							//extradata->fPPSSize = nalu.len;
							break;
                        case NALU_TYPE_SPS:
							if (sps != NULL)
							{
                                if (rtsp_rtp_h264_get_profileLevelId(nalu.buf + nalu.hdr_len, nalu.len,
																		 sps, nalu.len) < 4)
								{
								}
								else
								{
									*profileid = (sps[1] << 16) | (sps[2] << 8) | sps[3];
									//extradata->profileLevelId = (extradata->fSPS[1] << 16) | (extradata->fSPS[2] << 8) | extradata->fSPS[3];
									memcpy(sps, nalu.buf + nalu.hdr_len, nalu.len);
									//extradata->fSPSSize = nalu.len;
								}
							}
							sprintf(type_str, "SPS");
							break;
                        case NALU_TYPE_PPS:
							if(pps)
								memcpy(pps, nalu.buf + nalu.hdr_len, nalu.len);
							//extradata->fPPSSize = nalu.len;
							sprintf(type_str, "PPS");
							break;
                        case NALU_TYPE_AUD:
							sprintf(type_str, "AUD");
							break;
                        case NALU_TYPE_EOSEQ:
							sprintf(type_str, "EOSEQ");
							break;
                        case NALU_TYPE_EOSTREAM:
							sprintf(type_str, "EOSTREAM");
							break;
                        case NALU_TYPE_FILL:
							sprintf(type_str, "FILL");
							break;
						}
						char idc_str[20] = {0};
						switch (nalu.nal_idc >> 5)
						{
						case NALU_PRIORITY_DISPOSABLE:
							sprintf(idc_str, "DISPOS");
							break;
                        case NALU_PRIORITY_LOW:
							sprintf(idc_str, "LOW");
							break;
						case NALU_PRIORITY_HIGH:
							sprintf(idc_str, "HIGH");
							break;
						case NALU_PRIORITY_HIGHEST:
							sprintf(idc_str, "HIGHEST");
							break;
						}
						fprintf(stdout, "-------- NALU Table ------+---------+\n");
						fprintf(stdout, "       IDC |  TYPE |   LEN   |\n");
						fprintf(stdout, "---------+--------+-------+---------+\n");
						fprintf(stdout, " %7s| %6s| %8d|\n", idc_str, type_str, nalu.len);
						fflush(stdout);
					}
				}
			}
		}
		if(bufdata)
			free(bufdata);
	}
	else
	{
		fprintf(stdout, "fopen '%s' error:%s\n", filename, strerror(errno));
		fflush(stdout);
	}
	return 0;
}

static int zpl_rtp_h264_file_read_one_frame(FILE *fp, uint8_t *data, int len)
{
	int ret = fread(data, 1, len, fp);
	return ret;
}

int zpl_rtp_h264_file_open(zpl_rtp_h264_file_t *filemedia, char *filename)
{
	if (filemedia->fp == NULL)
	{
		filemedia->fp = fopen(filename, "r");
                //filemedia->file_len = os_file_size(filename);
	}
	if (filemedia->fp)
		return 0;
	return -1;
}

int zpl_rtp_h264_file_read_thread(zpl_rtp_h264_file_t *chn)
{
	zpl_rtp_h264_file_t *file = chn;
    H264_NALU_T nalu;

	while (file->fp && file->file_len && !feof(file->fp))
	{
		fseek(file->fp, file->file_ofsset, SEEK_SET);
		int rc = zpl_rtp_h264_file_read_nalu_size(file->fp);
		if (rc)
		{
			file->t_read = NULL;
            uint8_t *bufdata = (uint8_t*)malloc(rc);
			if (bufdata != NULL)
			{
				fseek(file->fp, file->file_ofsset, SEEK_SET);
				int len = zpl_rtp_h264_file_read_one_frame(file->fp, bufdata, rc);
				file->file_ofsset += len;

				file->file_len -= file->file_ofsset;
                memset(&nalu, 0, sizeof(H264_NALU_T));
				//zpl_rtp_h264_channel_get_nalusplit(bufdata, rc, &nalu);

				//rtsp_rtp_send_h264(chn->buffer_queue, bufdata);
                //file->t_read = thread_add_timer(file->t_master, zpl_rtp_h264_file_read_thread, chn, file->msec);
				return 0;
			}
		}
	}
	return -1;
}

int zpl_rtp_h264_file_read_start(zpl_rtp_h264_file_t *chn, int msec)
{
	zpl_rtp_h264_file_t *file = chn;
	if (!file->t_master)
        ;//file->t_master = thread_master_module_create(0);
	file->msec = msec;
    //file->t_read = thread_add_timer(file->t_master, zpl_rtp_h264_file_read_thread, chn, msec);
	return 0;
}

int zpl_rtp_h264_file_read_stop(zpl_rtp_h264_file_t *chn)
{
	zpl_rtp_h264_file_t *file = chn;
	if (file->t_read)
	{
        //thread_cancel(file->t_read);
		file->t_read = NULL;
	}
	return 0;
}
