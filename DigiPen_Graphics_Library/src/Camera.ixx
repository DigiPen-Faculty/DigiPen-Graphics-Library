//-------------------------------------------------------------------------------------------------
// file:    Camera.ixx
// author:  Andy Ellinger
// brief:   Header for camera functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "DGL.h"
#include <DirectXMath.h>

export module Camera;

namespace DGL
{

//------------------------------------------------------------------------------------------ Camera

export class CameraObject
{
public:

    void Initialize(HWND windowHandle);

    // Returns the current camera position
    DGL_Vec2 GetCameraPosition() const;

    // Sets the current camera position
    void SetCameraPosition(const DGL_Vec2& position);

    // Returns the current camera zoom factor
    float GetCameraZoom() const;

    // Sets the camera zoom factor
    void SetCameraZoom(float zoom);

    // Resets with the current window size
    void ResetWindowSize();

    // Returns the world matrix based on the current camera position and window size
    DGL_Mat4 GetWorldMatrix();

    // Translates a position in screen coordinates to the equivalent world coordinates
    // Screen coordinates go from (0, 0) in the top left corner to
    // (window width, window height) in the bottom right corner
    DGL_Vec2 ScreenToWorld(const DGL_Vec2& screenPos) const;

private:
    // The current camera position
    DGL_Vec2 mCameraPosition{ 0.0f, 0.0f };
    // The Z value used when creating the world matrix
    float mCameraZ{ -10.0f };
    // The scale used to modify the window size for the world matrix
    float mScale{ 1.0f };
    // The current size of the window
    DGL_Vec2 mWindowSize;
    // The current view matrix and projection matrix multiplied together
    DirectX::XMMATRIX mViewProjMatrix{ DirectX::XMMatrixIdentity() };

    HWND mWindowHandle{ nullptr };
};

} // namespace DGL
