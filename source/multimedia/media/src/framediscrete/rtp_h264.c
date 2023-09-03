
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
# include <assert.h>
#include <stdbool.h>
#ifdef ZPL_JRTPLIB_MODULE 
#include <jrtplib_api.h>
#define MAX_RTP_PAYLOAD_LENGTH RTP_DEFAULTPACKETSIZE
#endif
#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "rtp_h264.h"
#include "rtp_payload.h"

/*
 * Find next NAL unit from the specified H.264 bitstream data.
 */
static u_int8_t* find_next_nal_unit(u_int8_t *start,
                                      u_int8_t *end)
{
    u_int8_t *p = start;

    /* Simply lookup "0x000001" pattern */
    while (p <= end-3 && (p[0] || p[1] || p[2]!=1))
        ++p;

    if (p > end-3)
        /* No more NAL unit in this bitstream */
        return NULL;

    /* Include 8 bits leading zero */
    if (p>start && *(p-1)==0)
        return (p-1);

    return p;
}



/*
 * Find synchronization point (PSC, slice, GSBC, EOS, EOSBS) in H.263 
 * bitstream.
 */
static u_int8_t* find_sync_point(u_int8_t *data,
                                   int data_len)
{
    u_int8_t *p = data, *end = data+data_len-1;

    while (p < end && (*p || *(p+1)))
        ++p;

    if (p == end)
        return NULL;
        
    return p;
}


/*
 * Find synchronization point (PSC, slice, GSBC, EOS, EOSBS) in H.263 
 * bitstream, in reversed manner.
 */
static u_int8_t* find_sync_point_rev(u_int8_t *data,
                                       int data_len)
{
    u_int8_t *p = data+data_len-2;

    while (p >= data && (*p || *(p+1)))
        --p;

    if (p < data)
        return (data + data_len);
        
    return p;
}

/*
 * Create H264 packetizer.
 */
#if 1
static int rtp_h264_packetizer_create(h26x_packetizer_cfg *cfg,
                                h26x_packetizer **p)
{
    h26x_packetizer *p_;

    //PJ_ASSERT_RETURN(pool && p, -1);

    if (cfg &&
        cfg->mode != H264_PACKETIZER_MODE_NON_INTERLEAVED &&
        cfg->mode != H264_PACKETIZER_MODE_SINGLE_NAL &&
        cfg->unpack_nal_start != 0 && cfg->unpack_nal_start != 3 &&
        cfg->unpack_nal_start != 4)
    {
        return -1;
    }

    p_ = malloc(sizeof(h26x_packetizer));
    if (cfg) {
        memcpy(&p_->cfg, cfg, sizeof(*cfg));
        if (p_->cfg.unpack_nal_start == 0)
            p_->cfg.unpack_nal_start = 3;
    } else {
        p_->cfg.mode = H264_PACKETIZER_MODE_NON_INTERLEAVED;
        p_->cfg.mtu = MAX_RTP_PAYLOAD_LENGTH;
        p_->cfg.unpack_nal_start = 3;
    }

    *p = p_;

    return 0;
}
#endif
/*
 * Generate an RTP payload from H.264 frame bitstream, in-place processing.
 */
