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
#define LOG_TAG "Mfc_Mpeg4_enc"
#include <utils/Log.h>


#include "mpeg4_enc.h"
#include "oscl_mem.h"

Mpeg4Encoder_OMX::Mpeg4Encoder_OMX()
{

};

Mpeg4Encoder_OMX::~Mpeg4Encoder_OMX()
{

};


/* Initialization routine */
OMX_ERRORTYPE Mpeg4Encoder_OMX::Mp4EncInit(OMX_S32 iEncMode,
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
        OMX_VIDEO_PARAM_PROFILELEVELTYPE* aProfileLevel)
{
	LOGV("MFC: Mpeg4Encoder_OMX::Mp4EncInit() \n");

	frame_cnt = 0;
	hdr_size = 0;

	int ret = 0;
	int slices[2];

	unsigned char *p_output_buffer = NULL;
	long encoded_size = 0;
	int gop_num = 0;
	
	gop_num = aEncodeParam.xFramerate >> 16;	// temporal value by RainAde : I frame every 1sec

	if(m_mpeg4enc_create_flag == 1)
	{
		LOGV("s3c6410_mpeg4enc_create == 1 \n");
		return OMX_ErrorUndefined;
	}

	if(iEncMode == 0)	// MODE_H263
	{
		LOGV("MODE_H263] nFrameWidth=%d, nFrameHeight=%d, Framerate=%d, nBitrate=%d, gop_num=%d", aEncodeParam.nFrameWidth, aEncodeParam.nFrameHeight, aEncodeParam.xFramerate >> 16, aEncodeParam.nBitrate/1000, gop_num);
		
		m_mpeg4enc_handle = SsbSipMPEG4EncodeInit(SSBSIPMFCENC_H263, aEncodeParam.nFrameWidth, aEncodeParam.nFrameHeight, aEncodeParam.xFramerate >> 16, aEncodeParam.nBitrate/1000, gop_num);		
		//slices[0] = ANNEX_K_OFF | ANNEX_T_OFF;
		slices[0] = ANNEX_K_OFF | ANNEX_T_OFF | ANNEX_J_ON;	// yj: enable deblocking-filter
		SsbSipMPEG4EncodeSetConfig(m_mpeg4enc_handle, MPEG4_ENC_SETCONF_H263_ANNEX, slices);
	}
	else
	{
		LOGV("MODE_MPEG4] nFrameWidth=%d, nFrameHeight=%d, Framerate=%d, nBitrate=%d, gop_num=%d", aEncodeParam.nFrameWidth, aEncodeParam.nFrameHeight, aEncodeParam.xFramerate >> 16, aEncodeParam.nBitrate/1000, gop_num);

		m_mpeg4enc_handle = SsbSipMPEG4EncodeInit(SSBSIPMFCENC_MPEG4, aEncodeParam.nFrameWidth, aEncodeParam.nFrameHeight, aEncodeParam.xFramerate >> 16, aEncodeParam.nBitrate/1000, gop_num);
	}		  
	if (m_mpeg4enc_handle == NULL) {
		LOGV("m_mpeg4enc_handle == NULL \n");
		return OMX_ErrorInsufficientResources;
	}

//	LOGV("aEncodeParam.nFrameWidth = %d", aEncodeParam.nFrameWidth);
//	LOGV("aEncodeParam.nFrameHeight = %d", aEncodeParam.nFrameHeight);

	ret = SsbSipMPEG4EncodeExe(m_mpeg4enc_handle);
	if(ret != SSBSIP_MPEG4_ENC_RET_OK){
		LOGV("Error : SsbSipMPEG4EncodeExe(before GetInBuf) \n");
		return OMX_ErrorUndefined;
	}
	
	m_mpeg4enc_buffer = (unsigned char *)SsbSipMPEG4EncodeGetInBuf(m_mpeg4enc_handle, 0);
	if (m_mpeg4enc_buffer == NULL) {
		LOGV("m_mpeg4enc_buffer == NULL \n");
		return OMX_ErrorInsufficientResources;
	}

	m_mpeg4enc_create_flag = 1;

	return OMX_ErrorNone;
	
}


