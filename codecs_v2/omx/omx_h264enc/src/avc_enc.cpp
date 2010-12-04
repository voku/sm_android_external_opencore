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

//#define LOG_NDEBUG 0
#define LOG_TAG "Mfc_Avc_enc"
#include <utils/Log.h>

#include "avc_enc.h"

//Class constructor function
AvcEncoder_OMX::AvcEncoder_OMX()
{
};


//Class destructor function
AvcEncoder_OMX::~AvcEncoder_OMX()
{
};


/* Encoder Initialization routine */
OMX_ERRORTYPE AvcEncoder_OMX::AvcEncInit(OMX_VIDEO_PORTDEFINITIONTYPE aInputParam,
        OMX_CONFIG_ROTATIONTYPE aInputOrientationType,
        OMX_VIDEO_PORTDEFINITIONTYPE aEncodeParam,
        OMX_VIDEO_PARAM_AVCTYPE aEncodeAvcParam,
        OMX_VIDEO_PARAM_BITRATETYPE aRateControlType,
        OMX_VIDEO_PARAM_QUANTIZATIONTYPE aQuantType,
        OMX_VIDEO_PARAM_MOTIONVECTORTYPE aSearchRange,
        OMX_VIDEO_PARAM_INTRAREFRESHTYPE aIntraRefresh,
        OMX_VIDEO_PARAM_VBSMCTYPE aVbsmcType)
{
	LOGV("MFC: AvcEncoder_OMX::AvcEncInit()");

	frame_cnt = 0;
	hdr_size = 0;
	sps = 0;
	pps = 0;

	int ret = 0;
	int slices[2];

	unsigned char *p_output_buffer = NULL;
	long encoded_size = 0;
	int gop_num = 0;
	
	gop_num = aEncodeParam.xFramerate >> 16;	// temporal value by RainAde : I frame every 1sec
	
	if(m_avcenc_create_flag == 1)
	{
		LOGV("s3c6410_mpeg4enc_create == 1 \n");
		return OMX_ErrorUndefined;
	}

	LOGV("aEncodeParam.nFrameWidth = %d", aEncodeParam.nFrameWidth);
	LOGV("aEncodeParam.nFrameHeight = %d", aEncodeParam.nFrameHeight);

	// Open MFC
	m_avcenc_handle = SsbSipH264EncodeInit(aEncodeParam.nFrameWidth, aEncodeParam.nFrameHeight, aEncodeParam.xFramerate >> 16, aEncodeParam.nBitrate/1000, gop_num);
	slices[0] = 0;	// 0 for single, 1 for multiple
	slices[1] = 4;	// count of slices
	SsbSipH264EncodeSetConfig(m_avcenc_handle, H264_ENC_SETCONF_NUM_SLICES, slices);
				  
	if (m_avcenc_handle == NULL) {
		LOGV("m_avcenc_handle == NULL \n");
		return OMX_ErrorInsufficientResources;
	}

	// Init MFC
	ret = SsbSipH264EncodeExe(m_avcenc_handle);
	if(ret != SSBSIP_H264_ENC_RET_OK){
		LOGV("Error : SsbSipH264EncodeExe(before GetInBuf) \n");
		return OMX_ErrorUndefined;
	}

	// Get input buffer from MFC
	m_avcenc_buffer = (unsigned char *)SsbSipH264EncodeGetInBuf(m_avcenc_handle, 0);
	if (m_avcenc_buffer == NULL) {
		LOGV("m_avcenc_buffer == NULL \n");
		return OMX_ErrorInsufficientResources;
	}

	m_avcenc_create_flag = 1;

	return OMX_ErrorNone;	
}