static int rtp_h264_packetize(h26x_packetizer *pktz,
                       u_int8_t *buf,
                       int buf_len,
                       unsigned int *pos,
                       const u_int8_t **payload,
                       int *payload_len)
{
    u_int8_t *nal_start = NULL, *nal_end = NULL, *nal_octet = NULL;
    u_int8_t *p, *end;
    enum
    {
        HEADER_SIZE_FU_A = 2,
        HEADER_SIZE_STAP_A = 3,
    };
    enum
    {
        MAX_NALS_IN_AGGR = 32
    };

#if DBG_PACKETIZE
    if (*pos == 0 && buf_len)
    {
        zlog_debug(MODULE_MEDIA, "h264pack", "<< Start packing new frame >>");
    }
#endif

    p = buf + *pos;
    end = buf + buf_len;

    /* Find NAL unit startcode */
    if (end - p >= 4)
        nal_start = find_next_nal_unit(p, p + 4);
    if (nal_start)
    {
        /* Get NAL unit octet pointer */
        while (*nal_start++ == 0)
            ;
        nal_octet = nal_start;
    }
    else
    {
        /* This NAL unit is being fragmented */
        nal_start = p;
    }

    /* Get end of NAL unit */
    p = nal_start + pktz->cfg.mtu + 1;
    if (p > end || pktz->cfg.mode == H264_PACKETIZER_MODE_SINGLE_NAL)
        p = end;
    nal_end = find_next_nal_unit(nal_start, p);
    if (!nal_end)
        nal_end = p;

    /* Validate MTU vs NAL length on single NAL unit packetization */
    if ((pktz->cfg.mode == H264_PACKETIZER_MODE_SINGLE_NAL) &&
        nal_end - nal_start > pktz->cfg.mtu)
    {
        // assert(!"MTU too small for H.264 single NAL packetization mode");
        zlog_debug(MODULE_MEDIA, "MTU too small for H.264 (required=%u, MTU=%u)",
                   nal_end - nal_start, pktz->cfg.mtu);
        return -1;
    }

    /* Evaluate the proper payload format structure */

    /* Fragmentation (FU-A) packet */
    if ((pktz->cfg.mode != H264_PACKETIZER_MODE_SINGLE_NAL) &&
        (!nal_octet || nal_end - nal_start > pktz->cfg.mtu))
    {
        u_int8_t NRI, TYPE;

        if (nal_octet)
        {
            /* We have NAL unit octet, so this is the first fragment */
            NRI = (*nal_octet & 0x60) >> 5;
            TYPE = *nal_octet & 0x1F;

            /* Skip nal_octet in nal_start to be overriden by FU header */
            ++nal_start;
        }
        else
        {
            /* Not the first fragment, get NRI and NAL unit type
             * from the previous fragment.
             */
            p = nal_start - pktz->cfg.mtu;
            NRI = (*p & 0x60) >> 5;
            TYPE = *(p + 1) & 0x1F;
        }

        /* Init FU indicator (one octet: F+NRI+TYPE) */
        p = nal_start - HEADER_SIZE_FU_A;
        *p = (NRI << 5) | NAL_TYPE_FU_A;
        ++p;

        /* Init FU header (one octed: S+E+R+TYPE) */
        *p = TYPE;
        if (nal_octet)
            *p |= (1 << 7); /* S bit flag = start of fragmentation */
        if (nal_end - nal_start + HEADER_SIZE_FU_A <= pktz->cfg.mtu)
            *p |= (1 << 6); /* E bit flag = end of fragmentation */

        /* Set payload, payload length */
        *payload = nal_start - HEADER_SIZE_FU_A;
        if (nal_end - nal_start + HEADER_SIZE_FU_A > pktz->cfg.mtu)
            *payload_len = pktz->cfg.mtu;
        else
            *payload_len = nal_end - nal_start + HEADER_SIZE_FU_A;
        *pos = (unsigned int)(*payload + *payload_len - buf);

#if DBG_PACKETIZE
        zlog_debug(MODULE_MEDIA, "h264pack", "Packetized fragmented H264 NAL unit "
                                             "(pos=%d, type=%d, NRI=%d, S=%d, E=%d, len=%d/%d)",
                   *payload - buf, TYPE, NRI, *p >> 7, (*p >> 6) & 1, *payload_len,
                   buf_len);
#endif

        return 0;
    }

    /* Aggregation (STAP-A) packet */
    if ((pktz->cfg.mode != H264_PACKETIZER_MODE_SINGLE_NAL) &&
        (nal_end != end) &&
        (nal_end - nal_start + HEADER_SIZE_STAP_A) < pktz->cfg.mtu)
    {
        int total_size;
        unsigned int nal_cnt = 1;
        u_int8_t *nal[MAX_NALS_IN_AGGR];
        int nal_size[MAX_NALS_IN_AGGR];
        u_int8_t NRI;

        assert(nal_octet);

        /* Init the first NAL unit in the packet */
        nal[0] = nal_start;
        nal_size[0] = nal_end - nal_start;
        total_size = (int)nal_size[0] + HEADER_SIZE_STAP_A;
        NRI = (*nal_octet & 0x60) >> 5;

        /* Populate next NAL units */
        while (nal_cnt < MAX_NALS_IN_AGGR)
        {
            u_int8_t *tmp_end;

            /* Find start address of the next NAL unit */
            p = nal[nal_cnt - 1] + nal_size[nal_cnt - 1];
            while (*p++ == 0)
                ;
            nal[nal_cnt] = p;

            /* Find end address of the next NAL unit */
            tmp_end = p + (pktz->cfg.mtu - total_size);
            if (tmp_end > end)
                tmp_end = end;
            p = find_next_nal_unit(p + 1, tmp_end);
            if (p)
            {
                nal_size[nal_cnt] = p - nal[nal_cnt];
            }
            else
            {
                break;
            }

            /* Update total payload size (2 octet NAL size + NAL) */
            total_size += (2 + (int)nal_size[nal_cnt]);
            if (total_size <= pktz->cfg.mtu)
            {
                u_int8_t tmp_nri;

                /* Get maximum NRI of the aggregated NAL units */
                tmp_nri = (*(nal[nal_cnt] - 1) & 0x60) >> 5;
                if (tmp_nri > NRI)
                    NRI = tmp_nri;
            }
            else
            {
                break;
            }

            ++nal_cnt;
        }

        /* Only use STAP-A when we found more than one NAL units */
        if (nal_cnt > 1)
        {
            unsigned int i;

            /* Init STAP-A NAL header (F+NRI+TYPE) */
            p = nal[0] - HEADER_SIZE_STAP_A;
            *p++ = (NRI << 5) | NAL_TYPE_STAP_A;

            /* Append all populated NAL units into payload (SIZE+NAL) */
            for (i = 0; i < nal_cnt; ++i)
            {
                /* Put size (2 octets in network order) */
                assert(nal_size[i] <= 0xFFFF);
                *p++ = (u_int8_t)(nal_size[i] >> 8);
                *p++ = (u_int8_t)(nal_size[i] & 0xFF);

                /* Append NAL unit, watchout memmove()-ing bitstream! */
                if (p != nal[i])
                    memmove(p, nal[i], nal_size[i]);
                p += nal_size[i];
            }

            /* Set payload, payload length, and pos */
            *payload = nal[0] - HEADER_SIZE_STAP_A;
            assert(*payload >= buf + *pos);
            *payload_len = p - *payload;
            *pos = (unsigned int)(nal[nal_cnt - 1] + nal_size[nal_cnt - 1] - buf);

#if DBG_PACKETIZE
            PJ_LOG(3, ("h264pack", "Packetized aggregation of "
                                   "%d H264 NAL units (pos=%d, NRI=%d len=%d/%d)",
                       nal_cnt, *payload - buf, NRI, *payload_len, buf_len));
#endif

            return 0;
        }
    }

    /* Single NAL unit packet */
    *payload = nal_start;
    *payload_len = nal_end - nal_start;
    *pos = (unsigned int)(nal_end - buf);

#if DBG_PACKETIZE
    PJ_LOG(3, ("h264pack", "Packetized single H264 NAL unit "
                           "(pos=%d, type=%d, NRI=%d, len=%d/%d)",
               nal_start - buf, *nal_octet & 0x1F, (*nal_octet & 0x60) >> 5,
               *payload_len, buf_len));
#endif

    return 0;
}

