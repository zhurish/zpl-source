/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include "pjsua_app_common.h"

#define THIS_FILE       "pjsua_app_common.c"

#if defined(PJMEDIA_HAS_RTCP_XR) && (PJMEDIA_HAS_RTCP_XR != 0)
#   define SOME_BUF_SIZE        (1024 * 10)
#else
#   define SOME_BUF_SIZE        (1024 * 3)
#endif

#ifdef USE_GUI
void displayWindow(pjsua_vid_win_id wid);
#endif

static char some_buf[SOME_BUF_SIZE];

/** Variable definition **/

pjsua_app_config    _pjAppCfg = {.initialization = 0,};


int my_atoi(const char *cs)
{
    pj_str_t s;
    pj_cstr(&s, cs);
    return my_atoi2(&s);
}

int my_atoi2(const pj_str_t *str)
{
    const char *cs = str->ptr;
    pj_str_t s = *str;

    if (cs[0] == '-') {
        s.ptr++; s.slen--;
        return 0 - (int)pj_strtoul(&s);
    } else if (cs[0] == '+') {
        s.ptr++; s.slen--;
        return (int)pj_strtoul(&s);
    } else {
        return (int)pj_strtoul(&s);
    }
}

pj_bool_t pjapp_incoming_call(void)
{
	return _pjAppCfg.incomeing;
}
/*
 * Find next call when current call is disconnected or when user
 * press ']'
 */
pj_bool_t pjapp_find_next_call(void)
{
    int i, max;

    max = pjsua_call_get_max_count();
    for (i=_pjAppCfg.current_call+1; i<max; ++i) {
        if (pjsua_call_is_active(i)) {
            _pjAppCfg.current_call = i;
            return PJ_TRUE;
        }
    }

    for (i=0; i<_pjAppCfg.current_call; ++i) {
        if (pjsua_call_is_active(i)) {
            _pjAppCfg.current_call = i;
            return PJ_TRUE;
        }
    }
	_pjAppCfg.incomeing = PJ_FALSE;
    _pjAppCfg.current_call = PJSUA_INVALID_ID;
    return PJ_FALSE;
}

pj_bool_t pjapp_find_prev_call(void)
{
    int i, max;

    max = pjsua_call_get_max_count();
    for (i=_pjAppCfg.current_call-1; i>=0; --i) {
        if (pjsua_call_is_active(i)) {
            _pjAppCfg.current_call = i;
            return PJ_TRUE;
        }
    }

    for (i=max-1; i>_pjAppCfg.current_call; --i) {
        if (pjsua_call_is_active(i)) {
            _pjAppCfg.current_call = i;
            return PJ_TRUE;
        }
    }
    _pjAppCfg.incomeing = PJ_FALSE;
    _pjAppCfg.current_call = PJSUA_INVALID_ID;
    return PJ_FALSE;
}

pjsua_call_id pjapp_current_call(void)
{
	return _pjAppCfg.current_call;
}
/*
 * Send arbitrary request to remote host
 */
void pjapp_send_request(char *cstr_method, const pj_str_t *dst_uri)
{
    pj_str_t str_method;
    pjsip_method method;
    pjsip_tx_data *tdata;
    pjsip_endpoint *endpt;
    pj_status_t status;

    endpt = pjsua_get_pjsip_endpt();

    str_method = pj_str(cstr_method);
    pjsip_method_init_np(&method, &str_method);

    status = pjsua_acc_create_request(current_acc, &method, dst_uri, &tdata);

    status = pjsip_endpt_send_request(endpt, tdata, -1, NULL, NULL);
    if (status != PJ_SUCCESS) {
        pjsua_perror(THIS_FILE, "Unable to send request", status);
        return;
    }
}

/*
 * Print log of call states. Since call states may be too long for logger,
 * printing it is a bit tricky, it should be printed part by part as long 
 * as the logger can accept.
 */
void pjapp_log_call_dump(int call_id) 
{
    unsigned call_dump_len;
    unsigned part_len;
    unsigned part_idx;
    unsigned log_decor;

    pjsua_call_dump(call_id, PJ_TRUE, some_buf, sizeof(some_buf), "  ");
    call_dump_len = (unsigned)strlen(some_buf);

    log_decor = pj_log_get_decor();
    pj_log_set_decor(log_decor & ~(PJ_LOG_HAS_NEWLINE | PJ_LOG_HAS_CR));
    PJ_LOG(3,(THIS_FILE, "\n"));
    pj_log_set_decor(0);

    part_idx = 0;
    part_len = PJ_LOG_MAX_SIZE-80;
    while (part_idx < call_dump_len) {
        char p_orig, *p;

        p = &some_buf[part_idx];
        if (part_idx + part_len > call_dump_len)
            part_len = call_dump_len - part_idx;
        p_orig = p[part_len];
        p[part_len] = '\0';
        PJ_LOG(3,(THIS_FILE, "%s", p));
        p[part_len] = p_orig;
        part_idx += part_len;
    }
    pj_log_set_decor(log_decor);
}

