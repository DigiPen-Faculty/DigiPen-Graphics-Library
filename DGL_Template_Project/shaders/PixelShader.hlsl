
// This is the data structure that will be passed from the vertex shader to the pixel shader
// It must match the output structure in the vertex shader
struct vs_out {
    float4 position_clip : SV_POSITION;     // The position in clip space, which is used for rasterization
    float4 color : COLOR0;                  // The tint color that will be applied to the texture color
    float2 tex_coord : TEXCOORD0;           // The texture coordinates for sampling the texture
    float alpha : COLOR1;                   // The alpha value that will be used for alpha testing and blending
};

// The texture and sampler state that will be used for sampling the texture
Texture2D ObjTexture;
SamplerState ObjSamplerState;

// The main function of the pixel shader
float4 main(vs_out input) : SV_TARGET
{
    // Sample the texture using the texture coordinates from the vertex shader
    float4 color = ObjTexture.Sample(ObjSamplerState, input.tex_coord);
    
    // Perform alpha testing by discarding pixels that are below a certain alpha threshold
    clip(color.w * input.alpha - 0.01f);
    
    // Multiply the sampled color with the tint color from the vertex shader
    color.rgb *= input.color.rgb;

    // Multiply the alpha of the sampled color with the alpha from the vertex shader
    // This applies the alpha setting for fading and transparency effects
    color.a *= input.alpha;

    // Return the final color that will be output to the render target
    return color;
}