/*
 * Append RTP payload to a H.264 picture bitstream. Note that the only
 * payload format that cares about packet lost is the NAL unit
 * fragmentation format (FU-A/B), so we will only manage the "prev_lost"
 * state for the FU-A/B packets.
 */
static int rtp_h264_unpacketize(h26x_packetizer *pktz,
                                             const u_int8_t *payload,
                                             int   payload_len,
                                             u_int8_t *bits,
                                             int   bits_len,
                                             unsigned int   *bits_pos)
{
    const u_int8_t nal_start[4] = {0, 0, 0, 1};
    const u_int8_t *nal_start_code; 
    enum { MIN_PAYLOAD_SIZE = 2 };
    u_int8_t nal_type;

    nal_start_code = nal_start + sizeof(nal_start)/sizeof(nal_start[0]) -
                     pktz->cfg.unpack_nal_start;

#if DBG_UNPACKETIZE
    if (*bits_pos == 0 && payload_len) {
        PJ_LOG(3, ("h264unpack", ">> Start unpacking new frame <<"));
    }
#endif

    /* Check if this is a missing/lost packet */
    if (payload == NULL) {
        pktz->unpack_prev_lost = 1;
        return 0;
    }

    /* H264 payload size */
    if (payload_len < MIN_PAYLOAD_SIZE) {
        /* Invalid bitstream, discard this payload */
        pktz->unpack_prev_lost = 1;
        return -1;
    }

    /* Reset last sync point for every new picture bitstream */
    if (*bits_pos == 0)
        pktz->unpack_last_sync_pos = 0;

    nal_type = *payload & 0x1F;
    if (nal_type >= NAL_TYPE_SINGLE_NAL_MIN &&
        nal_type <= NAL_TYPE_SINGLE_NAL_MAX)
    {
        /* Single NAL unit packet */
        u_int8_t *p = bits + *bits_pos;

        /* Validate bitstream length */
        if (bits_len-*bits_pos < payload_len+pktz->cfg.unpack_nal_start) {
            /* Insufficient bistream buffer, discard this payload */
            assert(!"Insufficient H.264 bitstream buffer");
            return -1;
        }

        /* Write NAL unit start code */
        memcpy(p, nal_start_code, pktz->cfg.unpack_nal_start);
        p += pktz->cfg.unpack_nal_start;

        /* Write NAL unit */
        memcpy(p, payload, payload_len);
        p += payload_len;

        /* Update the bitstream writing offset */
        *bits_pos = (unsigned int)(p - bits);
        pktz->unpack_last_sync_pos = *bits_pos;

#if DBG_UNPACKETIZE
        PJ_LOG(3, ("h264unpack", "Unpacked single H264 NAL unit "
                   "(type=%d, NRI=%d, len=%d)",
                   nal_type, (*payload&0x60)>>5, payload_len));
#endif

    }
    else if (nal_type == NAL_TYPE_STAP_A)
    {
        /* Aggregation packet */
        u_int8_t *p, *p_end;
        const u_int8_t *q, *q_end;
        unsigned int cnt = 0;

        /* Validate bitstream length */
        if (bits_len - *bits_pos < payload_len + 32) {
            /* Insufficient bistream buffer, discard this payload */
            assert(!"Insufficient H.264 bitstream buffer");
            return -1;
        }

        /* Fill bitstream */
        p = bits + *bits_pos;
        p_end = bits + bits_len;
        q = payload + 1;
        q_end = payload + payload_len;
        while (q < q_end && p < p_end) {
            u_int16_t tmp_nal_size;

            /* Write NAL unit start code */
            memcpy(p, nal_start_code, pktz->cfg.unpack_nal_start);
            p += pktz->cfg.unpack_nal_start;

            /* Get NAL unit size */
            tmp_nal_size = (*q << 8) | *(q+1);
            q += 2;
            if (q + tmp_nal_size > q_end) {
                /* Invalid bitstream, discard the rest of the payload */
                return -1;
            }

            /* Write NAL unit */
            memcpy(p, q, tmp_nal_size);
            p += tmp_nal_size;
            q += tmp_nal_size;
            ++cnt;

            /* Update the bitstream writing offset */
            *bits_pos = (unsigned int)(p - bits);
            pktz->unpack_last_sync_pos = *bits_pos;
        }

#if DBG_UNPACKETIZE
        PJ_LOG(3, ("h264unpack", "Unpacked %d H264 NAL units (len=%d)",
                   cnt, payload_len));
#endif

    }
    else if (nal_type == NAL_TYPE_FU_A)
    {
        /* Fragmentation packet */
        u_int8_t *p;
        const u_int8_t *q = payload;
        u_int8_t NRI, TYPE, S, E;

        p = bits + *bits_pos;

        /* Validate bitstream length */
        if (bits_len-*bits_pos < payload_len+pktz->cfg.unpack_nal_start) {
            /* Insufficient bistream buffer, drop this packet */
            assert(!"Insufficient H.264 bitstream buffer");
            pktz->unpack_prev_lost = 1;
            return -1;
        }

        /* Get info */
        S = *(q+1) & 0x80;    /* Start bit flag */
        E = *(q+1) & 0x40;    /* End bit flag   */
        TYPE = *(q+1) & 0x1f;
        NRI = (*q & 0x60) >> 5;

        /* Fill bitstream */
        if (S) {
            /* This is the first part, write NAL unit start code */
            memcpy(p, nal_start_code, pktz->cfg.unpack_nal_start);
            p += pktz->cfg.unpack_nal_start;

            /* Write NAL unit octet */
            *p++ = (NRI << 5) | TYPE;
        } else if (pktz->unpack_prev_lost) {
            /* If prev packet was lost, revert the bitstream pointer to
             * the last sync point.
             */
            assert(pktz->unpack_last_sync_pos <= *bits_pos);
            *bits_pos = pktz->unpack_last_sync_pos;
            /* And discard this payload (and the following fragmentation
             * payloads carrying this same NAL unit.
             */
            return -1;
        }
        q += 2;

        /* Write NAL unit */
        memcpy(p, q, payload_len - 2);
        p += (payload_len - 2);

        /* Update the bitstream writing offset */
        *bits_pos = (unsigned int)(p - bits);
        if (E) {
            /* Update the sync pos only if the end bit flag is set */
            pktz->unpack_last_sync_pos = *bits_pos;
        }

#if DBG_UNPACKETIZE
        PJ_LOG(3, ("h264unpack", "Unpacked fragmented H264 NAL unit "
                   "(type=%d, NRI=%d, len=%d)",
                   TYPE, NRI, payload_len));
#endif

    } else {
        *bits_pos = 0;
        return -1;
    }

    pktz->unpack_prev_lost = 0;

    return 0;
}



