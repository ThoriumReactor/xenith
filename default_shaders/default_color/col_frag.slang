struct FragmentInput 
{
  float4 color : COLOR0;
};

[shader("fragment")]
float4 main(FragmentInput fragment_input) : SV_Target
{
  return fragment_input.color;
}
