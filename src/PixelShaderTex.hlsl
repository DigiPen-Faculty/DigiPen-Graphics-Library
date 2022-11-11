
struct vs_out {
    float4 position_clip : SV_POSITION; 
    float4 color : COLOR0;
    float2 tex_coord : TEXCOORD0;
    float alpha : COLOR1;
};

Texture2D ObjTexture;
SamplerState ObjSamplerState;

float4 ps_main(vs_out input) : SV_TARGET{
    float4 color = ObjTexture.Sample(ObjSamplerState, input.tex_coord);

    color.x = color.x + input.color.x;
    color.y = color.y + input.color.y;
    color.z = color.z + input.color.z;

    color.w *= input.alpha;

    return color;
}