/*
 * Create H263 packetizer.
 */
#if 0
static int rtp_h263_packetizer_create(const h26x_packetizer_cfg *cfg,
                                h26x_packetizer **p)
{
    h26x_packetizer *p_;

    //PJ_ASSERT_RETURN(pool && p, -1);

    if (cfg && cfg->mode != H263_PACKETIZER_MODE_RFC4629)
        return -1;

    p_ = malloc(sizeof(h26x_packetizer));
    if (cfg) {
        memcpy(&p_->cfg, cfg, sizeof(*cfg));
    } else {
        p_->cfg.mode = H263_PACKETIZER_MODE_RFC4629;
        p_->cfg.mtu = MAX_RTP_PAYLOAD_LENGTH;
    }

    *p = p_;

    return 0;
}
#endif

/*
 * Generate an RTP payload from H.263 frame bitstream, in-place processing.
 */
static int rtp_h263_packetize(h26x_packetizer *pktz,
                                           u_int8_t *bits,
                                           int bits_len,
                                           unsigned int *pos,
                                           const u_int8_t **payload,
                                           int *payload_len)
{
    u_int8_t *p, *end;

    assert(pktz && bits && pos && payload && payload_len);
    assert(*pos <= bits_len);

    p = bits + *pos;
    end = bits + bits_len;

    /* Put two octets payload header */
    if ((end-p > 2) && *p==0 && *(p+1)==0) {
        /* The bitstream starts with synchronization point, just override
         * the two zero octets (sync point mark) for payload header.
         */
        *p = 0x04;
    } else {
        /* Not started in synchronization point, we will use two octets
         * preceeding the bitstream for payload header!
         */

        if (*pos < 2) {
            /* Invalid H263 bitstream, it's not started with PSC */
            return -1;
        }

        p -= 2;
        *p = 0;
    }
    *(p+1) = 0;

    /* When bitstream truncation needed because of payload length/MTU 
     * limitation, try to use sync point for the payload boundary.
     */
    if (end-p > pktz->cfg.mtu) {
        end = find_sync_point_rev(p+2, pktz->cfg.mtu-2);
    }

    *payload = p;
    *payload_len = end-p;
    *pos = (unsigned int)(end - bits);

    return 0;
}


