#ifndef __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPH264ENCODE_H__
#define __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPH264ENCODE_H__


typedef unsigned int	H264_ENC_CONF;

#define H264_ENC_GETCONF_HEADER_SIZE		0x00001001
// added by RainAde for composer interface
#define H264_ENC_GETCONF_SPS_SIZE		0x00001002
#define H264_ENC_GETCONF_PPS_SIZE		0x00001003
// RainAde for Encoding pic type
#define H264_ENC_GETCONF_PIC_TYPE		0x00001004
// RainAde : for setting RC parameters 
#define H264_ENC_SETCONF_RC_PARA					0x00001005
// RainAde : for setting Picture Quantization Step on RC disable
#define H264_ENC_SETCONF_PIC_QS						0x00001006


#define H264_ENC_SETCONF_NUM_SLICES		0x00003001
#define H264_ENC_SETCONF_PARAM_CHANGE		0x00003010
#define H264_ENC_SETCONF_CUR_PIC_OPT		0x00003011

#define H264_ENC_PARAM_GOP_NUM			(0x7000A001)
#define H264_ENC_PARAM_INTRA_QP			(0x7000A002)
#define H264_ENC_PARAM_BITRATE			(0x7000A003)
#define H264_ENC_PARAM_F_RATE			(0x7000A004)
#define H264_ENC_PARAM_INTRA_REF		(0x7000A005)
#define H264_ENC_PARAM_SLICE_MODE		(0x7000A006)

#define H264_ENC_PIC_OPT_IDR			(0x7000B001)
#define H264_ENC_PIC_OPT_SKIP			(0x7000B002)
#define H264_ENC_PIC_OPT_RECOVERY		(0x7000B003)

// ENC_SEQ_RC_PARA (0x1AC)
#define RC_DISABLE						(0<<0)	// RC means rate control
#define RC_ENABLE						(1<<0)
#define SKIP_DISABLE					(1<<31)
#define SKIP_ENABLE						(0<<31)

#ifdef __cplusplus
extern "C" {
#endif


void *SsbSipH264EncodeInit(unsigned int uiWidth,     unsigned int uiHeight,
                           unsigned int uiFramerate, unsigned int uiBitrate_kbps,
                           unsigned int uiGOPNum);
int   SsbSipH264EncodeExe(void *openHandle);
int   SsbSipH264EncodeDeInit(void *openHandle);

int   SsbSipH264EncodeSetConfig(void *openHandle, H264_ENC_CONF conf_type, void *value);
int   SsbSipH264EncodeGetConfig(void *openHandle, H264_ENC_CONF conf_type, void *value);


void *SsbSipH264EncodeGetInBuf(void *openHandle, long size);
void *SsbSipH264EncodeGetOutBuf(void *openHandle, long *size);



#ifdef __cplusplus
}
#endif


// Error codes
#define SSBSIP_H264_ENC_RET_OK						(0)
#define SSBSIP_H264_ENC_RET_ERR_INVALID_HANDLE		(-1)
#define SSBSIP_H264_ENC_RET_ERR_INVALID_PARAM		(-2)

#define SSBSIP_H264_ENC_RET_ERR_CONFIG_FAIL			(-100)
#define SSBSIP_H264_ENC_RET_ERR_ENCODE_FAIL			(-101)
#define SSBSIP_H264_ENC_RET_ERR_GETCONF_FAIL		(-102)
#define SSBSIP_H264_ENC_RET_ERR_SETCONF_FAIL		(-103)


#endif /* __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPH264ENCODE_H__ */

