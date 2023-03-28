// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Pengutronix, Michael Tretter <kernel@pengutronix.de>
 *
 * Convert NAL units between raw byte sequence payloads (RBSP) and C structs
 *
 * The conversion is defined in "ITU-T Rec. H.264 (04/2017) Advanced video
 * coding for generic audiovisual services". Decoder drivers may use the
 * parser to parse RBSP from encoded streams and configure the hardware, if
 * the hardware is not able to parse RBSP itself.  Encoder drivers may use the
 * generator to generate the RBSP for SPS/PPS nal units and add them to the
 * encoded stream if the hardware does not generate the units.
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
//#include <linux/export.h>
//#include <linux/log2.h>
#include "nal-h264.h"
#include "nal-rbsp.h"

//#include "log2.h"
/*
 * See Rec. ITU-T H.264 (04/2017) Table 7-1 - NAL unit type codes, syntax
 * element categories, and NAL unit type classes
 */
enum nal_unit_type {
	SEQUENCE_PARAMETER_SET = 7,
	PICTURE_PARAMETER_SET = 8,
	FILLER_DATA = 12,
};

static inline __attribute_const__
int __order_base_2(unsigned long n)
{
	return n > 1 ? ilog2(n - 1) + 1 : 0;
}

/**
 * order_base_2 - calculate the (rounded up) base 2 order of the argument
 * @n: parameter
 *
 * The first few values calculated by this routine:
 *  ob2(0) = 0
 *  ob2(1) = 0
 *  ob2(2) = 1
 *  ob2(3) = 2
 *  ob2(4) = 2
 *  ob2(5) = 3
 *  ... and so on.
 */
#define order_base_2(n)				\
(						\
	__builtin_constant_p(n) ? (		\
		((n) == 0 || (n) == 1) ? 0 :	\
		ilog2((n) - 1) + 1) :		\
	__order_base_2(n)			\
)

static void nal_h264_write_start_code_prefix(struct rbsp *rbsp)
{
	unsigned char *p = rbsp->data + DIV_ROUND_UP(rbsp->pos, 8);
	int i = 4;

	if (DIV_ROUND_UP(rbsp->pos, 8) + i > rbsp->size) {
		rbsp->error = -EINVAL;
		return;
	}

	p[0] = 0x00;
	p[1] = 0x00;
	p[2] = 0x00;
	p[3] = 0x01;

	rbsp->pos += i * 8;
}

static void nal_h264_read_start_code_prefix(struct rbsp *rbsp)
{
	unsigned char *p = rbsp->data + DIV_ROUND_UP(rbsp->pos, 8);
	int i = 4;

	if (DIV_ROUND_UP(rbsp->pos, 8) + i > rbsp->size) {
		rbsp->error = -EINVAL;
		return;
	}

	if (p[0] != 0x00 || p[1] != 0x00 || p[2] != 0x00 || p[3] != 0x01) {
		rbsp->error = -EINVAL;
		return;
	}

	rbsp->pos += i * 8;
}




static void nal_h264_rbsp_hrd_parameters(struct rbsp *rbsp,
					 struct nal_h264_hrd_parameters *hrd)
{
	unsigned int i;

	if (!hrd) {
		rbsp->error = -EINVAL;
		return;
	}

	rbsp_uev(rbsp, &hrd->cpb_cnt_minus1);
	rbsp_bits(rbsp, 4, &hrd->bit_rate_scale);
	rbsp_bits(rbsp, 4, &hrd->cpb_size_scale);

	for (i = 0; i <= hrd->cpb_cnt_minus1; i++) {
		rbsp_uev(rbsp, &hrd->bit_rate_value_minus1[i]);
		rbsp_uev(rbsp, &hrd->cpb_size_value_minus1[i]);
		rbsp_bit(rbsp, &hrd->cbr_flag[i]);
	}

	rbsp_bits(rbsp, 5, &hrd->initial_cpb_removal_delay_length_minus1);
	rbsp_bits(rbsp, 5, &hrd->cpb_removal_delay_length_minus1);
	rbsp_bits(rbsp, 5, &hrd->dpb_output_delay_length_minus1);
	rbsp_bits(rbsp, 5, &hrd->time_offset_length);
}

