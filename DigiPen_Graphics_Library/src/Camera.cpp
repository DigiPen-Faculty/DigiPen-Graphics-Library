//-------------------------------------------------------------------------------------------------
// file:    Camera.cpp
// author:  Andy Ellinger
// brief:   Camera functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "Camera.h"

#include "DGL.h"
#include <math.h>

#include "GraphicsSystem.h"
#include "DGLMath.h"
#include "WindowSystem.h"

namespace DGL
{

//------------------------------------------------------------------------------------------ Camera

//*************************************************************************************************
void CameraObject::Initialize()
{
    // Save the window size
    mWindowSize = gWinSys->GetWindowSize();

    mInitialized = true;
}

//*************************************************************************************************
DGL_Vec2 CameraObject::GetCameraPosition() const
{
    // Return the current camera position
    return mCameraPosition;
}

//*************************************************************************************************
void CameraObject::SetCameraPosition(const DGL_Vec2& position)
{
    // Set the new camera position
    mCameraPosition = position;
    // Update the world matrix on the constant buffer
    gGraphics->Renderer.mConstantBuffer.worldMatrix = GetWorldMatrix();
}

//*************************************************************************************************
float CameraObject::GetCameraZoom() const
{
    // Return the current camera zoom factor
    return mScale;
}

//*************************************************************************************************
void CameraObject::SetCameraZoom(float zoom)
{
    // Keep the scale above a minimum value
    if (zoom <= 0.01f)
        zoom = 0.01f;

    // Set the new zoom factor
    mScale = zoom;
    // Update the world matrix on the constant buffer
    gGraphics->Renderer.mConstantBuffer.worldMatrix = GetWorldMatrix();
}

//*************************************************************************************************
float CameraObject::GetCameraRotation() const
{
    return mRotation;
}

//*************************************************************************************************
void CameraObject::SetCameraRotation(float radians)
{
    mRotation = radians;

    // Update the world matrix on the constant buffer
    gGraphics->Renderer.mConstantBuffer.worldMatrix = GetWorldMatrix();
}

//*************************************************************************************************
void CameraObject::ResetWindowSize()
{
    if (!mInitialized)
        return;

    // Save the window size
    mWindowSize = gWinSys->GetWindowSize();
    if (mWindowSize.x <= 0.0f || mWindowSize.y <= 0.0f)
        return;

    // Tell the graphics system to reset for the new window size
    gGraphics->Renderer.ResetOnSizeChange();
}

//*************************************************************************************************
DGL_Mat4 CameraObject::GetWorldMatrix() const
{
    // The visible area is the window size, scaled by the zoom factor
    return CreateWorldMatrix(mCameraPosition, mRotation, mWindowSize.x * mScale, mWindowSize.y * mScale);
}

//*************************************************************************************************
DGL_Mat4 CameraObject::CreateWorldMatrix(const DGL_Vec2& position, float rotation, float width, float height)
{
    // These matrices are built for row vectors (position * matrix), then the result is transposed
// for the shaders. This gives the same values as the left-handed DirectXMath functions DGL used
// before (XMMatrixLookAtLH, XMMatrixRotationAxis, and XMMatrixOrthographicLH).

// The view matrix moves the camera position to the origin
    DGL_Mat4 viewMatrix;
    Matrix_SetToIdentity(viewMatrix);
    viewMatrix.m[3][0] = -position.x;
    viewMatrix.m[3][1] = -position.y;
    viewMatrix.m[3][2] = cCameraDistance;

    // The rotation matrix rotates around the Z axis (transposed, since Matrix_RotateZ is for column vectors)
    DGL_Mat4 rotationMatrix = Matrix_Transpose(Matrix_RotateZ(rotation));

    // The orthographic projection matrix scales the visible area to the -1 to 1 range,
    // and the visible depth to the 0 to 1 range
    float depthRange = 1.0f / (cFarPlane - cNearPlane);
    DGL_Mat4 projectionMatrix{ 0 };
    projectionMatrix.m[0][0] = 2.0f / width;
    projectionMatrix.m[1][1] = 2.0f / height;
    projectionMatrix.m[2][2] = depthRange;
    projectionMatrix.m[3][2] = -depthRange * cNearPlane;
    projectionMatrix.m[3][3] = 1.0f;

    return Matrix_Transpose(Matrix_Multiply(Matrix_Multiply(viewMatrix, rotationMatrix), projectionMatrix));
}

//*************************************************************************************************
DGL_Vec2 CameraObject::ScreenToWorld(const DGL_Vec2& screenPos) const
{
    return ScreenToWorld(screenPos, mWindowSize, mCameraPosition, mRotation,
        mWindowSize.x * mScale, mWindowSize.y * mScale);
}

//*************************************************************************************************
DGL_Vec2 CameraObject::ScreenToWorld(const DGL_Vec2& screenPos, const DGL_Vec2& screenSize, 
    const DGL_Vec2& position, float rotation, float width, float height)
{
    // Convert the screen position to the -1 to 1 range (with +Y up)
    float x = ((2.0f * screenPos.x) / screenSize.x) - 1.0f;
    float y = -(((2.0f * screenPos.y) / screenSize.y) - 1.0f);

    // Undo the projection scale
    x *= width / 2.0f;
    y *= height / 2.0f;

    // Undo the rotation, then the camera position
    float cosine = cosf(rotation);
    float sine = sinf(rotation);
    return {
        x * cosine + y * sine + position.x,
        -x * sine + y * cosine + position.y
    };
}

} // namespace DGL
