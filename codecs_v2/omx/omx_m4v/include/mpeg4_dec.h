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
#ifndef MPEG4_DEC_H_INCLUDED
#define MPEG4_DEC_H_INCLUDED

#ifndef OMX_Component_h
#include "OMX_Component.h"
#endif

#ifndef __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPMPEG4DECODE_H__
#include "SsbSipMpeg4Decode.h"
#endif

//#define MFC_FPS	// for MFC' performance test
#ifdef MFC_FPS
#include <sys/time.h>
#endif

//#define LOG_NDEBUG 0
#define LOG_TAG "OMXM4V_MFC"
#include <utils/Log.h>


#define M4VD_TIMESTAMP_ARRAY_SIZE 17
//#define MPEG4_HEADER_BUFFER_SIZE 4086

class Mpeg4Decoder_OMX
{
    public:
		enum MFC_CODEC_TYPE
		{
			MFC_CODEC_BASE  = -1,
			MFC_CODEC_MP4D  = 0,
			MFC_CODEC_H263D = 1,
			MFC_CODEC_H264D = 2,
			MFC_CODEC_WMVD  = 3,
			MFC_CODEC_MAX   = 4,
		};

#ifdef MFC_FPS
		struct timeval  start, stop;
		unsigned int	time;
		int				frame_cnt;
		int				decode_cnt;
		int				need_cnt;

		unsigned int measureTime(struct timeval *start, struct timeval *stop);
		
#endif
        Mpeg4Decoder_OMX();

        OMX_ERRORTYPE Mp4DecInit();
        OMX_ERRORTYPE Mp4DecDeinit();
        OMX_BOOL Mp4DecodeVideo(OMX_U8** aOutBuf, OMX_U32* aOutBufSize, OMX_TICKS* aOutTimestamp,
                                OMX_U8** aInBuf, OMX_U32* aInBufSize, OMX_TICKS* aInTimestamp,
                                OMX_PARAM_PORTDEFINITIONTYPE* aPortParam,
                                OMX_S32* aFrameCount, OMX_BOOL aMarkerFlag, 
								OMX_BOOL *aResizeFlag);

		OMX_BOOL      GetYuv(OMX_U8** aOutBuf, OMX_U32* aOutBufSize, OMX_TICKS* aOutTimestamp);
		OMX_BOOL      SetCodecType(int codec_type);

	private:
		OMX_BOOL      	ResetTimestamp(void);
		OMX_BOOL      	AddTimestamp(OMX_TICKS* time);
		OMX_BOOL      	GetTimestamp(OMX_TICKS* time);

		int             mfc_create    (void);
		int             mfc_dec_slice (unsigned char* data, unsigned int size);
		unsigned char*	mfc_get_yuv   (unsigned int* out_size);
		int             mfc_flag_video_frame(unsigned char* data, int size);

    private:	
        OMX_S32 		iDisplay_Width, iDisplay_Height;

		OMX_TICKS     	m_time_queue[M4VD_TIMESTAMP_ARRAY_SIZE];
		int           	m_time_queue_start;
		int           	m_time_queue_end;
		
		void*        	m_mfc_handle;
		int             m_mfc_codec_type; // MFC CODEC_TYPE
		unsigned int    m_mfc_buffer_size;
		unsigned char*	m_mfc_buffer_base;
		unsigned char* 	m_mfc_buffer_now;
		int             m_mfc_flag_info_out;
		int             m_mfc_flag_create;
		int             m_mfc_flag_vol_found;
};


#endif ///#ifndef MPEG4_DEC_H_INCLUDED
