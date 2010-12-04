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
#include "oscl_types.h"
#include "avc_dec.h"

/* Initialization routine */
OMX_ERRORTYPE AvcDecoder_OMX::AvcDecInit_OMX()
{
#ifdef MFC_FPS
	time = 0;
	frame_cnt = 0;
	decode_cnt = 0;
	need_cnt = 0;
#endif

    iDisplay_Width = 0;
    iDisplay_Height = 0;

	delimiter_h264[0] = 0x00;
	delimiter_h264[1] = 0x00;
	delimiter_h264[2] = 0x00;
	delimiter_h264[3] = 0x01;

	if(ResetTimestamp() == OMX_FALSE)
	{
		LOGE("ResetTimestamp fail\n");
		return OMX_ErrorUndefined;
	}
	
	if(mfc_create() < 0)
	{
		LOGE("mfc_create fail\n");
		return OMX_ErrorUndefined;
	}

    return OMX_ErrorNone;
}

int AvcDecoder_OMX::mfc_create(void)
{
	if(m_mfc_flag_create == 1)
	{
		LOGE("mfc_create aleady done \n");
		return 0;
	}

	m_mfc_handle = SsbSipH264DecodeInit();
	if (m_mfc_handle == NULL)
	{
		LOGE("SsbSipH264DecodeInit Failed.\n");
		return -1;
	}

	m_mfc_buffer_base = (unsigned char*)SsbSipH264DecodeGetInBuf(m_mfc_handle, 0);

	m_mfc_buffer_now    = m_mfc_buffer_base;
	m_mfc_buffer_size   = 0;	
	m_mfc_flag_info_out = 0;
	m_mfc_flag_create   = 1;
					
    return 0;
}

/*Decode routine */
/* yj: Added MultiSliceFlag for multi-slice decoding */
OMX_BOOL AvcDecoder_OMX::AvcDecodeVideo_OMX(
							OMX_U8** 	aOutBuf, 
							OMX_U32* 	aOutBufSize,
							OMX_TICKS* 	aOutTimestamp,
        					OMX_U8** 	aInBuf, 
							OMX_U32* 	aInBufSize,
							OMX_TICKS*	aInTimestamp,
        					OMX_PARAM_PORTDEFINITIONTYPE* aPortParam,
        					OMX_S32* 	iFrameCount, 
							OMX_BOOL 	aMarkerFlag, 
							OMX_BOOL 	*aResizeFlag,
							OMX_BOOL	MultiSliceFlag)
{
    OMX_BOOL Status = OMX_TRUE;
	int ret = 0;

	unsigned char*	pNalBuffer	= *aInBuf;
	int 			NalSize		= *aInBufSize;

	// RainAde for thumbnail
	//*aInBufSize  = 0;
    *aResizeFlag = OMX_FALSE;

#ifdef MFC_FPS
	gettimeofday(&start, NULL);
	decode_cnt++;
#endif

	ret = mfc_dec_slice(pNalBuffer, NalSize, MultiSliceFlag);

	switch(ret)
	{
		case -1 :
		{
			Status = OMX_FALSE;
			LOGE("mfc_dec_slice fail\n");
			break;
		}
		case 0:
		{
#ifdef MFC_FPS
			need_cnt++;
#endif
			Status = OMX_TRUE;
	        // RainAde for thumbnail
			*aInBufSize  = 0;
			break;
		}
		case 1 :
		{
#ifdef MFC_FPS
			gettimeofday(&stop, NULL);
			time += measureTime(&start, &stop);
			frame_cnt++;
#endif

			if(aPortParam->format.video.nFrameWidth  != (unsigned int)iDisplay_Width
			|| aPortParam->format.video.nFrameHeight != (unsigned int)iDisplay_Height)
			{
				LOGV("iDisplay_Width : %d\n", (int)iDisplay_Width);
				LOGV("iDisplay_Height: %d\n", (int)iDisplay_Height);

				if(iDisplay_Width > 720)
				{
					Status = OMX_FALSE;
					break;
				}
					
				aPortParam->format.video.nFrameWidth  = iDisplay_Width;
				aPortParam->format.video.nFrameHeight = iDisplay_Height;

				*aResizeFlag = OMX_TRUE;
				Status = OMX_TRUE;
				break;
			}
				
			if(AddTimestamp(aInTimestamp) == OMX_FALSE)
			{
				LOGE("AddTimestamp fail\n");
			    return OMX_FALSE;
			}
			
			if(GetYuv(aOutBuf, aOutBufSize, aOutTimestamp) == OMX_FALSE)
			{
				Status = OMX_FALSE;
				break;
			}

			(*iFrameCount)++;
							
			Status = OMX_TRUE;
	        // RainAde for thumbnail
			*aInBufSize  = 0;
			break;
		}
		default :
		{
			Status = OMX_FALSE;
			LOGE("UnExpected Operation\n");
			break;
		}
	}

	if(Status == OMX_FALSE)
	{
		*iFrameCount = 0;
		*aOutBufSize = 0;

		return OMX_FALSE;
	}

	return OMX_TRUE;
}

