//-------------------------------------------------------------------------------------------------
// file:    Error.cpp
// author:  Andy Ellinger
// brief:   Handler for error messages
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "Error.h"

#include "DGL.h"
#include <iostream>
#include <sstream>
#include <cstdio>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__APPLE__)
#include <csignal>
#include <sys/sysctl.h>
#include <unistd.h>
#else
#include <csignal>
#include <fstream>
#endif

#include "WindowSystem.h"

namespace DGL
{
ErrorHandler* gError = nullptr;

#if !defined(_WIN32)
//*************************************************************************************************
// Returns true if a debugger is attached, so that we only stop the program when someone can see it
static bool IsDebuggerAttached()
{
#if defined(__APPLE__)
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
    struct kinfo_proc info {};
    size_t size = sizeof(info);
    if (sysctl(mib, 4, &info, &size, nullptr, 0) != 0)
        return false;
    return (info.kp_proc.p_flag & P_TRACED) != 0;
#else
    // On Linux, the TracerPid line is non-zero when a debugger is attached
    std::ifstream status("/proc/self/status");
    string line;
    while (std::getline(status, line))
    {
        if (line.rfind("TracerPid:", 0) == 0)
            return std::stoi(line.substr(10)) != 0;
    }
    return false;
#endif
}
#endif

//*************************************************************************************************
static string GetMessageFromHresult(long hr)
{
#if !defined(_WIN32)
    // There are no system error descriptions for HRESULT values outside of Windows
    char fullMessage[32];
    snprintf(fullMessage, sizeof(fullMessage), "Error 0x%08lx", (unsigned long)hr);
    return string(fullMessage);
#else
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
#endif
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

#if defined(_WIN32)
    MessageBox((HWND)gWinSys->GetNativeHandle(), msg.c_str(), "DGL Error", MB_ICONWARNING | MB_APPLMODAL);

    DebugBreak();
#else
    // There is no built-in message box, so the error is only printed to stderr.
    // Without a debugger, SIGTRAP would end the program, so only break when one is attached.
    if (IsDebuggerAttached())
        raise(SIGTRAP);
#endif
#endif
}

} // namespace DGL

using namespace DGL;

//*************************************************************************************************
const char* DGL_System_GetLastError(void)
{
    return gError->GetLastError();
}
