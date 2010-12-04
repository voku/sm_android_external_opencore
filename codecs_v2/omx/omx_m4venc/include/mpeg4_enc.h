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
#ifndef MPEG4_ENC_H_INCLUDED
#define MPEG4_ENC_H_INCLUDED

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OMX_Component_h
// RainAde: changed openmax header for opencore ver. 2.0.3
//#include "omx_component.h"
#include "OMX_Component.h"
#endif

#ifndef __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPMPEG4ENCODE_H__
#include "SsbSipMpeg4Encode.h"
#endif

enum
{
    MODE_H263 = 0,
    MODE_MPEG4
};

// RainAde for Encoding pic type
enum
{
    PIC_TYPE_INTRA = 0,
    PIC_TYPE_INTER
};

class Mpeg4Encoder_OMX
{
    public:

        Mpeg4Encoder_OMX();
        ~Mpeg4Encoder_OMX();
		
        OMX_ERRORTYPE Mp4EncInit(OMX_S32 iEncMode,
                                 OMX_VIDEO_PORTDEFINITIONTYPE aInputParam,
                                 OMX_CONFIG_ROTATIONTYPE aInputOrientationType,
                                 OMX_VIDEO_PORTDEFINITIONTYPE aEncodeParam,
                                 OMX_VIDEO_PARAM_MPEG4TYPE aEncodeMpeg4Param,
                                 OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE aErrorCorrection,
                                 OMX_VIDEO_PARAM_BITRATETYPE aRateControlType,
                                 OMX_VIDEO_PARAM_QUANTIZATIONTYPE aQuantType,
                                 OMX_VIDEO_PARAM_MOTIONVECTORTYPE aSearchRange,
                                 OMX_VIDEO_PARAM_INTRAREFRESHTYPE aIntraRefresh,
                                 OMX_VIDEO_PARAM_H263TYPE aH263Type,
                                 OMX_VIDEO_PARAM_PROFILELEVELTYPE* aProfileLevel);

        OMX_ERRORTYPE Mp4EncDeinit();

        OMX_BOOL Mp4EncodeVideo(OMX_U8*    aOutBuffer,
                                OMX_U32*   aOutputLength,
                                OMX_BOOL*  aBufferOverRun,
                                OMX_U8**   aOverBufferPointer,
                                OMX_U8*    aInBuffer,
                                OMX_U32    aInBufSize,
                                OMX_TICKS  aInTimeStamp,
                                OMX_TICKS* aOutTimeStamp,
                                OMX_BOOL*  aSyncFlag);

        OMX_ERRORTYPE Mp4RequestIFrame();
        OMX_BOOL Mp4UpdateBitRate(OMX_U32 aEncodedBitRate);
        OMX_BOOL Mp4UpdateFrameRate(OMX_U32 aEncodeFramerate);

    private:

	// RainAde : for MFC(mpeg4/h263) encoder
        int m_mpeg4enc_create_flag;
        void *m_mpeg4enc_handle;
        unsigned char * m_mpeg4enc_buffer;

	// RainAde : for composer interface
	int frame_cnt;
	int hdr_size;

};

#endif ///#ifndef MPEG4_ENC_H_INCLUDED
