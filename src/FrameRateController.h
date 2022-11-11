//-------------------------------------------------------------------------------------------------
// file:    FrameRateController.cpp
// author:  Andy Ellinger
// brief:   Functionality for frame rate control
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

namespace DGL
{

//----------------------------------------------------------------------------- FrameRateController

class FrameRateController
{
public:
    // Sets the max frame rate and resets the controller
    void Initialize(unsigned frameRateMax);

    // Resets the frame counter and the frame rate variables
    void Reset();

    // Increases the frame counter and delays to match the max frame rate
    void StartFrame();

    // Returns the time between the previous frame and this frame, in seconds
    double GetDeltaTime() const;

    // Returns the current frame count
    unsigned GetFrameCount() const;

private:
    // The maximum desired frame rate
    double mFrameRateMax{ 0.0 };
    // The time between the previous frame and this frame
    double mDeltaTime{ 0.0 };
    // The minimum time that should elapse between frames
    double mFrameTimeMin{ 0.0 };
    // The number of frames since the controller was reset
    unsigned mFrameCounter{ 0 };
    // The system time on the previous frame
    double mPrevFrameTime{ 0.0 };
    // Average sleep value
    double mAverageSleepCycles{ 0.002 };
    // Used when deciding whether to sleep
    double mSleepCyclesBuffer{ 0.001 };
};

} // namepspace DGL