#ifdef PJSUA_HAS_VIDEO
void pjapp_config_video_init(pjsua_acc_config *acc_cfg)
{
    acc_cfg->vid_in_auto_show = _pjAppCfg.vid.in_auto_show;
    acc_cfg->vid_out_auto_transmit = _pjAppCfg.vid.out_auto_transmit;
    /* Note that normally GUI application will prefer a borderless
     * window.
     */
    acc_cfg->vid_wnd_flags = PJMEDIA_VID_DEV_WND_BORDER |
                             PJMEDIA_VID_DEV_WND_RESIZABLE;
    acc_cfg->vid_cap_dev = _pjAppCfg.vid.vcapture_dev;
    acc_cfg->vid_rend_dev = _pjAppCfg.vid.vrender_dev;

    if (_pjAppCfg.avi_auto_play &&
        _pjAppCfg.avi_def_idx != PJSUA_INVALID_ID &&
        _pjAppCfg.avi[_pjAppCfg.avi_def_idx].dev_id != PJMEDIA_VID_INVALID_DEV)
    {
        acc_cfg->vid_cap_dev = _pjAppCfg.avi[_pjAppCfg.avi_def_idx].dev_id;
    }
}
#else
void pjapp_config_video_init(pjsua_acc_config *acc_cfg)
{
    PJ_UNUSED_ARG(acc_cfg);
}
#endif

#ifdef HAVE_MULTIPART_TEST
  /*
   * Enable multipart in msg_data and add a dummy body into the
   * multipart bodies.
   */
  void add_multipart(pjsua_msg_data *msg_data)
  {
      static pjsip_multipart_part *alt_part;

      if (!alt_part) {
          pj_str_t type, subtype, content;

          alt_part = pjsip_multipart_create_part(_pjAppCfg.pool);

          type = pj_str("text");
          subtype = pj_str("plain");
          content = pj_str("Sample text body of a multipart bodies");
          alt_part->body = pjsip_msg_body_create(_pjAppCfg.pool, &type,
                                                 &subtype, &content);
      }

      msg_data->multipart_ctype.type = pj_str("multipart");
      msg_data->multipart_ctype.subtype = pj_str("mixed");
      pj_list_push_back(&msg_data->multipart_parts, alt_part);
  }
#endif

/* arrange windows. arg:
 *   -1:    arrange all windows
 *   != -1: arrange only this window id
 */
void pjapp_arrange_window(pjsua_vid_win_id wid)
{
#if PJSUA_HAS_VIDEO
    pjmedia_coord pos;
    int i, last;

    pos.x = 0;
    pos.y = 10;
    last = (wid == PJSUA_INVALID_ID) ? PJSUA_MAX_VID_WINS : wid;

    for (i=0; i<last; ++i) {
        pjsua_vid_win_info wi;
        pj_status_t status;

        status = pjsua_vid_win_get_info(i, &wi);
        if (status != PJ_SUCCESS)
            continue;

        if (wid == PJSUA_INVALID_ID)
            pjsua_vid_win_set_pos(i, &pos);

        if (wi.show)
            pos.y += wi.size.h;
    }

    if (wid != PJSUA_INVALID_ID)
        pjsua_vid_win_set_pos(wid, &pos);

#ifdef USE_GUI
    displayWindow(wid);
#endif

#else
    PJ_UNUSED_ARG(wid);
#endif
}

