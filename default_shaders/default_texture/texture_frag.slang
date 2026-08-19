[[vk::binding(0, 0)]]
Sampler2D bindless_textures_array[];

struct FragmentInput 
{
  float4 color : COLOR0;
  float2 uv : TEXCOORD0;
  nointerpolation uint32_t texture_id : TEXCOORD1;
};

[shader("fragment")]
float4 main(FragmentInput fragment_input) : SV_Target
{
  float4 sampled_texel = bindless_textures_array[fragment_input.texture_id].Sample(fragment_input.uv);

  return sampled_texel * fragment_input.color;
}