OMX_BOOL AvcDecoder_OMX::GetYuv(OMX_U8** aOutBuf, OMX_U32* aOutBufSize, OMX_TICKS* aOutTimestamp)
{
	unsigned char*  out_data_virt = NULL;
	unsigned int    out_size      = 0;
				
	out_data_virt = mfc_get_yuv(&out_size);
				
	if(out_data_virt)
	{
		*aOutBuf     = out_data_virt;
		*aOutBufSize = out_size;
				
		if(GetTimestamp(aOutTimestamp) == OMX_FALSE)
			LOGE("GetTimestamp fail\n");
	}
	else
	{
		//*aOutBufSize   = 0;
		//*aOutTimestamp = 0;
		return OMX_FALSE;
	}

	return OMX_TRUE;
}
				
OMX_BOOL AvcDecoder_OMX::ResetTimestamp(void)
{
	memset(m_time_queue, 0, sizeof(OMX_TICKS)*AVCD_TIMESTAMP_ARRAY_SIZE);

	m_time_queue_start = 0;
	m_time_queue_end   = 0;
	
	return OMX_TRUE;
}

OMX_BOOL AvcDecoder_OMX::AddTimestamp(OMX_TICKS* timeStamp)
{
	m_time_queue[m_time_queue_end] = *timeStamp;

	// RainAde : changed for bug fix -> index should not be over AVCD_TIMESTAMP_ARRAY_SIZE
	//if(AVCD_TIMESTAMP_ARRAY_SIZE <= m_time_queue_end)	
	if(AVCD_TIMESTAMP_ARRAY_SIZE-1 <= m_time_queue_end)
		m_time_queue_end = 0;
	else
		m_time_queue_end++;

	if(m_time_queue_end == m_time_queue_start)
	{
		LOGE("TIME QUEUE IS OVERFLOWED\n");
		return OMX_FALSE;
	}

	return OMX_TRUE;
}

OMX_BOOL AvcDecoder_OMX::GetTimestamp(OMX_TICKS* timeStamp)
{
	int index = 0;

	if(m_time_queue_end == m_time_queue_start)
	{
		LOGE("TIME QUEUE IS UNDERFLOWED m_time_queue_end : %d / m_time_queue_start : %d\n", m_time_queue_end, m_time_queue_start);
		return OMX_FALSE;
	}
	else
		index = m_time_queue_start;

	// RainAde : changed for bug fix -> index should not be over AVCD_TIMESTAMP_ARRAY_SIZE
	//if(AVCD_TIMESTAMP_ARRAY_SIZE <= m_time_queue_start)
	if(AVCD_TIMESTAMP_ARRAY_SIZE-1 <= m_time_queue_start)
		m_time_queue_start = 0;
	else
		m_time_queue_start++;

	// RainAde : changed for bug fix -> data is not delivered properly
	//timeStamp = &m_time_queue[index];
	*timeStamp = m_time_queue[index];

	return OMX_TRUE;
}