/*
 * Append an RTP payload to a H.263 picture bitstream.
 */
static int rtp_h263_unpacketize (h26x_packetizer *pktz,
                                              const u_int8_t *payload,
                                              int payload_len,
                                              u_int8_t *bits,
                                              int bits_size,
                                              unsigned int *pos)
{
    u_int8_t P, V, PLEN;
    const u_int8_t *p = payload;
    u_int8_t *q;

    q = bits + *pos;

    /* Check if this is a missing/lost packet */
    if (payload == NULL) {
        pktz->unpack_prev_lost = 1;
        return 0;
    }

    /* H263 payload header size is two octets */
    if (payload_len < 2) {
        /* Invalid bitstream, discard this payload */
        pktz->unpack_prev_lost = 1;
        return -1;
    }

    /* Reset last sync point for every new picture bitstream */
    if (*pos == 0)
        pktz->unpack_last_sync_pos = 0;

    /* Get payload header info */
    P = *p & 0x04;
    V = *p & 0x02;
    PLEN = ((*p & 0x01) << 5) + ((*(p+1) & 0xF8)>>3);

    /* Get start bitstream pointer */
    p += 2;         /* Skip payload header */
    if (V)
        p += 1;     /* Skip VRC data */
    if (PLEN)
        p += PLEN;  /* Skip extra picture header data */

    /* Get bitstream length */
    if (payload_len > (int)(p - payload)) {
        payload_len -= (p - payload);
    } else {
        /* Invalid bitstream, discard this payload */
        pktz->unpack_prev_lost = 1;
        return -1;
    }

    /* Validate bitstream length */
    if (bits_size < *pos + payload_len + 2) {
        /* Insufficient bistream buffer, discard this payload */
        assert(!"Insufficient H.263 bitstream buffer");
        pktz->unpack_prev_lost = 1;
        return -1;
    }

    /* Start writing bitstream */

    /* No sync point flag */
    if (!P) {
        if (*pos == 0) {
            /* Previous packet must be lost */
            pktz->unpack_prev_lost = 1;

            /* If there is extra picture header, let's use it. */
            if (PLEN) {
                /* Write two zero octets for PSC */
                *q++ = 0;
                *q++ = 0;
                /* Copy the picture header */
                p -= PLEN;
                memcpy(q, p, PLEN);
                p += PLEN;
                q += PLEN;
            }
        } else if (pktz->unpack_prev_lost) {
            /* If prev packet was lost, revert the bitstream pointer to
             * the last sync point.
             */
            assert(pktz->unpack_last_sync_pos <= *pos);
            q = bits + pktz->unpack_last_sync_pos;
        }

        /* There was packet lost, see if this payload contain sync point
         * (usable data).
         */
        if (pktz->unpack_prev_lost) {
            u_int8_t *sync;
            sync = find_sync_point((u_int8_t*)p, payload_len);
            if (sync) {
                /* Got sync point, update P/sync-point flag */
                P = 1;
                /* Skip the two zero octets */
                sync += 2;
                /* Update payload length and start bitstream pointer */
                payload_len -= (sync - p);
                p = sync;
            } else {
                /* No sync point in it, just discard this payload */
                return -1;
            }
        }
    }

    /* Write two zero octets when payload flagged with sync point */
    if (P) {
        pktz->unpack_last_sync_pos = (unsigned int)(q - bits);
        *q++ = 0;
        *q++ = 0;
    }

    /* Write the payload to the bitstream */
    memcpy(q, p, payload_len);
    q += payload_len;

    /* Update the bitstream writing offset */
    *pos = (unsigned int)(q - bits);

    pktz->unpack_prev_lost = 0;

    return 0;
}


