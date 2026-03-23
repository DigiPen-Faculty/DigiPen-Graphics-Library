
// This is the data structure that will be passed from the application to the vertex shader
// It must match the input layout defined in the application code
struct vs_in {
    float2 position_local : POSITION;   // The local position of the vertex, which will be transformed to clip space in the vertex shader
    float4 color : COLOR;               // The color of the vertex, which will be used for tinting the texture in the pixel shader
    float2 tex_coord : TEX;             // The texture coordinates for sampling the texture in the pixel shader
};

// This is the data structure that will be passed from the vertex shader to the pixel shader
// It must match the input structure in the pixel shader, but does not need to be changed in the application code
struct vs_out {
    float4 position_clip : SV_POSITION; // The position in clip space, which is used for rasterization
    float4 color : COLOR0;              // The color that will be used to modify the texture color in the pixel shader
                                        // In this shader it's a combination of the vertex color and the tint color
    float2 tex_coord : TEXCOORD0;       // The texture coordinates for sampling the texture
    float alpha : COLOR1;               // The alpha value that will be used for alpha testing and blending
};

// The constant buffer that will be used to pass transformation matrices and other data from the application to the vertex shader
// It must match the constant buffer layout defined in the application code
cbuffer cbPerObject
{
    float4x4 worldViewProjection;   // This is calculated by DGL using the Camera properties
    float4x4 transform;             // The transformation matrix that will be applied to the vertex positions 
                                    // This is from either the TransformData property or the TransformMatrix
    float4 tintColor;               // The color set in the TintColor DGL property
    float2 texOffset;               // This will be added to the texture coordinates, from the TextureOffset DGL property
    float alpha;                    // The alpha value for fading and transparency effects, from the Alpha DGL property
    float shaderData;               // This is the ShaderData property
    float4 vectorData;              // This is the ShaderVector property
};

// The main function of the vertex shader
vs_out main(vs_in input) 
{
    // Initialize the output structure and zero the memory to ensure that all fields are set to default values
    vs_out output = (vs_out)0; 

    // Transform the local position of the vertex to clip space using the transformation matrix and the world-view-projection matrix
    float4 v = float4(input.position_local.x, input.position_local.y, 0.0, 1.0);
    v = mul(v, transform);
    output.position_clip = mul(v, worldViewProjection);

    // Combine the vertex color with the tint color to create the final color that will be passed to the pixel shader
    output.color.rgb = (input.color.rgb * input.color.w) + (tintColor.rgb * tintColor.w);

    // Add the texture offset to the input texture coordinates to create the final texture coordinates for the pixel shader
    output.tex_coord = input.tex_coord + texOffset;

    // Pass the alpha value from the constant buffer to the pixel shader
    output.alpha = alpha;

    // Return the output structure that will be passed to the pixel shader
    return output;
}