int AvcDecoder_OMX::mfc_dec_slice(unsigned char* data, unsigned int size, OMX_BOOL MultiSliceFlag)
{
	SSBSIP_H264_STREAM_INFO avcd_info;
	int flag_video_frame = 0;

	if(m_mfc_flag_create == 0)
	{
		LOGE("mfc codec not yes created \n");
		return -1;
	}

	flag_video_frame = mfc_flag_video_frame(data, size);
	if(flag_video_frame < 0)
	{
		LOGE("mfc_flag_video_frame error \n");
		return -1;
	}

	// yj: multi-slice
	if(flag_video_frame == 1 && MultiSliceFlag == OMX_TRUE)
		flag_video_frame = 0;

	if(flag_video_frame == 1)
	{
		memcpy(m_mfc_buffer_now, delimiter_h264, 4);
		m_mfc_buffer_now  += 4;
		m_mfc_buffer_size += 4;

		memcpy(m_mfc_buffer_now, data, size);
		m_mfc_buffer_now  += size;
		m_mfc_buffer_size += size;
	
		if(m_mfc_flag_info_out == 0)
		{
			if(SsbSipH264DecodeExe(m_mfc_handle, m_mfc_buffer_size) < 0)
			{
				LOGE("SsbSipH264DecodeExe for GetConfig fail \n");
				return -1;
			}

			if(SsbSipH264DecodeGetConfig(m_mfc_handle, H264_DEC_GETCONF_STREAMINFO, &avcd_info) < 0)
			{
				LOGE("SsbSipH264DecodeGetConfig fail\n");
				return -1;
			}
			
			iDisplay_Width  = avcd_info.width;
			iDisplay_Height = avcd_info.height;

			m_mfc_flag_info_out =  1;
		}

		if(SsbSipH264DecodeExe(m_mfc_handle, m_mfc_buffer_size) < 0)
		{
			LOGE("SsbSipH264DecodeExe(Main) fail \n");
			return -1;
		}

		m_mfc_buffer_now  = m_mfc_buffer_base;
		m_mfc_buffer_size = 0;
	}
	else
	{		
		memcpy(m_mfc_buffer_now, delimiter_h264, 4);
		m_mfc_buffer_now  += 4;
		m_mfc_buffer_size += 4;

		memcpy(m_mfc_buffer_now, data, size);
		m_mfc_buffer_now  += size;
		m_mfc_buffer_size += size;

		return 0;
	}
	
	return 1;
}

int AvcDecoder_OMX::mfc_flag_video_frame(unsigned char* data, int size)
{
	int flag_video_frame = 0; // 0 : only header  1 : I frame include
	int forbidden_zero_bit;

	if (size > 0)
	{
		forbidden_zero_bit = data[0] >> 7;
		if (forbidden_zero_bit != 0)
			return -1;

		if(	   1 == (data[0] & 0x1F)   // NALTYPE_SLICE
			|| 5 == (data[0] & 0x1F))  // NALTYPE_IDR
		{
			flag_video_frame = 1;
		}
		else
			flag_video_frame = 0;
	}

	return flag_video_frame;
}

unsigned char* AvcDecoder_OMX::mfc_get_yuv(unsigned int* out_size)
{
	unsigned char*	pYUVBuf = NULL;	// memory address of YUV420 Frame Buffer
	long	        nYUVLeng = 0;	// size of frame buffer	

	pYUVBuf = (unsigned char *)SsbSipH264DecodeGetOutBuf(m_mfc_handle, &nYUVLeng);

	if(pYUVBuf)
	{
		*out_size = (unsigned int)nYUVLeng;
	}
	else
	{
		*out_size  = 0;
	}

	return pYUVBuf;
}



OMX_ERRORTYPE AvcDecoder_OMX::AvcDecDeinit_OMX()
{
#ifdef MFC_FPS
	LOGV("[[ MFC_FPS ]] Decoding Time: %u, Frame Count: %d, Need Count: %d, FPS: %f\n", time, frame_cnt-1, need_cnt, (float)(frame_cnt-1)*1000/time);
#endif

	if(m_mfc_flag_create == 0)
	{
		return OMX_ErrorNone;
	}

	if(SsbSipH264DecodeDeInit(m_mfc_handle) < 0)
	{
		LOGE("SsbSipH264DecodeDeInit\n");
		return OMX_ErrorUndefined;
	}

	m_mfc_buffer_base   = NULL;
	m_mfc_buffer_now    = NULL;
	m_mfc_buffer_size   = 0;
	m_mfc_flag_info_out = 0;
	m_mfc_flag_create   = 0;

    return OMX_ErrorNone;
}

#ifdef MFC_FPS
unsigned int AvcDecoder_OMX::measureTime(struct timeval *start, struct timeval *stop)
{
	unsigned int sec, usec, time;

	sec = stop->tv_sec - start->tv_sec;
	if(stop->tv_usec >= start->tv_usec)
	{
		usec = stop->tv_usec - start->tv_usec;
	}
   	else
	{	
		usec = stop->tv_usec + 1000000 - start->tv_usec;
		sec--;
  	}
	time = sec*1000 + ((double)usec)/1000;
	return time;
}
#endif


