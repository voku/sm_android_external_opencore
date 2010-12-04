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
/**
	@file omx_Avc_component.h
	OpenMax decoder_component component.

*/

#ifndef OMX_AVC_COMPONENT_H_INCLUDED
#define OMX_AVC_COMPONENT_H_INCLUDED

#ifndef PV_OMXCOMPONENT_H_INCLUDED
#include "pv_omxcomponent.h"
#endif

#ifndef AVC_DEC_H_INCLUDED
#include "avc_dec.h"
#endif

// yj: multi-slice
#define NAL_START_CODE_SIZE		 4

#define INPUT_BUFFER_SIZE_AVC	 64000
#define OUTPUT_BUFFER_SIZE_AVC	 152064

#define NUMBER_INPUT_BUFFER_AVC  10
#define NUMBER_OUTPUT_BUFFER_AVC  3   //0    // 3


class OpenmaxAvcAO : public OmxComponentVideo
{
	public:
		OpenmaxAvcAO();
		~OpenmaxAvcAO();

		static OMX_ERRORTYPE BaseComponentAllocateBuffer(
									OMX_IN OMX_HANDLETYPE hComponent,
									OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
									OMX_IN OMX_U32 nPortIndex,
									OMX_IN OMX_PTR pAppPrivate,
									OMX_IN OMX_U32 nSizeBytes);
		OMX_ERRORTYPE		 AllocateBuffer(
									OMX_IN OMX_HANDLETYPE hComponent,
									OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
									OMX_IN OMX_U32 nPortIndex,
									OMX_IN OMX_PTR pAppPrivate,
									OMX_IN OMX_U32 nSizeBytes);
		static OMX_ERRORTYPE BaseComponentFreeBuffer(
									OMX_IN  OMX_HANDLETYPE hComponent,
									OMX_IN  OMX_U32 nPortIndex,
									OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);
		OMX_ERRORTYPE 		 FreeBuffer(
									OMX_IN  OMX_HANDLETYPE hComponent,
									OMX_IN  OMX_U32 nPortIndex,
									OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);
		OMX_ERRORTYPE		 GetConfig(
									OMX_IN  OMX_HANDLETYPE hComponent,
									OMX_IN  OMX_INDEXTYPE nIndex,
									OMX_INOUT OMX_PTR pComponentConfigStructure);
		
		OMX_ERRORTYPE ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy);
		OMX_ERRORTYPE DestroyComponent();
		OMX_ERRORTYPE ComponentInit();
		OMX_ERRORTYPE ComponentDeInit();

        static void	  ComponentGetRolesOfComponent(OMX_STRING* aRoleString);

		void		  ComponentBufferMgmtWithoutMarker();
		OMX_BOOL	  ParseFullAVCFramesIntoNALs(OMX_BUFFERHEADERTYPE* aInputBuffer);
		void		  ProcessData();
		void		  DecodeWithoutMarker();
		void		  DecodeWithMarker();

    private:
		// RainAde : removed for bug fix (thumbnail)
		// OMX_BOOL        iUseExtTimestamp;
        AvcDecoder_OMX* ipAvcDec;
        OMX_BOOL		iDecodeReturn;

        // yj: multi-slice
        // variables for "frame" mode i.e. iOMXComponentNeedsFullAVCFrames is turned on
        OMX_U32 iNALSizeArray[MAX_NAL_PER_FRAME]; // 100 should be more than enough NALs per frame
        OMX_U32 iCurrNAL;
        OMX_U32 iNumNALs;
        OMX_U32 iNALOffset;

};

#endif // OMX_AVC_COMPONENT_H_INCLUDED
