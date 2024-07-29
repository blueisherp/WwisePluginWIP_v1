/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2024 Audiokinetic Inc.
*******************************************************************************/

#include "MyPluginFX.h"
#include "../MyPluginConfig.h"

#include <AK/AkWwiseSDKVersion.h>

AK::IAkPlugin* CreateMyPluginFX(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, MyPluginFX());
}

AK::IAkPluginParam* CreateMyPluginFXParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, MyPluginFXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(MyPluginFX, AkPluginTypeEffect, MyPluginConfig::CompanyID, MyPluginConfig::PluginID)

MyPluginFX::MyPluginFX()
    : m_pParams(nullptr)
    , m_pAllocator(nullptr)
    , m_pContext(nullptr)
{
}

MyPluginFX::~MyPluginFX()
{
}

AKRESULT MyPluginFX::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkEffectPluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
    m_pParams = (MyPluginFXParams*)in_pParams;
    m_pAllocator = in_pAllocator;
    m_pContext = in_pContext;
    SampleRate = in_rFormat.uSampleRate;

    return AK_Success;
}

AKRESULT MyPluginFX::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT MyPluginFX::Reset()
{
    return AK_Success;
}

AKRESULT MyPluginFX::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
    out_rPluginInfo.eType = AkPluginTypeEffect;
    out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bCanProcessObjects = false;
    out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}

#define AK_LINTODB( __lin__ ) (log10f(__lin__) * 20.f)

void MyPluginFX::Execute(AkAudioBuffer* io_pBuffer)
{
    const AkUInt32 uNumChannels = io_pBuffer->NumChannels();

    AkReal32 CompRatio = m_pParams->RTPC.fRatio;
    AkReal32 DBThreshold = m_pParams->RTPC.fThreshold;        // Threshold in dB
    AkReal32 rmsMonitorBefore = 0.f;                          // RMS for Monitor, pre-effect
    AkReal32 rmsMonitorAfter = 0.f;                           // RMS for Monitor, post-effect (needed only for Monitor Data)
    const AkUInt32 rmsFrames = (SampleRate / 100);            // Number of frames over 10ms
    AkUInt16 uFramesProcessed;
    // For each channel
    for (AkUInt32 i = 0; i < uNumChannels; ++i)
    {
        AkReal32* AK_RESTRICT pBuf = (AkReal32 * AK_RESTRICT)io_pBuffer->GetChannel(i);
        

        uFramesProcessed = 0;
        while (uFramesProcessed < io_pBuffer->uValidFrames)
        {
            AkReal32& AFSample = pBuf[uFramesProcessed];            // Amp Factor of the sample (LIN)

            #ifndef AK_OPTIMIZED // Monitor post-effect RMS
            if (m_pContext->CanPostMonitorData())
            {
                rmsMonitorBefore += powf(AFSample, 2);
            }
            #endif

            AkReal32 rmsMovingNew =                                 // Calculating new RMS based on previous instance
                sqrtf(
                    (
                        (powf(rmsMoving, 2) * (rmsFrames - 1)       // a fake sum of previous frames' squares
                        ) + powf(AFSample, 2)                       // add square of new sample
                    ) / rmsFrames                                   // divide by frames to get new average of squares
                );                                                  // square root everything

            rmsMoving = rmsMovingNew;

            // If Moving RMS is over Threshold 
            if (rmsMoving > AK_DBTOLIN(DBThreshold))
            {
                // db to be compressed, a negative float 
                AkReal32 DBCompressed =  -1 *                 
                        ((AK_LINTODB(rmsMoving) - DBThreshold)      // the difference between threshold and RMS
                                * (1 - (1 / CompRatio))             // apply compressor's ratio
                        );                                          // make negative

                // Compress the sample in linear
                AFSample *= AK_DBTOLIN(DBCompressed);
            }

            #ifndef AK_OPTIMIZED // Monitor post-effect RMS
            if (m_pContext->CanPostMonitorData())
            {
                rmsMonitorAfter += powf(AFSample, 2);
            }
            #endif
            ++uFramesProcessed;
        }

    }

    #ifndef AK_OPTIMIZED       // RMS calculation and serialization
        if (m_pContext->CanPostMonitorData())
        {
            rmsMonitorBefore = sqrt(rmsMonitorBefore/io_pBuffer->MaxFrames());
            rmsMonitorAfter = sqrt(rmsMonitorAfter / io_pBuffer->MaxFrames());
            AkReal32 rmsDiff = AK_LINTODB(rmsMonitorAfter) - AK_LINTODB(rmsMonitorBefore);              // Should be negative
            AkReal32 monitorData[1] = { rmsDiff};
            m_pContext->PostMonitorData((void*)monitorData, sizeof(monitorData));
        }
    #endif
}

AKRESULT MyPluginFX::TimeSkip(AkUInt32 in_uFrames)
{
    return AK_DataReady;
}
