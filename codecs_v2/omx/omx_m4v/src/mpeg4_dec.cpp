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
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#include "mpeg4_dec.h"
#include "oscl_mem.h"
#include "omx_mpeg4_component.h"


Mpeg4Decoder_OMX::Mpeg4Decoder_OMX()
{
}

/* Initialization routine */
OMX_ERRORTYPE Mpeg4Decoder_OMX::Mp4DecInit()
{
#ifdef MFC_FPS
	time = 0;
	frame_cnt = 0;
	decode_cnt = 0;
	need_cnt = 0;
#endif

    iDisplay_Width = 0;
    iDisplay_Height = 0;

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

int Mpeg4Decoder_OMX::mfc_create(void)
{
	if(m_mfc_flag_create == 1)
	{
		LOGE("mfc_create aleady done \n");
		return 0;
	}

	m_mfc_handle = SsbSipMPEG4DecodeInit();
	if (m_mfc_handle == NULL)
	{
		LOGE("SsbSipMPEG4DecodeInit Failed.\n");
		return -1;
	}

	m_mfc_buffer_base = (unsigned char*)SsbSipMPEG4DecodeGetInBuf(m_mfc_handle, 0);

	m_mfc_buffer_now    	= m_mfc_buffer_base;
	m_mfc_buffer_size   	= 0;	
	m_mfc_flag_info_out 	= 0;
	m_mfc_flag_create   	= 1;
	m_mfc_flag_vol_found	= 0;
					
    return 0;
}

OMX_BOOL Mpeg4Decoder_OMX::SetCodecType(int codec_type)
{
	m_mfc_codec_type = codec_type;
	return OMX_TRUE;
}

/*Decode routine */
OMX_BOOL Mpeg4Decoder_OMX::Mp4DecodeVideo(
								OMX_U8**  	aOutBuf, 
								OMX_U32*  	aOutBufSize,
								OMX_TICKS* 	aOutTimestamp,
        						OMX_U8**  	aInBuf, 
								OMX_U32*  	aInBufSize,
								OMX_TICKS*	aInTimestamp,
        						OMX_PARAM_PORTDEFINITIONTYPE* aPortParam,
        						OMX_S32*  	aFrameCount, 
								OMX_BOOL  	aMarkerFlag, 
								OMX_BOOL* 	aResizeFlag)
{
    OMX_BOOL Status = OMX_TRUE;
	int ret = 0;

	unsigned char*	pNalBuffer	= *aInBuf;
	int 			NalSize		= *aInBufSize;

    *aResizeFlag = OMX_FALSE;

#ifdef MFC_FPS
	gettimeofday(&start, NULL);
	decode_cnt++;
#endif

	ret = mfc_dec_slice(pNalBuffer, NalSize);

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

			(*aFrameCount)++;
							
			Status = OMX_TRUE;
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
		*aFrameCount = 0;
		*aOutBufSize = 0;

		return OMX_FALSE;
	}

	return OMX_TRUE;
}

