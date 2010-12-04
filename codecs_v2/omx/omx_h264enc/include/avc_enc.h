/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
#ifndef AVC_ENC_H_INCLUDED
#define AVC_ENC_H_INCLUDED


#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OMX_Component_h
// RainAde : changed openmax header for opencore ver. 2.0.3
#include "OMX_Component.h"
//#include "omx_component.h"
#endif

#ifndef __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPH264ENCODE_H__
#include "SsbSipH264Encode.h"
#endif

// RainAde for Encoding pic type
enum
{
    PIC_TYPE_INTRA = 0,
    PIC_TYPE_INTER
};

class AvcEncoder_OMX
{
    public:

        AvcEncoder_OMX();
        ~AvcEncoder_OMX();

        OMX_ERRORTYPE AvcEncInit(OMX_VIDEO_PORTDEFINITIONTYPE aInputParam,
                                 OMX_CONFIG_ROTATIONTYPE aInputOrientationType,
                                 OMX_VIDEO_PORTDEFINITIONTYPE aEncodeParam,
                                 OMX_VIDEO_PARAM_AVCTYPE aEncodeAvcParam,
                                 OMX_VIDEO_PARAM_BITRATETYPE aRateControlType,
                                 OMX_VIDEO_PARAM_QUANTIZATIONTYPE aQuantType,
                                 OMX_VIDEO_PARAM_MOTIONVECTORTYPE aSearchRange,
                                 OMX_VIDEO_PARAM_INTRAREFRESHTYPE aIntraRefresh,
                                 OMX_VIDEO_PARAM_VBSMCTYPE aVbsmcType);


        OMX_BOOL AvcEncodeVideo(OMX_U8*    aOutBuffer,
                                     OMX_U32*   aOutputLength,
                                     OMX_BOOL*  aBufferOverRun,
                                     OMX_U8**   aOverBufferPointer,
                                     OMX_U8*    aInBuffer,
                                     OMX_U32*   aInBufSize,
                                     OMX_TICKS  aInTimeStamp,
                                     OMX_TICKS* aOutTimeStamp,
                                     OMX_BOOL*  aSyncFlag);

        OMX_BOOL AvcEncodeSendInput(OMX_U8*    aInBuffer,
                                    OMX_U32*   aInBufSize,
                                    OMX_TICKS  aInTimeStamp);


        OMX_ERRORTYPE AvcEncDeinit();

        OMX_ERRORTYPE AvcRequestIFrame();
        OMX_BOOL AvcUpdateBitRate(OMX_U32 aEncodedBitRate);
        OMX_BOOL AvcUpdateFrameRate(OMX_U32 aEncodeFramerate);

    private:

	// RainAde : for MFC(avc) encoder
	int m_avcenc_create_flag;
	void *m_avcenc_handle;
	unsigned char * m_avcenc_buffer;

	// RainAde : for composer interface
	int frame_cnt;
	int hdr_size;
	int sps;
	int pps;

};


#endif ///#ifndef AVC_ENC_H_INCLUDED
