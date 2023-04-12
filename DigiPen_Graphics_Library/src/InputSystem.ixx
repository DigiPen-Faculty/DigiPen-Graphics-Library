//-------------------------------------------------------------------------------------------------
// file:    InputSystem.h
// author:  Andy Ellinger
// brief:   Input system
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "DGL.h"
#include <array>

export module Input;

namespace DGL
{

//------------------------------------------------------------------------------------- InputSystem

export class InputSystem
{
public:
    // Sets the global pointer
    InputSystem();

    // Updates the input system, saving the previous key data and getting the mouse data
    void Update();
    
    // Returns the current scroll delta value
    int GetMouseScrollDelta() const;

    // Returns the current mouse position in screen coordinates
    // These coordinates will go from (0, 0) in the top left corner to 
    // (width, height) in the bottom right corner
    DGL_Vec2 GetMousePosition() const;

    // Returns the current change betweeen the mouse position last frame and this frame
    DGL_Vec2 GetMousePositionDelta() const;

    // Sets the state of the specified key
    void SetKeyState(unsigned char key, bool state);

    // Sets the mouse scroll delta value
    void SetMouseScrollDelta(int delta);

    // Returns true if the specified key is currently pressed
    bool KeyDown(unsigned char key) const;

    // Returns true if the specified key is down this frame and was up last frame
    bool KeyTriggered(unsigned char key) const;

    // Returns true if the specified key is up this frame and was down last frame
    bool KeyReleased(unsigned char key) const;

private:
    // Number of keys in the array. See the Windows virtual key list for all possible values.
    static constexpr int cInputKeyCount{ 256 };
    // Stores the current state of all keys. 
    std::array<bool, cInputKeyCount> mKeyState{ false };
    // Stores the state of all keys on the previous frame. 
    std::array<bool, cInputKeyCount> mPrevKeyState{ false };
    // Stores the position of the mouse pointer on this frame. 
    DGL_Vec2 mMousePos{ 0, 0 };
    // Stores the change in the mouse position from last frame to this frame. 
    DGL_Vec2 mMouseDelta{ 0, 0 };
    // Stores the change in the mouse wheel position. 
    int mMouseWheelDelta{ 0 };
};

// Global pointer for accessing the input system
export extern InputSystem* gInput;

}