/*Encode routine */
OMX_BOOL AvcEncoder_OMX::AvcEncodeVideo(OMX_U8* aOutBuffer,
        OMX_U32*   aOutputLength,
        OMX_BOOL*  aBufferOverRun,
        OMX_U8**   aOverBufferPointer,
        OMX_U8*    aInBuffer,
        OMX_U32*   aInBufSize,
        OMX_TICKS  aInTimeStamp,
        OMX_TICKS* aOutTimeStamp,
        OMX_BOOL*  aSyncFlag)
{
//	LOGV("MFC: Avcncoder_OMX::AvcEncodeVideo() \n");

	unsigned char *p_output_buffer = NULL;
	long encoded_size = 0;
	int ret;
	int tmp;
// RainAde for Encodig pic type
	int pic_type;
	
	// TimeStamp : bypass
	*aOutTimeStamp = aInTimeStamp;
	
	// copy YUV data from OMX component src buffer to MFC src buffer
	// input data from Real camera
	if(frame_cnt != 1 && frame_cnt != 2) // 1: PPS, 2: 1st I frame
		memcpy(m_avcenc_buffer, aInBuffer, *aInBufSize);

	// *** Encoding
	if(frame_cnt != 1 && frame_cnt != 2) // 1: PPS, 2: 1st I frame
	{
		ret = SsbSipH264EncodeExe(m_avcenc_handle);
		if(ret != SSBSIP_H264_ENC_RET_OK ) {
			LOGV("Error : SsbSipMPEG4EncodeExe \n");
			return OMX_FALSE;
		}
	}
	
	// get dst buffer address(stored Encoded data) and size(Encoded size)
	p_output_buffer = (unsigned char *)SsbSipH264EncodeGetOutBuf(m_avcenc_handle, &encoded_size);
	if (p_output_buffer == NULL) {
		LOGV("p_output_buffer == NULL \n");
		return OMX_FALSE;
	}

	if(frame_cnt == 0)	// SPS
	{
		SsbSipH264EncodeGetConfig(m_avcenc_handle, H264_ENC_GETCONF_HEADER_SIZE, &hdr_size);
		SsbSipH264EncodeGetConfig(m_avcenc_handle, H264_ENC_GETCONF_SPS_SIZE, &sps);
		SsbSipH264EncodeGetConfig(m_avcenc_handle, H264_ENC_GETCONF_PPS_SIZE, &pps);		
	}	

	if(frame_cnt == 0) // 0: SPS
		encoded_size = sps;
	else if(frame_cnt == 1) // 1: PPS
		encoded_size = pps;
	else if(frame_cnt == 2) // 2: 1st I frame
		encoded_size -= hdr_size;

	if(*aOutputLength < encoded_size) {
		LOGV("output buffer is smaller than encoded data size \n");
		return OMX_FALSE;
	}
		
	// copy Encoded data from MFC dst buffer to OMX component dst buffer as much as the Encoded data size 
	*aOutputLength = (OMX_U32)encoded_size-4; // remove startcode 0x000001

	if(frame_cnt == 1)	// PPS
		memcpy(aOutBuffer, p_output_buffer+sps+4, *aOutputLength);
	else if(frame_cnt == 2) // 1st I frame
		memcpy(aOutBuffer, p_output_buffer+hdr_size+4, *aOutputLength);
	else
		memcpy(aOutBuffer, p_output_buffer+4, *aOutputLength); // SPS and other frames

// RainAde for Encoding pic type
	SsbSipH264EncodeGetConfig(m_avcenc_handle, H264_ENC_GETCONF_PIC_TYPE, &pic_type);

// RainAde for thumbnail
	if(pic_type == PIC_TYPE_INTRA)
		*aSyncFlag = OMX_TRUE;
	else
		*aSyncFlag = OMX_FALSE;
	
	LOGV("pic_type = %d, *aSyncFlag = %d", pic_type, *aSyncFlag);

	if(frame_cnt++ > 1) // neither SPS nor PPS
		*aInBufSize = 0;
		
	return OMX_TRUE;

}


//Deinitialize the avc encoder here and perform the cleanup and free operations
OMX_ERRORTYPE AvcEncoder_OMX::AvcEncDeinit()
{
	LOGV("MFC: AvcEncoder_OMX::AvcEncDeinit() \n");

	int ret;
	
	if(m_avcenc_create_flag == 0)
	{
		LOGV("s3c6410_avcenc_create == 0\n");
		return OMX_ErrorUndefined;
	}

	ret = SsbSipH264EncodeDeInit(m_avcenc_handle);
	if(ret != SSBSIP_H264_ENC_RET_OK ) {
		LOGV("Error : SsbSipMPEG4EncodeDeInit\n");
		return OMX_ErrorUndefined;
	}
	
	m_avcenc_buffer = NULL;
	m_avcenc_create_flag = 0;

    return OMX_ErrorNone;
}


//Request for an I frame while encoding is in process
OMX_ERRORTYPE AvcEncoder_OMX::AvcRequestIFrame()
{
	int para_change[2];
	int ret;

	para_change[0] = H264_ENC_PIC_OPT_IDR;
	para_change[1] = 1;

	ret = SsbSipH264EncodeSetConfig(m_avcenc_handle, H264_ENC_SETCONF_CUR_PIC_OPT, para_change);
	
	if(ret != SSBSIP_H264_ENC_RET_OK){
		LOGV("Error : SsbSipMPEG4EncodeSetConfig(Request I frame) \n");
		return OMX_ErrorUndefined;
	}

    return OMX_ErrorNone;
}

//Routine to update bitrate dynamically when encoding is in progress
OMX_BOOL AvcEncoder_OMX::AvcUpdateBitRate(OMX_U32 aEncodedBitRate)
{
	int para_change[2];
	int ret;

	para_change[0] = H264_ENC_PARAM_BITRATE;
	para_change[1] = aEncodedBitRate/1000;

	ret = SsbSipH264EncodeSetConfig(m_avcenc_handle, H264_ENC_SETCONF_PARAM_CHANGE, para_change);

	if(ret != SSBSIP_H264_ENC_RET_OK){
		LOGV("Error : SsbSipMPEG4EncodeSetConfig(Bitrate) \n");
		return OMX_FALSE;
	}

    return OMX_TRUE;
}

//Routine to update frame rate dynamically when encoding is in progress
OMX_BOOL AvcEncoder_OMX::AvcUpdateFrameRate(OMX_U32 aEncodeFramerate)
{
	int para_change[2];
	int ret;

	para_change[0] = H264_ENC_PARAM_F_RATE;
	para_change[1] = aEncodeFramerate >> 16;

	ret = SsbSipH264EncodeSetConfig(m_avcenc_handle, H264_ENC_SETCONF_PARAM_CHANGE, para_change);

	if(ret != SSBSIP_H264_ENC_RET_OK){
		LOGV("Error : SsbSipMPEG4EncodeSetConfig(Framerate) \n");
		return OMX_FALSE;
	}

    return OMX_TRUE;
}