static void aud_print_dev(pj_cli_cmd_val *cval, int id, const pjmedia_aud_dev_info *vdi, const char *title)
{
    char capnames[120];
    unsigned i;
    int st_len;

    capnames[0] = '\0';
    st_len = 0;
    for (i=0; i<sizeof(int)*8 && (1 << i) < PJMEDIA_AUD_DEV_CAP_MAX; ++i) {
        if (vdi->caps & (1 << i)) {
            const char *capname = pjmedia_aud_dev_cap_name(1 << i, NULL);
            if (capname) {
                int tmp_len = (int)strlen(capname);
                if ((int)sizeof(capnames) - st_len <= tmp_len)
                    break;

                st_len += (tmp_len + 2);
                if (*capnames)
                    strcat(capnames, ", ");
                strcat(capnames, capname);
            }
        }
    }
    pj_cli_out(cval, "%3d %s [%s] %s", id, vdi->name, vdi->driver,
              title);
    pj_cli_out(cval, "    Supported capabilities: %s", capnames);
}
void pjapp_aud_list_devs(pj_cli_cmd_val *cval)
{
    unsigned i, count;
    pjmedia_aud_dev_info vdi;
    pj_status_t status;

    pj_cli_out(cval, "Audio device list:");
    count = pjmedia_aud_dev_count();
    if (count == 0) {
        pj_cli_out(cval, " - no device detected -");
        return;
    } else {
        pj_cli_out(cval, "%d device(s) detected:", count);
    }

    status = pjmedia_aud_dev_get_info(PJMEDIA_AUD_DEFAULT_CAPTURE_DEV, &vdi);
    if (status == PJ_SUCCESS)
        aud_print_dev(cval,PJMEDIA_AUD_DEFAULT_CAPTURE_DEV, &vdi,
                      "(default capture device)");

    status = pjmedia_aud_dev_get_info(PJMEDIA_AUD_DEFAULT_PLAYBACK_DEV, &vdi);
    if (status == PJ_SUCCESS)
        aud_print_dev(cval,PJMEDIA_AUD_DEFAULT_PLAYBACK_DEV, &vdi,
                      "(default playback device)");

    for (i=0; i<count; ++i) {
        status = pjmedia_aud_dev_get_info(i, &vdi);
        if (status == PJ_SUCCESS)
            aud_print_dev(cval,i, &vdi, "");
    }
}
#if PJSUA_HAS_VIDEO
void pjapp_vid_print_dev(pj_cli_cmd_val *cval, int id, const pjmedia_vid_dev_info *vdi, const char *title)
{
    char capnames[120];
    char formats[200];
    const char *dirname;
    unsigned i;
    int st_len;

    if (vdi->dir == PJMEDIA_DIR_CAPTURE_RENDER) {
        dirname = "capture, render";
    } else if (vdi->dir == PJMEDIA_DIR_CAPTURE) {
        dirname = "capture";
    } else {
        dirname = "render";
    }


    capnames[0] = '\0';
    st_len = 0;
    for (i=0; i<sizeof(int)*8 && (1 << i) < PJMEDIA_VID_DEV_CAP_MAX; ++i) {
        if (vdi->caps & (1 << i)) {
            const char *capname = pjmedia_vid_dev_cap_name(1 << i, NULL);
            if (capname) {
                int tmp_len = (int)strlen(capname);
                if ((int)sizeof(capnames) - st_len <= tmp_len)
                    break;

                st_len += (tmp_len + 2);
                if (*capnames)
                    strcat(capnames, ", ");
                strcat(capnames, capname);
            }
        }
    }

    formats[0] = '\0';
    st_len = 0;
    for (i=0; i<vdi->fmt_cnt; ++i) {
        const pjmedia_video_format_info *vfi =
                pjmedia_get_video_format_info(NULL, vdi->fmt[i].id);
        if (vfi) {
            int tmp_len = (int)strlen(vfi->name);
            if ((int)sizeof(formats) - st_len <= tmp_len) {
                st_len = -1;
                break;
            }

            st_len += (tmp_len + 2);
            if (*formats)
                strcat(formats, ", ");
            strcat(formats, vfi->name);
        }
    }

    pj_cli_out(cval, "%3d %s [%s][%s] %s", id, vdi->name, vdi->driver,
              dirname, title);
    pj_cli_out(cval, "    Supported capabilities: %s", capnames);
    pj_cli_out(cval, "    Supported formats: %s%s", formats,
                              (st_len<0? " ..." : ""));
}
void pjapp_vid_list_devs(pj_cli_cmd_val *cval)
{
    unsigned i, count;
    pjmedia_vid_dev_info vdi;
    pj_status_t status;

    pj_cli_out(cval, "Video device list:");
    count = pjsua_vid_dev_count();
    if (count == 0) {
        pj_cli_out(cval, " - no device detected -");
        return;
    } else {
        pj_cli_out(cval, "%d device(s) detected:", count);
    }

    status = pjsua_vid_dev_get_info(PJMEDIA_VID_DEFAULT_RENDER_DEV, &vdi);
    if (status == PJ_SUCCESS)
        pjapp_vid_print_dev(cval,PJMEDIA_VID_DEFAULT_RENDER_DEV, &vdi,
                      "(default renderer device)");

    status = pjsua_vid_dev_get_info(PJMEDIA_VID_DEFAULT_CAPTURE_DEV, &vdi);
    if (status == PJ_SUCCESS)
        pjapp_vid_print_dev(cval,PJMEDIA_VID_DEFAULT_CAPTURE_DEV, &vdi,
                      "(default capture device)");

    for (i=0; i<count; ++i) {
        status = pjsua_vid_dev_get_info(i, &vdi);
        if (status == PJ_SUCCESS)
            pjapp_vid_print_dev(cval,i, &vdi, "");
    }
}

void pjapp_config_show_video(pj_cli_cmd_val *cval, int acc_id, const pjsua_acc_config *acc_cfg)
{
    pj_cli_out(cval,
              "Account %d:\n"
              "  RX auto show:     %d\n"
              "  TX auto transmit: %d\n"
              "  Capture dev:      %d\n"
              "  Render dev:       %d",
              acc_id,
              acc_cfg->vid_in_auto_show,
              acc_cfg->vid_out_auto_transmit,
              acc_cfg->vid_cap_dev,
              acc_cfg->vid_rend_dev);
}


#endif