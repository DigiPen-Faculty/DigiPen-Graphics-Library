//-------------------------------------------------------------------------------------------------
// file:    FrameRateController.cpp
// author:  Andy Ellinger, Justin Chambers
// brief:   Functionality for frame rate control
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "DGL.h"
#include "FrameRateController.h"
#include <thread>
#include <chrono>

namespace DGL
{

//----------------------------------------------------------------------------- FrameRateController

//*************************************************************************************************
void FrameRateController::Initialize(unsigned frameRateMax)
{
    // Save the max frame rate
    mFrameRateMax = frameRateMax;
    // Save the current time
    mPrevFrameTime = DGL_System_GetTime();

    Reset();
}

//*************************************************************************************************
void FrameRateController::Reset()
{
    // Reset the frame counter
    mFrameCounter = 0;

    // Check if the max frame rate is not zero
    if (mFrameRateMax)
    {
        // Calculate the delta time and minimum frame time
        mDeltaTime = 1.0 / mFrameRateMax;
        mFrameTimeMin = 1.0 / mFrameRateMax;
    }
    else
    {
        mDeltaTime = 0.0;
        mFrameTimeMin = 0.0;
    }
}

//*************************************************************************************************
void FrameRateController::StartFrame()
{
    // Increase the frame counter
    ++mFrameCounter;

    // Create the variables
    double timeNow = 0;
    double currTimeLeft = 0;
    double prevTimeLeft = 0;

    do
    {
        // Save the current time
        timeNow = DGL_System_GetTime();
        // Calculate the difference from the previous frame
        mDeltaTime = timeNow - mPrevFrameTime;

        // Compute remaining microseconds in the frame
        currTimeLeft = mFrameTimeMin - mDeltaTime;

        // If our remaining microseconds this frame are greater than the Sleep function's
        // average margin of error (plus an additional buffer value) then go ahead and Sleep
        if (currTimeLeft > mAverageSleepCycles + mSleepCyclesBuffer)
        {
            // Save the time left
            prevTimeLeft = currTimeLeft;

            // Give back cycles to other processes if we don't need them
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); 

            // Update the current time and delta time
            timeNow = DGL_System_GetTime();
            mDeltaTime = timeNow - mPrevFrameTime;

            // Recompute remaining micros
            currTimeLeft = mFrameTimeMin - mDeltaTime;

            // Update the average sleep over 16 samples so we maintain awareness of the system's margin of error.
            mAverageSleepCycles = ((mAverageSleepCycles * 15.0) + 
                (prevTimeLeft - currTimeLeft)) / 16.0;	// multiply by 15, add new sample, divide by 16
        }

    } while (mDeltaTime < mFrameTimeMin);

    mPrevFrameTime = timeNow;
}

//*************************************************************************************************
double FrameRateController::GetDeltaTime() const
{
    return mDeltaTime;
}

//*************************************************************************************************
unsigned FrameRateController::GetFrameCount() const
{
    return mFrameCounter;
}

} // namepspace DGL