int rtp_h26x_packetizer_init(int codec, int mtu, int mode, int startbit,
                                    h26x_packetizer *p_pktz)
{
    memset(p_pktz, 0, sizeof(h26x_packetizer));
    p_pktz->cfg.codec = codec;
    p_pktz->cfg.mtu = mtu;
    p_pktz->cfg.mode = mode;
    p_pktz->cfg.unpack_nal_start = startbit;
    return OK;
}   

int rtp_h26x_packetize(h26x_packetizer *pktz,
                                            u_int8_t *bits,
                                            int bits_len,
                                            unsigned int *bits_pos,
                                            const u_int8_t **payload,
                                            int *payload_len)
{
    if(pktz->cfg.codec == RTP_MEDIA_PAYLOAD_H263)
    {
        return rtp_h263_packetize(pktz, bits, bits_len, bits_pos, payload, payload_len);
    }
    else if(pktz->cfg.codec == RTP_MEDIA_PAYLOAD_H264)
    {
        return rtp_h264_packetize(pktz, bits, bits_len, bits_pos, payload, payload_len);
    }
    return ERROR;
}                                            
int rtp_h26x_unpacketize (h26x_packetizer *pktz,
                                              const u_int8_t *payload,
                                              int payload_len,
                                              u_int8_t *bits,
                                              int bits_size,
                                              unsigned int *pos)
{
    if(pktz->cfg.codec == RTP_MEDIA_PAYLOAD_H263)
    {
        return rtp_h263_unpacketize(pktz, payload, payload_len, bits, bits_size, pos);
    }
    else if(pktz->cfg.codec == RTP_MEDIA_PAYLOAD_H264)
    {
        return rtp_h264_unpacketize(pktz, payload, payload_len, bits, bits_size, pos);
    }
    return ERROR;
}



