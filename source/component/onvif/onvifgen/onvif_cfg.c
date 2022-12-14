#include "soapH.h"
#include <assert.h>
#include "onvif_cfg.h"


struct onvif_config  _onvif_config;

int onvif_video_source_add(struct onvif_VideoSource *vs)
{
    if(_onvif_config.onvif_video_source == NULL)
        _onvif_config.onvif_video_source = vs;
    else
    {
        struct onvif_VideoSource *tmp = _onvif_config.onvif_video_source;
        _onvif_config.onvif_video_source = vs;
        _onvif_config.onvif_video_source->next = tmp;
    }    
    return 0;
}

int onvif_video_source_del(struct onvif_VideoSource *vs)
{
    if(_onvif_config.onvif_video_source == vs)
    {
        struct onvif_VideoSource *tmp = _onvif_config.onvif_video_source->next;
        _onvif_config.onvif_video_source = tmp;
    }
    else
    {
        struct onvif_VideoSource *tmp = _onvif_config.onvif_video_source->next;
        struct onvif_VideoSource *ptmp = _onvif_config.onvif_video_source->next;
        if(_onvif_config.onvif_video_source->next == vs)
        {
            struct onvif_VideoSource *tmp = tmp->next;
            _onvif_config.onvif_video_source->next = tmp;
        }
        while(tmp)
        {
            ptmp = tmp->next;
            if(ptmp == vs)
            {
                tmp->next = ptmp->next;
                break;
            }
        }
    }
    if(vs)
    {
        if(vs->VideoSource.token)
            free(vs->VideoSource.token);
        if(vs->VideoSource.Resolution)
            free(vs->VideoSource.Resolution);
        if(vs->VideoSource.Imaging)
        {
            vs->VideoSource.Imaging->BacklightCompensation;
            if(vs->VideoSource.Imaging->Brightness)
                free(vs->VideoSource.Imaging->Brightness);
    
            if(vs->VideoSource.Imaging->ColorSaturation)
                free(vs->VideoSource.Imaging->ColorSaturation);
            if(vs->VideoSource.Imaging->Contrast)
                free(vs->VideoSource.Imaging->Contrast);

            if(vs->VideoSource.Imaging->Exposure)
                free(vs->VideoSource.Imaging->Exposure);
            if(vs->VideoSource.Imaging->Focus)
                free(vs->VideoSource.Imaging->Focus);
            if(vs->VideoSource.Imaging->IrCutFilter)
                free(vs->VideoSource.Imaging->IrCutFilter);
            if(vs->VideoSource.Imaging->Sharpness)
                free(vs->VideoSource.Imaging->Sharpness);
            if(vs->VideoSource.Imaging->WideDynamicRange)
                free(vs->VideoSource.Imaging->WideDynamicRange);
            if(vs->VideoSource.Imaging->WhiteBalance)
                free(vs->VideoSource.Imaging->WhiteBalance);
            if(vs->VideoSource.Imaging->Extension)
                free(vs->VideoSource.Imaging->Extension);

            soap_del_xsd__anyAttribute(&vs->VideoSource.Imaging->__anyAttribute);

            free(vs->VideoSource.Imaging);
        }
        if(vs->VideoSource.Extension)
        {
            vs->VideoSource.Extension->__any;
            vs->VideoSource.Extension->Imaging;
            vs->VideoSource.Extension->Extension;
            free(vs->VideoSource.Extension);
        }
        soap_del_xsd__anyAttribute(&vs->VideoSource.__anyAttribute);
    }    
    return 0;
}