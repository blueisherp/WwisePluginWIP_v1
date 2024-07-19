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

#include "MyPluginPluginGUI.h"
#include <sstream>
#include <iomanip>

MyPluginPluginGUI::MyPluginPluginGUI()
{
}

HINSTANCE MyPluginPluginGUI::GetResourceHandle() const
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return AfxGetStaticModuleState()->m_hCurrentResourceHandle;
}

AK_WWISE_PLUGIN_GUI_WINDOWS_BEGIN_POPULATE_TABLE(MyPluginProperties)
AK_WWISE_PLUGIN_GUI_WINDOWS_POP_ITEM(
    IDC_THRESHOLD,
    "Threshold"
)
AK_WWISE_PLUGIN_GUI_WINDOWS_POP_ITEM(
    IDC_RATIO,
    "Ratio"
)
AK_WWISE_PLUGIN_GUI_WINDOWS_END_POPULATE_TABLE();

bool MyPluginPluginGUI::GetDialog(AK::Wwise::Plugin::eDialog in_eDialog, UINT& out_uiDialogID, AK::Wwise::Plugin::PopulateTableItem*& out_pTable) const
{
    AKASSERT(in_eDialog == AK::Wwise::Plugin::SettingsDialog);

    out_uiDialogID = IDD_DIALOG1;
    out_pTable = MyPluginProperties;

    return true;
}


bool MyPluginPluginGUI::WindowProc(AK::Wwise::Plugin::eDialog in_eDialog, HWND in_hWnd, uint32_t in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT& out_lResult)
{
    switch (in_message)
    {
    case WM_INITDIALOG:
        m_hwndPropView = in_hWnd;
        break;
    case WM_DESTROY:
        m_hwndPropView = NULL;
        break;
    }
    out_lResult = 0;
    return false;
}

#define round2(__FLOAT__) (roundf((__FLOAT__) * 100) / 100)

// Serializing Monitor Data
void MyPluginPluginGUI::NotifyMonitorData( AkTimeMs in_iTimeStamp, const AK::Wwise::Plugin::MonitorData* in_pMonitorDataArray, unsigned int in_uMonitorDataArraySize, bool in_bIsRealtime)
{
    if (m_hwndPropView != NULL &&
        in_pMonitorDataArray != nullptr)
    {
        if (in_pMonitorDataArray->pData != nullptr &&
            in_pMonitorDataArray->uDataSize == (sizeof(AkReal32) * 2)) // expects 2 elements of AKReal32 (rmsBefore, rmsAfter)
        {
            AkReal32* serializedData = (AkReal32*)in_pMonitorDataArray->pData;

            // Convert floats into strings with fixed precision (2 decimal places)
            std::ostringstream ossInput, ossOutput, ossDiff;
            ossInput << std::fixed << std::setprecision(2) << serializedData[0];
            std::string inputSTR = ossInput.str();
            ossOutput << std::fixed << std::setprecision(2) << serializedData[1];
            std::string outputSTR = ossOutput.str();

            // Difference = AfterRMS - BeforeRMS, AKA the dB compressed
            ossDiff << std::fixed << std::setprecision(2) << serializedData[1] - serializedData[0];
            std::string diffSTR = ossDiff.str();

            HWND inputRMSLabel = ::GetDlgItem(m_hwndPropView, IDC_INPUT_RMS);
            ::SetWindowTextA(inputRMSLabel, inputSTR.c_str());

            HWND outputRMSLabel = ::GetDlgItem(m_hwndPropView, IDC_OUTPUT_RMS);
            ::SetWindowTextA(outputRMSLabel, outputSTR.c_str());

            HWND diffRMSLabel = ::GetDlgItem(m_hwndPropView, IDC_DIFF_RMS);
            ::SetWindowTextA(diffRMSLabel, diffSTR.c_str());

        }
    }
}


ADD_AUDIOPLUGIN_CLASS_TO_CONTAINER(
    MyPlugin,            // Name of the plug-in container for this shared library
    MyPluginPluginGUI,   // Authoring plug-in class to add to the plug-in container
    MyPluginFX           // Corresponding Sound Engine plug-in class
);
