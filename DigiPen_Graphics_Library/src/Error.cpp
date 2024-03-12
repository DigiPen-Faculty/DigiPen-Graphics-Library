//-------------------------------------------------------------------------------------------------
// file:    Error.cpp
// author:  Andy Ellinger
// brief:   Handler for error messages
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "DGL.h"
#include <iostream>
#include <sstream>

module Errors;

import WindowsSystem;

namespace DGL
{
ErrorHandler* gError = nullptr;

//*************************************************************************************************
string GetMessageFromHresult(long hr)
{
    if (HRESULT_FACILITY(hr) == FACILITY_WINDOWS)
        hr = HRESULT_CODE(hr);
    TCHAR* errorMessage;
    char fullMessage[100];

    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&errorMessage,
        0,
        nullptr) != 0)
    {
        snprintf(fullMessage, 100, "Error 0x%08x: %s", hr, errorMessage);

        LocalFree(errorMessage);
    }
    else
        snprintf(fullMessage, 100, "Error 0x%08x", hr);

    return string(fullMessage);
}

//------------------------------------------------------------------------------------ ErrorHandler

//*************************************************************************************************
ErrorHandler::ErrorHandler()
{
    gError = this;
}

//*************************************************************************************************
const char* ErrorHandler::GetLastError() const
{
    return mLastError.c_str();
}

//*************************************************************************************************
void ErrorHandler::SetError(string_view text, long hr)
{
    string newText(text);
    newText += GetMessageFromHresult(hr);
    SetError(newText);
}

//*************************************************************************************************
void ErrorHandler::SetError(string_view text)
{
    mLastError = text;
#ifdef _DEBUG
    string msg = "DGL Error: ";
    msg += text;

    std::cerr << msg << std::endl;

    MessageBox(gWinSys->GetWindowHandle(), msg.c_str(), "DGL Error", MB_ICONWARNING | MB_APPLMODAL);

    DebugBreak();
#endif
}

} // namespace DGL

using namespace DGL;

//*************************************************************************************************
const char* DGL_System_GetLastError(void)
{
    return gError->GetLastError();
}