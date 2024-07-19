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

#pragma once

#include "../MyPluginPlugin.h"
#include "../resource.h"
#include <string>

class MyPluginPluginGUI final
	: public AK::Wwise::Plugin::PluginMFCWindows<>
	, public AK::Wwise::Plugin::GUIWindows
    , public AK::Wwise::Plugin::Notifications::Monitor
{
public:
	MyPluginPluginGUI();

    HINSTANCE GetResourceHandle() const override;

    bool GetDialog(
        AK::Wwise::Plugin::eDialog in_eDialog,
        UINT& out_uiDialogID,
        AK::Wwise::Plugin::PopulateTableItem*& out_pTable
    ) const override;

    bool WindowProc(
        AK::Wwise::Plugin::eDialog in_eDialog,
        HWND in_hWnd,
        uint32_t in_message,
        WPARAM in_wParam,
        LPARAM in_lParam,
        LRESULT& out_lResult
    ) override;

    void NotifyMonitorData(
        AkTimeMs in_iTimeStamp,
        const AK::Wwise::Plugin::MonitorData* in_pMonitorDataArray,
        unsigned int in_uMonitorDataArraySize,
        bool in_bIsRealtime) override;


private:
    HWND m_hwndPropView = nullptr;
};
