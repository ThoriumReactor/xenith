struct CameraPushConstantData
{
  float4x4 model;
  float4x4 view;
  float4x4 projection;
};

[[vk::push_constant]]
CameraPushConstantData camera_push_constant;


struct VertexInput
{
  [[vk::location(0)]] float3 position : POSITION;
  [[vk::location(1)]] float3 normals : NORMAL;
  [[vk::location(2)]] float4 color : COLOR;
  [[vk::location(3)]] float2 uv : TEXCOORD0;
  [[vk::location(4)]] uint32_t texture_id : TEXCOORD1;
};

struct VertexOutput
{
  float4 position : SV_Position;
  float4 color : COLOR0;
  float2 uv : TEXCOORD1;
  nointerpolation uint32_t texture_id : TEXCOORD2;
};


[shader("vertex")]
VertexOutput main(VertexInput vertex_input)
{
  float4 local_position = float4(vertex_input.position, 1.0f);
  float4 world_position = mul(camera_push_constant.model, local_position);

  float4 view_position = mul(camera_push_constant.view, world_position);
  float4 clip_position = mul(camera_push_constant.projection, view_position);

  float4 color = vertex_input.color;


  VertexOutput output;
  output.position = clip_position;
  output.color = color;
  output.uv = vertex_input.uv;
  output.texture_id = vertex_input.texture_id;

  return output;
}