/*Encode routine */
OMX_BOOL Mpeg4Encoder_OMX::Mp4EncodeVideo(OMX_U8*    aOutBuffer,
        OMX_U32*   aOutputLength,
        OMX_BOOL*  aBufferOverRun,
        OMX_U8**   aOverBufferPointer,
        OMX_U8*    aInBuffer,
        OMX_U32    aInBufSize,
        OMX_TICKS  aInTimeStamp,
        OMX_TICKS* aOutTimeStamp,
        OMX_BOOL*  aSyncFlag)
{
//	LOGV("MFC: Mpeg4Encoder_OMX::Mp4EncodeVideo() \n");

	unsigned char *p_output_buffer = NULL;
	long encoded_size = 0;
	int ret;
// RainAde for Encodig pic type
	int pic_type;

	// TimeStamp : bypass
	*aOutTimeStamp = aInTimeStamp;
	  
	// copy YUV data from OMX component src buffer to MFC src buffer
	// input data from Real camera
	if(frame_cnt != 1) // 1: 1st I frame
		memcpy(m_mpeg4enc_buffer, aInBuffer, aInBufSize);

	// *** Encoding
	if(frame_cnt != 1) // 1: 1st I frame
	{
		ret = SsbSipMPEG4EncodeExe(m_mpeg4enc_handle);
		if(ret != SSBSIP_MPEG4_ENC_RET_OK ) {
			LOGV("Error : SsbSipMPEG4EncodeExe \n");
			return OMX_FALSE;
		}
	}
	
	// get dst buffer address(stored Encoded data) and size(Encoded size)
	p_output_buffer = (unsigned char *)SsbSipMPEG4EncodeGetOutBuf(m_mpeg4enc_handle, &encoded_size);
	if (p_output_buffer == NULL) {
		LOGV("p_output_buffer == NULL \n");
		return OMX_FALSE;
	}

	if(frame_cnt == 0)	// VOL
		SsbSipMPEG4EncodeGetConfig(m_mpeg4enc_handle, MPEG4_ENC_GETCONF_HEADER_SIZE, &hdr_size);

	if(frame_cnt == 0) // 0: VOL
		encoded_size = hdr_size;
	else if(frame_cnt == 1) // 1: 1st I frame
		encoded_size -= hdr_size;
	
	if(*aOutputLength < encoded_size) {
		LOGV("output buffer is smaller than encoded data size \n");
		return OMX_FALSE;
	}

	// copy Encoded data from MFC dst buffer to OMX component dst buffer as much as the Encoded data size	
	*aOutputLength = (OMX_U32)encoded_size;

	if(frame_cnt != 1)
		memcpy(aOutBuffer, p_output_buffer, *aOutputLength);
	else
		memcpy(aOutBuffer, p_output_buffer+hdr_size, *aOutputLength);
	
// RainAde for Encoding pic type
	SsbSipMPEG4EncodeGetConfig(m_mpeg4enc_handle, MPEG4_ENC_GETCONF_PIC_TYPE, &pic_type);

// RainAde for thumbnail
	if(pic_type == PIC_TYPE_INTRA)
		*aSyncFlag = OMX_TRUE;
	else
		*aSyncFlag = OMX_FALSE;
	
	LOGV("pic_type = %d, *aSyncFlag = %d", pic_type, *aSyncFlag);

	frame_cnt++;
	
	return OMX_TRUE;

}


OMX_ERRORTYPE Mpeg4Encoder_OMX::Mp4EncDeinit()
{
	LOGV("MFC: Mpeg4Encoder_OMX::Mp4EncDeinit() \n");

	int ret;
	
	if(m_mpeg4enc_create_flag == 0)
	{
		LOGV("s3c6410_mpeg4enc_create == 0\n");
		return OMX_ErrorUndefined;
	}

	ret = SsbSipMPEG4EncodeDeInit(m_mpeg4enc_handle);
	if(ret != SSBSIP_MPEG4_ENC_RET_OK ) {
		LOGV("Error : SsbSipMPEG4EncodeDeInit\n");
		return OMX_ErrorUndefined;
	}
	
	m_mpeg4enc_buffer = NULL;
	m_mpeg4enc_create_flag = 0;

    return OMX_ErrorNone;
}


OMX_ERRORTYPE Mpeg4Encoder_OMX::Mp4RequestIFrame()
{
	int para_change[2];
	int ret;

	para_change[0] = MPEG4_ENC_PIC_OPT_IDR;
	para_change[1] = 1;

	ret = SsbSipMPEG4EncodeSetConfig(m_mpeg4enc_handle, MPEG4_ENC_SETCONF_CUR_PIC_OPT, para_change);
	
	if(ret != SSBSIP_MPEG4_ENC_RET_OK){
		LOGV("Error : SsbSipMPEG4EncodeSetConfig(Request I frame) \n");
		return OMX_ErrorUndefined;
	}

    return OMX_ErrorNone;
}


OMX_BOOL Mpeg4Encoder_OMX::Mp4UpdateBitRate(OMX_U32 aEncodedBitRate)
{
	int para_change[2];
	int ret;

	para_change[0] = MPEG4_ENC_PARAM_BITRATE;
	para_change[1] = aEncodedBitRate/1000;

	ret = SsbSipMPEG4EncodeSetConfig(m_mpeg4enc_handle, MPEG4_ENC_SETCONF_PARAM_CHANGE, para_change);

	if(ret != SSBSIP_MPEG4_ENC_RET_OK){
		LOGV("Error : SsbSipMPEG4EncodeSetConfig(Bitrate) \n");
		return OMX_FALSE;
	}

    return OMX_TRUE;
}


OMX_BOOL Mpeg4Encoder_OMX::Mp4UpdateFrameRate(OMX_U32 aEncodeFramerate)
{
	int para_change[2];
	int ret;

	para_change[0] = MPEG4_ENC_PARAM_F_RATE;
	para_change[1] = aEncodeFramerate >> 16;

	ret = SsbSipMPEG4EncodeSetConfig(m_mpeg4enc_handle, MPEG4_ENC_SETCONF_PARAM_CHANGE, para_change);

	if(ret != SSBSIP_MPEG4_ENC_RET_OK){
		LOGV("Error : SsbSipMPEG4EncodeSetConfig(Framerate) \n");
		return OMX_FALSE;
	}

    return OMX_TRUE;
}