OMX_BOOL Mpeg4Decoder_OMX::GetYuv(OMX_U8** aOutBuf, OMX_U32* aOutBufSize, OMX_TICKS* aOutTimestamp)
{
	unsigned char*	out_data_virt = NULL;
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

OMX_ERRORTYPE Mpeg4Decoder_OMX::Mp4DecDeinit()
{
#ifdef MFC_FPS
	LOGV("[[ MFC_FPS ]] Decoding Time: %u, Frame Count: %d, Need Count: %d, FPS: %f\n", time, frame_cnt-1, need_cnt, (float)(frame_cnt-1)*1000/time);
#endif

	if(m_mfc_flag_create == 0)
	{
		return OMX_ErrorNone;
	}

	if(SsbSipMPEG4DecodeDeInit(m_mfc_handle) < 0)
	{
		LOGE("SsbSipMPEG4DecodeDeInit\n");
		return OMX_ErrorUndefined;
	}

	m_mfc_buffer_base   	= NULL;
	m_mfc_buffer_now    	= NULL;
	m_mfc_buffer_size   	= 0;
	m_mfc_flag_info_out 	= 0;
	m_mfc_flag_create   	= 0;
	m_mfc_flag_vol_found	= 0;

    return OMX_ErrorNone;
}

OMX_BOOL Mpeg4Decoder_OMX::ResetTimestamp(void)
{
	memset(m_time_queue, 0, sizeof(OMX_TICKS)*M4VD_TIMESTAMP_ARRAY_SIZE);

	m_time_queue_start = 0;
	m_time_queue_end   = 0;
	
	return OMX_TRUE;
}

OMX_BOOL Mpeg4Decoder_OMX::AddTimestamp(OMX_TICKS * timeStamp)
{
	m_time_queue[m_time_queue_end] = *timeStamp;

	// RainAde : changed for bug fix -> index should not be over AVCD_TIMESTAMP_ARRAY_SIZE
	//if(M4VD_TIMESTAMP_ARRAY_SIZE <= m_time_queue_end)
	if(M4VD_TIMESTAMP_ARRAY_SIZE-1 <= m_time_queue_end)
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

OMX_BOOL Mpeg4Decoder_OMX::GetTimestamp(OMX_TICKS * timeStamp)
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
	//if(M4VD_TIMESTAMP_ARRAY_SIZE <= m_time_queue_start)
	if(M4VD_TIMESTAMP_ARRAY_SIZE-1 <= m_time_queue_start)
		m_time_queue_start = 0;
	else
		m_time_queue_start++;

	// RainAde : changed for bug fix -> data is not delivered properly
	//timeStamp = &m_time_queue[index];
	*timeStamp = m_time_queue[index];

	return OMX_TRUE;
}

int Mpeg4Decoder_OMX::mfc_dec_slice(unsigned char* data, unsigned int size)
{
	SSBSIP_MPEG4_STREAM_INFO m4vd_info;
	int flag_video_frame = 0;
	unsigned char* pData;
	int i;
	bool bFindVop = false;

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

	if(flag_video_frame == 1)
	{
		memcpy(m_mfc_buffer_now, data, size);
		m_mfc_buffer_now  += size;
		m_mfc_buffer_size += size;
	
		// RainAde : add for debugging (case: VOL -> VOL+VOP -> VOP... )
		if(m_mfc_flag_vol_found == 1)
		{
			if(m_mfc_flag_info_out == 0)
			{
				if(SsbSipMPEG4DecodeExe(m_mfc_handle, m_mfc_buffer_size) < 0)
				{
					LOGE("SsbSipMPEG4DecodeExe for GetConfig fail \n");
					return -1;
				}

				if(SsbSipMPEG4DecodeGetConfig(m_mfc_handle, MPEG4_DEC_GETCONF_STREAMINFO, &m4vd_info) < 0)
				{
					LOGE("SsbSipMPEG4DecodeGetConfig fail\n");
					return -1;
				}
			
				iDisplay_Width  = m4vd_info.width;
				iDisplay_Height = m4vd_info.height;

				m_mfc_flag_info_out =  1;
			}

			if(SsbSipMPEG4DecodeExe(m_mfc_handle, m_mfc_buffer_size) < 0)
			{
				LOGE("SsbSipMPEG4DecodeExe(Main) fail \n");
				return -1;
			}

			m_mfc_buffer_now  = m_mfc_buffer_base;
			m_mfc_buffer_size = 0;
		}
		else	// RainAde : add for debugging (case: VOP without VOL ) 
		{
			LOGE("Receieved VOP before getting VOL header \n");
			return -1;
		}
	}
	else
	{		
		// case: VOL -> VOL+VOP -> VOP... 
		if(m_mfc_flag_vol_found == 0)
		{
			memcpy(m_mfc_buffer_now, data, size);
			m_mfc_buffer_now  += size;
			m_mfc_buffer_size += size;
			
			m_mfc_flag_vol_found = 1;
		}
		else
		{
			for(pData = data, i = 0; i < (size-3); i++, pData++)
			{
				if(0x00 == *pData)
					if(0x00 == *(pData+1))
						if(0x01 == *(pData+2))
							if(0xb3 == *(pData+3) || 0xb6 == *(pData+3))
							{
								bFindVop = true;
								break;
							}
			}

			if(bFindVop)
			{
				memcpy(m_mfc_buffer_now, pData, size-i);
				m_mfc_buffer_now  += (size-i);
				m_mfc_buffer_size += (size-i);
			
				// RainAde : add for debugging (case: VOL -> VOL+VOP -> VOP... )
				if(m_mfc_flag_info_out == 0)
				{
					if(SsbSipMPEG4DecodeExe(m_mfc_handle, m_mfc_buffer_size) < 0)
					{
						LOGE("SsbSipMPEG4DecodeExe for GetConfig fail \n");
						return -1;
					}

					if(SsbSipMPEG4DecodeGetConfig(m_mfc_handle, MPEG4_DEC_GETCONF_STREAMINFO, &m4vd_info) < 0)
					{
						LOGE("SsbSipMPEG4DecodeGetConfig fail\n");
						return -1;
					}
			
					iDisplay_Width  = m4vd_info.width;
					iDisplay_Height = m4vd_info.height;

					m_mfc_flag_info_out =  1;
				}

				if(SsbSipMPEG4DecodeExe(m_mfc_handle, m_mfc_buffer_size) < 0)
				{
					LOGE("SsbSipMPEG4DecodeExe(Main 2) fail \n");
					return -1;
				}
			}
			
			m_mfc_buffer_now  = m_mfc_buffer_base;
			m_mfc_buffer_size = 0;

			return 1;
		}

		return 0;
	}
	
	return 1;
}

int Mpeg4Decoder_OMX::mfc_flag_video_frame(unsigned char* data, int size)
{
	int flag_video_frame = 0; // 0 : only header  1 : I frame include

	switch(m_mfc_codec_type)
	{
		case MFC_CODEC_MP4D:
		{
			if (4 <= size)
			{
				if(0xb6 == (int)data[3] || 0xb3 == (int)data[3])
					flag_video_frame = 1;
				else
					flag_video_frame = 0;
			}
			else
				return -1;

			break;
		}
		case MFC_CODEC_H263D:
		{
			flag_video_frame = 1;
			m_mfc_flag_vol_found = 1;
			break;
		}
		default:
			break;
	}

	return flag_video_frame;
}

unsigned char* Mpeg4Decoder_OMX::mfc_get_yuv(unsigned int* out_size)
{
	unsigned char*	pYUVBuf = NULL;	// memory address of YUV420 Frame Buffer
	long	        nYUVLeng = 0;	// size of frame buffer	

	pYUVBuf = (unsigned char *)SsbSipMPEG4DecodeGetOutBuf(m_mfc_handle, &nYUVLeng);	

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

#ifdef MFC_FPS
unsigned int Mpeg4Decoder_OMX::measureTime(struct timeval *start, struct timeval *stop)
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
