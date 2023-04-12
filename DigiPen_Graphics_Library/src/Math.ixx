//-------------------------------------------------------------------------------------------------
// file:    Math.ixx
// author:  Andy Ellinger
// brief:   Math functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "DGL.h"

export module Math;

namespace DGL
{

// Creates a rotation matrix from the provided angle
// The angle should be in radians
export DGL_Mat4 Matrix_RotateZ(const float& angle);

// Returns the result of multiplying the two matrices together
export DGL_Mat4 Matrix_Multiply(const DGL_Mat4& matrix1, const DGL_Mat4 matrix2);

// Sets the provided matrix to the identity values
export void Matrix_SetToIdentity(DGL_Mat4& matrix);

} // namespace DGL