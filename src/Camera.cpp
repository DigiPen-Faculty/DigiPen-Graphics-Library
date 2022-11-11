//-------------------------------------------------------------------------------------------------
// file:    Camera.cpp
// author:  Andy Ellinger
// brief:   Camera functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "Camera.h"
#include "GraphicsSystem.h"

namespace DGL
{

//*************************************************************************************************
DGL_Mat4 DxToMat4(const DirectX::XMMATRIX& dxMatrix)
{
    DGL_Mat4 newMatrix;
    DirectX::XMFLOAT4X4 temp;
    DirectX::XMStoreFloat4x4(&temp, dxMatrix);
    memcpy(newMatrix.m, temp.m, sizeof(float) * 16);
    return newMatrix;
}

//------------------------------------------------------------------------------------------ Camera

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
    gGraphics->D3D.SetWorldMatrix(GetWorldMatrix());
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
    // Set the new zoom factor
    mScale = zoom;
    // Update the world matrix on the constant buffer
    gGraphics->D3D.SetWorldMatrix(GetWorldMatrix());
}

//*************************************************************************************************
void CameraObject::SetWindowSize(const DGL_Vec2& size)
{
    // Save the window size
    mWindowSize = size;
    // Tell the graphics system to reset for the new window size
    gGraphics->D3D.ResetOnSizeChange();
}

//*************************************************************************************************
DGL_Mat4 CameraObject::GetWorldMatrix()
{
    // Create the view matrix 
    // The first parameter is the eye position (the current camera position)
    // The second parameter is the position the eye is looking at (directly in front of the camera)
    // The third parameter is the up direction
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(mCameraPosition.x, mCameraPosition.y, mCameraZ, 0.0f),
        DirectX::XMVectorSet(mCameraPosition.x, mCameraPosition.y, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    );

    // Create an orthographic projection matrix using the current window size and scale
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixOrthographicLH(
        mWindowSize.x * mScale,
        mWindowSize.y * mScale,
        1.0f,
        1000.0f
    );

    // Store the result of multiplying the matrices
    mProjViewMatrix = viewMatrix * projectionMatrix;

    // Return the world matrix, which is the transpose of view * projection
    return DxToMat4(DirectX::XMMatrixTranspose(mProjViewMatrix));
}

//*************************************************************************************************
DGL_Vec2 CameraObject::ScreenToWorld(const DGL_Vec2& screenPos) const
{
    // The XMMatrixInverse function needs a vector as the first parameter,
    // but it doesn't actually matter what that vector is
    DirectX::XMVECTOR temp;
    // Get the inverse of the view matrix * projection matrix
    DirectX::XMMATRIX invProjView = DirectX::XMMatrixInverse(&temp, mProjViewMatrix);

    // Adjust the screen position so it's in a -1 to 1 range
    DirectX::XMVECTOR adjPosVec{
        (((2.0f * screenPos.x) / mWindowSize.x) - 1),
        -(((2.0f * screenPos.y) / mWindowSize.y) - 1),
        1.0, 0.0f
    };

    // Multiply the adjusted screen position by the inverse projection view matrix
    DirectX::XMVECTOR transformedPos = DirectX::XMVector4Transform(adjPosVec, invProjView);

    // Return the X and Y values of the transformed vector
    return {
        DirectX::XMVectorGetX(transformedPos) + mCameraPosition.x,
        DirectX::XMVectorGetY(transformedPos) + mCameraPosition.y
    };
}

} // namespace DGL
