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
#ifndef AVC_DEC_H_INCLUDED
#define AVC_DEC_H_INCLUDED

#ifndef OMX_Component_h
#include "OMX_Component.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPH264DECODE_H__
#include "SsbSipH264Decode.h"
#endif

//#define MFC_FPS	// for MFC' performance test
#ifdef MFC_FPS
#include <sys/time.h>
#endif

//#define LOG_NDEBUG 0
#define LOG_TAG "OMXAVC_MFC"
#include <utils/Log.h>


#define AVCD_TIMESTAMP_ARRAY_SIZE 17


class AvcDecoder_OMX
{
    public:
#ifdef MFC_FPS
		struct timeval  start, stop;
		unsigned int	time;
		int				frame_cnt;
		int				decode_cnt;
		int				need_cnt;

		unsigned int measureTime(struct timeval *start, struct timeval *stop);
#endif
		AvcDecoder_OMX()  { };
		~AvcDecoder_OMX() { };

		OMX_ERRORTYPE	AvcDecInit_OMX();
		OMX_ERRORTYPE	AvcDecDeinit_OMX();
		OMX_BOOL 		AvcDecodeVideo_OMX(	OMX_U8** aOutBuf, OMX_U32* aOutBufSize, OMX_TICKS* aOutTimestamp,
											OMX_U8** aInBuf, OMX_U32* aInBufSize, OMX_TICKS* aInTimestamp,
											OMX_PARAM_PORTDEFINITIONTYPE* aPortParam,
											OMX_S32* iFrameCount, OMX_BOOL aMarkerFlag,
											OMX_BOOL *aResizeFlag, OMX_BOOL MultiSliceFlag);
		OMX_BOOL      	GetYuv(OMX_U8** aOutBuf, OMX_U32* aOutBufSize, OMX_TICKS* aOutTimestamp);

	private:
		OMX_BOOL      	ResetTimestamp(void);
		OMX_BOOL      	AddTimestamp(OMX_TICKS* time);
		OMX_BOOL      	GetTimestamp(OMX_TICKS* time);

		int             mfc_create    (void);
		int             mfc_dec_slice (unsigned char* data, unsigned int size, OMX_BOOL MultiSliceFlag);
		unsigned char*	mfc_get_yuv   (unsigned int* out_size);
		int             mfc_flag_video_frame(unsigned char* data, int size);

	private:
        OMX_S32 		iDisplay_Width, iDisplay_Height;

        OMX_TICKS		m_time_queue[AVCD_TIMESTAMP_ARRAY_SIZE];
		int           	m_time_queue_start;
		int           	m_time_queue_end;

		void*        	m_mfc_handle;
		unsigned int    m_mfc_buffer_size;
		unsigned char*	m_mfc_buffer_base;
		unsigned char* 	m_mfc_buffer_now;
		int             m_mfc_flag_info_out;
		int             m_mfc_flag_create;

		OMX_U8 			delimiter_h264[4];
};

typedef class AvcDecoder_OMX AvcDecoder_OMX;

#endif	//#ifndef AVC_DEC_H_INCLUDED

