struct vs_in {
    float2 position_local : POSITION;
    float4 color : COLOR;
    float2 tex_coord : TEX;
};

struct vs_out {
    float4 position_clip : SV_POSITION; 
    float4 color : COLOR0;
    float2 tex_coord : TEXCOORD0;
    float alpha : COLOR1;
};

cbuffer cbPerObject
{
    float4x4 worldViewProjection;
    float4x4 transform;
    float4 tintColor;
    float2 texOffset;
    float alpha;
    float shaderData;
};

vs_out vs_main(vs_in input) {
    vs_out output = (vs_out)0; // zero the memory first

    float4 v = float4(input.position_local.x, input.position_local.y, 0.0, 1.0);
    v = mul(v, transform);
    output.position_clip = mul(v, worldViewProjection);

    output.color.x = (input.color.x * input.color.w) + (tintColor.x * tintColor.w);
    output.color.y = (input.color.y * input.color.w) + (tintColor.y * tintColor.w);
    output.color.z = (input.color.z * input.color.w) + (tintColor.z * tintColor.w);
    output.color.w = (input.color.w * input.color.w) + (tintColor.w * tintColor.w);

    output.tex_coord = input.tex_coord + texOffset;

    output.alpha = alpha;

    return output;
}

