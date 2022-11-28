//-------------------------------------------------------------------------------------------------
// file:    Error.cpp
// author:  Andy Ellinger
// brief:   Handler for error messages
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "DGL.h"
#include "Error.h"
#include <iostream>

namespace DGL
{
ErrorHandler* gError = nullptr;

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
    std::stringstream stream;
    stream << text << hr;
    SetError(stream.str());
}

//*************************************************************************************************
void ErrorHandler::SetError(string_view text)
{
    mLastError = text;
#ifdef _DEBUG
    std::cerr << "DGL Error: " << mLastError << std::endl;
#endif
}

} // namespace DGL

using namespace DGL;

//*************************************************************************************************
const char* DGL_System_GetLastError(void)
{
    return gError->GetLastError();
}