struct VS_INPUT {
  g_position : vec3f,
  g_uv : vec2f,
  g_color : vec4f,
  InstanceId : u32,
}

struct VS_OUTPUT {
  g_position : vec4f,
  g_color : vec4f,
}

alias Arr = array<vec4f, 10u>;

struct CB {
  /* @offset(0) */
  offsets : Arr,
}

@internal(disable_validation__binding_point_collision) @group(0) @binding(0) var<uniform> x_36 : CB;

var<private> input_g_position : vec3f;

var<private> input_g_uv : vec2f;

var<private> input_g_color : vec4f;

var<private> input_InstanceId : u32;

var<private> x_entryPointOutput_g_position : vec4f;

var<private> x_entryPointOutput_g_color : vec4f;

fn x_main_struct_VS_INPUT_vf3_vf2_vf4_u11_(input : ptr<function, VS_INPUT>) -> VS_OUTPUT {
  var output : VS_OUTPUT;
  let x_24 = (*(input)).g_position;
  output.g_position = vec4f(x_24.x, x_24.y, x_24.z, 1.0f);
  output.g_position.x = (output.g_position.x + x_36.offsets[(*(input)).InstanceId].x);
  output.g_position.y = (output.g_position.y + x_36.offsets[(*(input)).InstanceId].y);
  output.g_color = (*(input)).g_color;
  let x_64 = output;
  return x_64;
}

fn main_1() {
  var input_1 : VS_INPUT;
  var flattenTemp : VS_OUTPUT;
  var param : VS_INPUT;
  input_1.g_position = input_g_position;
  input_1.g_uv = input_g_uv;
  input_1.g_color = input_g_color;
  input_1.InstanceId = input_InstanceId;
  param = input_1;
  let x_88 = x_main_struct_VS_INPUT_vf3_vf2_vf4_u11_(&(param));
  flattenTemp = x_88;
  x_entryPointOutput_g_position = flattenTemp.g_position;
  x_entryPointOutput_g_color = flattenTemp.g_color;
  return;
}

struct main_out {
  @builtin(position)
  x_entryPointOutput_g_position_1 : vec4f,
  @location(0)
  x_entryPointOutput_g_color_1 : vec4f,
}

@vertex
fn main(@location(0) input_g_position_param : vec3f, @location(1) input_g_uv_param : vec2f, @location(2) input_g_color_param : vec4f, @builtin(instance_index) input_InstanceId_param : u32) -> main_out {
  input_g_position = input_g_position_param;
  input_g_uv = input_g_uv_param;
  input_g_color = input_g_color_param;
  input_InstanceId = input_InstanceId_param;
  main_1();
  return main_out(x_entryPointOutput_g_position, x_entryPointOutput_g_color);
}