static void nal_h264_rbsp_vui_parameters(struct rbsp *rbsp,
					 struct nal_h264_vui_parameters *vui)
{
	if (!vui) {
		rbsp->error = -EINVAL;
		return;
	}

	rbsp_bit(rbsp, &vui->aspect_ratio_info_present_flag);
	if (vui->aspect_ratio_info_present_flag) {
		rbsp_bits(rbsp, 8, &vui->aspect_ratio_idc);
		printf("======H.264 SPS: aspect_ratio_idc %d\r\n", vui->aspect_ratio_idc);
		if (vui->aspect_ratio_idc == 255) {
			rbsp_bits(rbsp, 16, &vui->sar_width);
			rbsp_bits(rbsp, 16, &vui->sar_height);
			printf("======H.264 SPS: -> sar %dx%d\r\n", vui->sar_width, vui->sar_height);
		}
	}

	rbsp_bit(rbsp, &vui->overscan_info_present_flag);
	if (vui->overscan_info_present_flag)
		rbsp_bit(rbsp, &vui->overscan_appropriate_flag);

	rbsp_bit(rbsp, &vui->video_signal_type_present_flag);
	if (vui->video_signal_type_present_flag) {
		rbsp_bits(rbsp, 3, &vui->video_format);
		rbsp_bit(rbsp, &vui->video_full_range_flag);

		rbsp_bit(rbsp, &vui->colour_description_present_flag);
		if (vui->colour_description_present_flag) {
			rbsp_bits(rbsp, 8, &vui->colour_primaries);
			rbsp_bits(rbsp, 8, &vui->transfer_characteristics);
			rbsp_bits(rbsp, 8, &vui->matrix_coefficients);
		}
	}

	rbsp_bit(rbsp, &vui->chroma_loc_info_present_flag);
	if (vui->chroma_loc_info_present_flag) {
		rbsp_uev(rbsp, &vui->chroma_sample_loc_type_top_field);
		rbsp_uev(rbsp, &vui->chroma_sample_loc_type_bottom_field);
	}

	rbsp_bit(rbsp, &vui->timing_info_present_flag);
	if (vui->timing_info_present_flag) {
		rbsp_bits(rbsp, 32, &vui->num_units_in_tick);
		rbsp_bits(rbsp, 32, &vui->time_scale);
		rbsp_bit(rbsp, &vui->fixed_frame_rate_flag);

	printf("=========H.264 SPS: num_units_in_tick %d time_scale %d fixed_frame_rate_flag %d\r\n", 
		vui->num_units_in_tick, vui->time_scale, vui->fixed_frame_rate_flag);	
	}

	rbsp_bit(rbsp, &vui->nal_hrd_parameters_present_flag);
	if (vui->nal_hrd_parameters_present_flag)
		nal_h264_rbsp_hrd_parameters(rbsp, &vui->nal_hrd_parameters);

	rbsp_bit(rbsp, &vui->vcl_hrd_parameters_present_flag);
	if (vui->vcl_hrd_parameters_present_flag)
		nal_h264_rbsp_hrd_parameters(rbsp, &vui->vcl_hrd_parameters);

	if (vui->nal_hrd_parameters_present_flag ||
	    vui->vcl_hrd_parameters_present_flag)
		rbsp_bit(rbsp, &vui->low_delay_hrd_flag);

	rbsp_bit(rbsp, &vui->pic_struct_present_flag);

	rbsp_bit(rbsp, &vui->bitstream_restriction_flag);
	if (vui->bitstream_restriction_flag) {
		rbsp_bit(rbsp, &vui->motion_vectors_over_pic_boundaries_flag);
		rbsp_uev(rbsp, &vui->max_bytes_per_pic_denom);
		rbsp_uev(rbsp, &vui->max_bits_per_mb_denom);
		rbsp_uev(rbsp, &vui->log2_max_mv_length_horizontal);
		rbsp_uev(rbsp, &vui->log21_max_mv_length_vertical);
		rbsp_uev(rbsp, &vui->max_num_reorder_frames);
		rbsp_uev(rbsp, &vui->max_dec_frame_buffering);
	}
}

