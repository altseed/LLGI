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

struct CS_INPUT {
  /* @offset(0) */
  value1 : f32,
  /* @offset(4) */
  value2 : f32,
}

alias RTArr = @stride(8) array<CS_INPUT>;

struct read {
  /* @offset(0) */
  x_data : RTArr,
}

@group(0) @binding(0) var<storage, read> read_1 : read;

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
  let x_40 = (*(input)).InstanceId;
  let x_43 = read_1.x_data[x_40].value1;
  let x_47 = output.g_position.x;
  output.g_position.x = (x_47 + x_43);
  let x_51 = (*(input)).InstanceId;
  let x_54 = read_1.x_data[x_51].value2;
  let x_57 = output.g_position.y;
  output.g_position.y = (x_57 + x_54);
  let x_62 = (*(input)).g_color;
  output.g_color = x_62;
  let x_64 = output;
  return x_64;
}

fn main_1() {
  var input_1 : VS_INPUT;
  var flattenTemp : VS_OUTPUT;
  var param : VS_INPUT;
  let x_70 = input_g_position;
  input_1.g_position = x_70;
  let x_74 = input_g_uv;
  input_1.g_uv = x_74;
  let x_79 = input_g_color;
  input_1.g_color = x_79;
  let x_83 = input_InstanceId;
  input_1.InstanceId = x_83;
  let x_87 = input_1;
  param = x_87;
  let x_88 = x_main_struct_VS_INPUT_vf3_vf2_vf4_u11_(&(param));
  flattenTemp = x_88;
  let x_92 = flattenTemp.g_position;
  x_entryPointOutput_g_position = x_92;
  let x_95 = flattenTemp.g_color;
  x_entryPointOutput_g_color = x_95;
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
