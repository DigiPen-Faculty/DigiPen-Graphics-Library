
struct vs_out {
    float4 position_clip : SV_POSITION; 
    float4 color : COLOR0;
    float2 tex_coord : TEXCOORD0;
    float alpha : COLOR1;
};

float4 ps_main(vs_out input) : SV_TARGET{
    float4 color = input.color;
    color.w *= input.alpha;
    return color;
}