static void nal_h264_rbsp_sps(struct rbsp *rbsp, struct nal_h264_sps *sps)
{
	unsigned int i;

	if (!sps) {
		rbsp->error = -EINVAL;
		return;
	}

	rbsp_bits(rbsp, 8, &sps->profile_idc);
	rbsp_bit(rbsp, &sps->constraint_set0_flag);
	rbsp_bit(rbsp, &sps->constraint_set1_flag);
	rbsp_bit(rbsp, &sps->constraint_set2_flag);
	rbsp_bit(rbsp, &sps->constraint_set3_flag);
	rbsp_bit(rbsp, &sps->constraint_set4_flag);
	rbsp_bit(rbsp, &sps->constraint_set5_flag);
	rbsp_bits(rbsp, 2, &sps->reserved_zero_2bits);
	rbsp_bits(rbsp, 8, &sps->level_idc);
	printf("=========H.264 SPS: profile_idc %d level %d\r\n", sps->profile_idc, sps->level_idc);
	rbsp_uev(rbsp, &sps->seq_parameter_set_id);

	if (sps->profile_idc == 100 || sps->profile_idc == 110 ||
	    sps->profile_idc == 122 || sps->profile_idc == 244 ||
	    sps->profile_idc == 44 || sps->profile_idc == 83 ||
	    sps->profile_idc == 86 || sps->profile_idc == 118 ||
	    sps->profile_idc == 128 || sps->profile_idc == 138 ||
	    sps->profile_idc == 139 || sps->profile_idc == 134 ||
	    sps->profile_idc == 135) {
		rbsp_uev(rbsp, &sps->chroma_format_idc);

		if (sps->chroma_format_idc == 3)
			rbsp_bit(rbsp, &sps->separate_colour_plane_flag);
		rbsp_uev(rbsp, &sps->bit_depth_luma_minus8);
		rbsp_uev(rbsp, &sps->bit_depth_chroma_minus8);
		rbsp_bit(rbsp, &sps->qpprime_y_zero_transform_bypass_flag);
		rbsp_bit(rbsp, &sps->seq_scaling_matrix_present_flag);
		if (sps->seq_scaling_matrix_present_flag)
			rbsp->error = -EINVAL;
	}

	rbsp_uev(rbsp, &sps->log2_max_frame_num_minus4);

	rbsp_uev(rbsp, &sps->pic_order_cnt_type);
	switch (sps->pic_order_cnt_type) {
	case 0:
		rbsp_uev(rbsp, &sps->log2_max_pic_order_cnt_lsb_minus4);
		break;
	case 1:
		rbsp_bit(rbsp, &sps->delta_pic_order_always_zero_flag);
		rbsp_sev(rbsp, &sps->offset_for_non_ref_pic);
		rbsp_sev(rbsp, &sps->offset_for_top_to_bottom_field);

		rbsp_uev(rbsp, &sps->num_ref_frames_in_pic_order_cnt_cycle);
		for (i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
			rbsp_sev(rbsp, &sps->offset_for_ref_frame[i]);
		break;
	default:
		rbsp->error = -EINVAL;
		break;
	}

	rbsp_uev(rbsp, &sps->max_num_ref_frames);
	rbsp_bit(rbsp, &sps->gaps_in_frame_num_value_allowed_flag);
	rbsp_uev(rbsp, &sps->pic_width_in_mbs_minus1);
	rbsp_uev(rbsp, &sps->pic_height_in_map_units_minus1);

	rbsp_bit(rbsp, &sps->frame_mbs_only_flag);
	if (!sps->frame_mbs_only_flag)
		rbsp_bit(rbsp, &sps->mb_adaptive_frame_field_flag);

    printf("=======H.264 SPS: pic_width:  %u mbs\r\n", (zpl_uint32)sps->pic_width_in_mbs_minus1);
    printf("=======H.264 SPS: pic_height: %u mbs\r\n", (zpl_uint32)sps->pic_height_in_map_units_minus1);
    printf("=======H.264 SPS: frame only flag: %d\r\n", sps->frame_mbs_only_flag);

	rbsp_bit(rbsp, &sps->direct_8x8_inference_flag);

	rbsp_bit(rbsp, &sps->frame_cropping_flag);
	if (sps->frame_cropping_flag) {
		rbsp_uev(rbsp, &sps->crop_left);
		rbsp_uev(rbsp, &sps->crop_right);
		rbsp_uev(rbsp, &sps->crop_top);
		rbsp_uev(rbsp, &sps->crop_bottom);
        printf("=======H.264 SPS: cropping %d %d %d %d\r\n",
               sps->crop_left, sps->crop_top, sps->crop_right, sps->crop_bottom);
	}
	rbsp_bit(rbsp, &sps->vui_parameters_present_flag);
	if (sps->vui_parameters_present_flag)
		nal_h264_rbsp_vui_parameters(rbsp, &sps->vui);
}

static void nal_h264_rbsp_pps(struct rbsp *rbsp, struct nal_h264_pps *pps)
{
	int i;

	rbsp_uev(rbsp, &pps->pic_parameter_set_id);
	rbsp_uev(rbsp, &pps->seq_parameter_set_id);
	rbsp_bit(rbsp, &pps->entropy_coding_mode_flag);
	rbsp_bit(rbsp, &pps->bottom_field_pic_order_in_frame_present_flag);
	rbsp_uev(rbsp, &pps->num_slice_groups_minus1);
	if (pps->num_slice_groups_minus1 > 0) {
		rbsp_uev(rbsp, &pps->slice_group_map_type);
		switch (pps->slice_group_map_type) {
		case 0:
			for (i = 0; i < pps->num_slice_groups_minus1; i++)
				rbsp_uev(rbsp, &pps->run_length_minus1[i]);
			break;
		case 2:
			for (i = 0; i < pps->num_slice_groups_minus1; i++) {
				rbsp_uev(rbsp, &pps->top_left[i]);
				rbsp_uev(rbsp, &pps->bottom_right[i]);
			}
			break;
		case 3: case 4: case 5:
			rbsp_bit(rbsp, &pps->slice_group_change_direction_flag);
			rbsp_uev(rbsp, &pps->slice_group_change_rate_minus1);
			break;
		case 6:
			rbsp_uev(rbsp, &pps->pic_size_in_map_units_minus1);
			for (i = 0; i < pps->pic_size_in_map_units_minus1; i++)
				rbsp_bits(rbsp,
					  order_base_2(pps->num_slice_groups_minus1 + 1),
					  &pps->slice_group_id[i]);
			break;
		default:
			break;
		}
	}
	rbsp_uev(rbsp, &pps->num_ref_idx_l0_default_active_minus1);
	rbsp_uev(rbsp, &pps->num_ref_idx_l1_default_active_minus1);
	rbsp_bit(rbsp, &pps->weighted_pred_flag);
	rbsp_bits(rbsp, 2, &pps->weighted_bipred_idc);
	rbsp_sev(rbsp, &pps->pic_init_qp_minus26);
	rbsp_sev(rbsp, &pps->pic_init_qs_minus26);
	rbsp_sev(rbsp, &pps->chroma_qp_index_offset);
	rbsp_bit(rbsp, &pps->deblocking_filter_control_present_flag);
	rbsp_bit(rbsp, &pps->constrained_intra_pred_flag);
	rbsp_bit(rbsp, &pps->redundant_pic_cnt_present_flag);
	if (/* more_rbsp_data() */ false) {
		rbsp_bit(rbsp, &pps->transform_8x8_mode_flag);
		rbsp_bit(rbsp, &pps->pic_scaling_matrix_present_flag);
		if (pps->pic_scaling_matrix_present_flag)
			rbsp->error = -EINVAL;
		rbsp_sev(rbsp, &pps->second_chroma_qp_index_offset);
	}
}

/**
 * nal_h264_write_sps() - Write SPS NAL unit into RBSP format
 * @dev: device pointer
 * @dest: the buffer that is filled with RBSP data
 * @n: maximum size of @dest in bytes
 * @sps: &struct nal_h264_sps to convert to RBSP
 *
 * Convert @sps to RBSP data and write it into @dest.
 *
 * The size of the SPS NAL unit is not known in advance and this function will
 * fail, if @dest does not hold sufficient space for the SPS NAL unit.
 *
 * Return: number of bytes written to @dest or negative error code
 */
ssize_t nal_h264_write_sps(void *dest, size_t n, struct nal_h264_sps *sps)
{
	struct rbsp rbsp;
	unsigned int forbidden_zero_bit = 0;
	unsigned int nal_ref_idc = 0;
	unsigned int nal_unit_type = SEQUENCE_PARAMETER_SET;

	if (!dest)
		return -EINVAL;

	rbsp_init(&rbsp, dest, n, &rbsp_write_ops);

	nal_h264_write_start_code_prefix(&rbsp);

	rbsp_bit(&rbsp, &forbidden_zero_bit);
	rbsp_bits(&rbsp, 2, &nal_ref_idc);
	rbsp_bits(&rbsp, 5, &nal_unit_type);

	nal_h264_rbsp_sps(&rbsp, sps);

	rbsp_trailing_bits(&rbsp);

	if (rbsp.error)
		return rbsp.error;

	return DIV_ROUND_UP(rbsp.pos, 8);
}


/**
 * nal_h264_read_sps() - Read SPS NAL unit from RBSP format
 * @dev: device pointer
 * @sps: the &struct nal_h264_sps to fill from the RBSP data
 * @src: the buffer that contains the RBSP data
 * @n: size of @src in bytes
 *
 * Read RBSP data from @src and use it to fill @sps.
 *
 * Return: number of bytes read from @src or negative error code
 */
ssize_t nal_h264_read_sps(struct nal_h264_sps *sps, void *src, size_t n)
{
	struct rbsp rbsp;
	unsigned int forbidden_zero_bit;
	unsigned int nal_ref_idc;
	unsigned int nal_unit_type;

	if (!src)
		return -EINVAL;

	rbsp_init(&rbsp, src, n, &rbsp_read_ops);

	nal_h264_read_start_code_prefix(&rbsp);

	rbsp_bit(&rbsp, &forbidden_zero_bit);
	rbsp_bits(&rbsp, 2, &nal_ref_idc);
	rbsp_bits(&rbsp, 5, &nal_unit_type);

	if (rbsp.error ||
	    forbidden_zero_bit != 0 ||
	    /*nal_ref_idc != 0 ||*/
	    nal_unit_type != SEQUENCE_PARAMETER_SET)
		return -EINVAL;

	nal_h264_rbsp_sps(&rbsp, sps);

	rbsp_trailing_bits(&rbsp);

	if (rbsp.error)
		return rbsp.error;

	return DIV_ROUND_UP(rbsp.pos, 8);
}


/**
 * nal_h264_write_pps() - Write PPS NAL unit into RBSP format
 * @dev: device pointer
 * @dest: the buffer that is filled with RBSP data
 * @n: maximum size of @dest in bytes
 * @pps: &struct nal_h264_pps to convert to RBSP
 *
 * Convert @pps to RBSP data and write it into @dest.
 *
 * The size of the PPS NAL unit is not known in advance and this function will
 * fail, if @dest does not hold sufficient space for the PPS NAL unit.
 *
 * Return: number of bytes written to @dest or negative error code
 */
ssize_t nal_h264_write_pps(void *dest, size_t n, struct nal_h264_pps *pps)
{
	struct rbsp rbsp;
	unsigned int forbidden_zero_bit = 0;
	unsigned int nal_ref_idc = 0;
	unsigned int nal_unit_type = PICTURE_PARAMETER_SET;

	if (!dest)
		return -EINVAL;

	rbsp_init(&rbsp, dest, n, &rbsp_write_ops);

	nal_h264_write_start_code_prefix(&rbsp);

	/* NAL unit header */
	rbsp_bit(&rbsp, &forbidden_zero_bit);
	rbsp_bits(&rbsp, 2, &nal_ref_idc);
	rbsp_bits(&rbsp, 5, &nal_unit_type);

	nal_h264_rbsp_pps(&rbsp, pps);

	rbsp_trailing_bits(&rbsp);

	if (rbsp.error)
		return rbsp.error;

	return DIV_ROUND_UP(rbsp.pos, 8);
}

/**
 * nal_h264_read_pps() - Read PPS NAL unit from RBSP format
 * @dev: device pointer
 * @pps: the &struct nal_h264_pps to fill from the RBSP data
 * @src: the buffer that contains the RBSP data
 * @n: size of @src in bytes
 *
 * Read RBSP data from @src and use it to fill @pps.
 *
 * Return: number of bytes read from @src or negative error code
 */
ssize_t nal_h264_read_pps(struct nal_h264_pps *pps, void *src, size_t n)
{
	struct rbsp rbsp;

	if (!src)
		return -EINVAL;

	rbsp_init(&rbsp, src, n, &rbsp_read_ops);

	nal_h264_read_start_code_prefix(&rbsp);

	/* NAL unit header */
	rbsp.pos += 8;

	nal_h264_rbsp_pps(&rbsp, pps);

	rbsp_trailing_bits(&rbsp);

	if (rbsp.error)
		return rbsp.error;

	return DIV_ROUND_UP(rbsp.pos, 8);
}




int nal_h264_get_frame(FILE *fp, zpl_media_bufcache_t *outpacket)
{
    int ret = 0;
    static int tatol_len = 0;
    uint32_t packetsize = NALU_PACKET_SIZE_MAX, packet_offset = 0;
    uint32_t nalulen = 0, flags = 0, frist = 0;
    int offset_len = 0;
    uint8_t *p = NULL;
    H264_NALU_T naluhdr;
    char buftmp[NALU_PACKET_SIZE_MAX + 16];
    memset(&naluhdr, 0, sizeof(H264_NALU_T));
    outpacket->len = 0;
    while(fp)
    {
        packetsize = NALU_PACKET_SIZE_MAX - packet_offset;
        ret = fread(buftmp + packet_offset, 1, packetsize, fp);
        if(ret > 0)
        {
            if(flags == 0)
            {
                if (is_nalu4_start(buftmp))
                {
                    naluhdr.hdr_len = 4;
                    naluhdr.len = naluhdr.hdr_len;
                    naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                    naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                    naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                    naluhdr.buf = buftmp;
                    flags |= 0x01;/* 找到开始的标志 */
                    frist = 1;
                }
                else if (is_nalu3_start(buftmp))
                {
                    naluhdr.hdr_len = 3;
                    naluhdr.len = naluhdr.hdr_len;
                    naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                    naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                    naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                    naluhdr.buf = buftmp;
                    flags |= 0x01;/* 找到开始的标志 */
                    frist = 1;
                }
            }
            if(flags & 0x01)
            {
                if(frist)
                    p = buftmp + naluhdr.hdr_len;
                else    
                    p = buftmp;
                nalulen = zpl_media_channel_get_nextnalu(p, (ret)-4);
                if(nalulen)
                {
                    naluhdr.len += nalulen;
                    if(frist)
                        zpl_media_bufcache_add(outpacket, buftmp, naluhdr.len);
                    else
                        zpl_media_bufcache_add(outpacket, buftmp, nalulen);
                    if(frist)
                        offset_len = naluhdr.len - ret;
                    else    
                        offset_len = nalulen - ret;
                    if(offset_len)
                        fseek(fp, offset_len, SEEK_CUR);

                    flags = 0;
                    zpl_media_channel_nalu_show(&naluhdr);
                    tatol_len += outpacket->len;
                    return outpacket->len;
                }
                else
                {
                    if(ret > 8)
                    {
                        if(frist)
                            naluhdr.len = (ret-8);
                        else    
                            naluhdr.len += (ret-8);
                        zpl_media_bufcache_add(outpacket, buftmp, ret - 8);
    
                        packet_offset = 0;
                        fseek(fp, -8, SEEK_CUR);
                        frist = 0;
                    }
                    else
                    {
                        zpl_media_bufcache_add(outpacket, buftmp, ret);
                        naluhdr.len += ret;
                        offset_len = 0;
                        flags = 0;
                        //zpl_media_channel_nalu_show(&naluhdr);
                        tatol_len += outpacket->len;
                        return outpacket->len;
                    }
                }
            }
        }
        else
        {
            if(feof(fp))
            {
                zm_msg_error("===========can not get media frame data eof tatol_len=%d packetsize=%d, ret=%d error(%s)", tatol_len, packetsize, ret, strerror(errno));
                break;
            }
            else
            {
                if(ferror(fp))
                {
                    zm_msg_error("===========can not get media frame data tatol_len=%d packetsize=%d, ret=%d error(%s)", tatol_len, packetsize, ret, strerror(errno));
                    break;
                }
            }
        }
    }
    offset_len = 0;
    tatol_len = 0;
    return -1;
}


static int zpl_media_file_get_frame_h264_test(FILE *fp)
{
    int ret = 0;
    static int tatol_len = 0;
    uint32_t packetsize = 1400, packet_offset = 0;
    uint32_t nalulen = 0, flags = 0, frist = 0;
    int offset_len = 0;//, frame_len = 0;
    uint8_t *p = NULL;
    H264_NALU_T naluhdr;
    char buftmp[1400 + 16];
    char tmpdata[16];
    memset(&naluhdr, 0, sizeof(H264_NALU_T));
    while(fp)
    {
        packetsize = 1400 - packet_offset;
        ret = fread(buftmp + packet_offset, 1, packetsize, fp);
        if(ret > 0)
        {
            if(flags == 0)
            {
                if (is_nalu4_start(buftmp))
                {
                    naluhdr.hdr_len = 4;
                    naluhdr.len = naluhdr.hdr_len;
                    naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                    naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                    naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                    naluhdr.buf = buftmp;
                    flags |= 0x01;/* 找到开始的标志 */
                    frist = 1;
                }
                else if (is_nalu3_start(buftmp))
                {
                    naluhdr.hdr_len = 3;
                    naluhdr.len = naluhdr.hdr_len;
                    naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                    naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                    naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                    naluhdr.buf = buftmp;
                    flags |= 0x01;/* 设置开始的标志 */
                    frist = 1;
                }
            }
            if(flags & 0x01)/* 找到开始的标志 */
            {
                if(frist)
                    p = buftmp + naluhdr.hdr_len;
                else    
                    p = buftmp;
                nalulen = zpl_media_channel_get_nextnalu(p, (ret)-4);
                if(nalulen)
                {
                    naluhdr.len += nalulen;
                    tatol_len += naluhdr.len;
                    if(frist)
                        offset_len = naluhdr.len - ret;
                    else    
                        offset_len = nalulen - ret;
                    if(offset_len)
                        fseek(fp, offset_len, SEEK_CUR);
                    //printf("===========get media frame size=%d offset_len=%d\r\n", naluhdr.len, offset_len);
                    packet_offset = flags = 0;
                    
                    return naluhdr.len;
                }
                else
                {
                    if(ret > 8)
                    {
                        if(frist)
                            naluhdr.len = (ret-8);
                        else    
                            naluhdr.len += (ret-8);
                        memcpy(tmpdata, buftmp+(ret-8), 8);
                        memcpy(buftmp, tmpdata, 8);
                        packet_offset = 0;
                        fseek(fp, -8, SEEK_CUR);
                        frist = 0;
                    }
                    else
                    {
                        naluhdr.len += ret;
                        offset_len = 0;
                        flags = 0;
                        frist = 0;
                        tatol_len += ret;
                        printf("===========get media last frame size=%d\r\n", naluhdr.len);
                        printf("===========get media total frame size=%d\r\n", tatol_len);
                        return naluhdr.len;
                    }
                }
            }
        }
        else
        {
            if(feof(fp))
            {
                printf("===========can not get media frame data eof tatol_len=%d packetsize=%d, ret=%d error(%s)\r\n", tatol_len, packetsize, ret, strerror(errno));
                break;
            }
            else
            {
                if(ferror(fp))
                {
                    printf("===========can not get media frame data tatol_len=%d packetsize=%d, ret=%d error(%s)\r\n", tatol_len, packetsize, ret, strerror(errno));
                    break;
                }
            }
        }
    }
    offset_len = 0;
    tatol_len = 0;
    return -1;
}


int get_frame_h264_test(void)
{
    FILE *fp = NULL;
    zpl_media_file_open("/home/zhurish/workspace/working/zpl-source/source/multimedia/media/out-video.h264");
    exit(0);
    fp = fopen("/home/zhurish/workspace/working/zpl-source/source/multimedia/media/out-video.h264", "r");
    if(fp)
    {
        while(zpl_media_file_get_frame_h264_test(fp) > 0)
        ;
        fclose(fp);
    }
    return 0;
}