static h26x_packetizer *my_h264_pktz = NULL;

static int rtp_payload_send_h264_oneframe(void *session, const u_int8_t *buffer, u_int32_t len)
{
    u_int8_t *payload = NULL;
    u_int32_t payload_len = 0;
    int has_more = 1;
    int ret = 0;
    if(my_h264_pktz == NULL)
    {
        h26x_packetizer_cfg cfg;
        memset(&cfg, 0, sizeof(h26x_packetizer_cfg));
        cfg.mode = H264_PACKETIZER_MODE_NON_INTERLEAVED;
        cfg.unpack_nal_start = 4;
        cfg.mtu = MAX_RTP_PAYLOAD_LENGTH;
        rtp_h264_packetizer_create(&cfg, &my_h264_pktz);
        rtp_h26x_packetizer_init(RTP_MEDIA_PAYLOAD_H264, MAX_RTP_PAYLOAD_LENGTH,
                                                 H264_PACKETIZER_MODE_NON_INTERLEAVED, 4, my_h264_pktz);
    }
    my_h264_pktz->frame_data = buffer;
    my_h264_pktz->frame_size = len;
    my_h264_pktz->frame_pos = 0;
    if(my_h264_pktz)
    {
        while(has_more)
        {
            ret = rtp_h26x_packetize(my_h264_pktz,my_h264_pktz->frame_data,
                                        my_h264_pktz->frame_size,
                                        &my_h264_pktz->frame_pos,
                                        &payload, &payload_len);
            if(ret != 0)
                return -1;                      
            has_more = (my_h264_pktz->frame_pos < my_h264_pktz->frame_size);
#ifdef ZPL_JRTPLIB_MODULE 
            if(payload)
            {
                if(len < MAX_RTP_PAYLOAD_LENGTH)
                    ret = zpl_mediartp_session_rtp_sendto(session, payload, payload_len, 255, 1, 1);
                else
                    ret = zpl_mediartp_session_rtp_sendto(session, payload, payload_len, 255, has_more?0:1, has_more?0:1);
            }
#endif                                     
        }
    }
    return ret;
}

int rtp_payload_send_h264(void *session, const u_int8_t *buffer, u_int32_t len)
{
    return rtp_payload_send_h264_oneframe(session, buffer, len);
}

#if 0
int zpl_mediartp_session_adap_rtp_packet(zpl_media_channel_t *chm, uint32_t codec, const uint8_t *buffer, uint32_t len, int user_ts)
//static int rtp_payload_send_h264_oneframe(void *session, const u_int8_t *buffer, u_int32_t len, int user_ts)
{
    u_int8_t *payload = NULL;
    u_int32_t payload_len = 0;
    int has_more = 1;
    int ret = 0;
    if(my_h264_pktz == NULL)
    {
        h26x_packetizer_cfg cfg;
        memset(&cfg, 0, sizeof(h26x_packetizer_cfg));
        cfg.mode = H264_PACKETIZER_MODE_NON_INTERLEAVED;
        cfg.unpack_nal_start = 4;
        cfg.mtu = MAX_RTP_PAYLOAD_LENGTH;
        rtp_h264_packetizer_create(&cfg, &my_h264_pktz);
    }
    my_h264_pktz->frame_data = buffer;
    my_h264_pktz->frame_size = len;
    my_h264_pktz->frame_pos = 0;
    if(my_h264_pktz)
    {
        while(has_more)
        {
            ret = rtp_h264_packetize(my_h264_pktz,my_h264_pktz->frame_data,
                                        my_h264_pktz->frame_size,
                                        &my_h264_pktz->frame_pos,
                                        &payload, &payload_len);
            if(ret != 0)
                return -1;                      
            has_more = (my_h264_pktz->frame_pos < my_h264_pktz->frame_size);
#ifdef ZPL_JRTPLIB_MODULE 
            if(payload)
            {
                if(len < MAX_RTP_PAYLOAD_LENGTH)
                    ret = zpl_mediartp_session_rtp_sendto(chm, payload, payload_len, 255, 1, user_ts);
                else
                    ret = zpl_mediartp_session_rtp_sendto(chm, payload, payload_len, 255, has_more?0:1, has_more?0:user_ts);
            }
#endif                                     
        }
    }
    return ret;
}
#endif