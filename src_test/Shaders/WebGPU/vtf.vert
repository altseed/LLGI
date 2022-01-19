struct VS_INPUT {
  g_position : vec3f,
  g_uv : vec2f,
  g_color : vec4f,
}

struct VS_OUTPUT {
  g_position : vec4f,
  g_color : vec4f,
}

@internal(disable_validation__binding_point_collision) @group(1) @binding(0) var txt : texture_2d<f32>;

@internal(disable_validation__binding_point_collision) @group(4) @binding(0) var smp : sampler;

var<private> input_g_position : vec3f;

var<private> input_g_uv : vec2f;

var<private> input_g_color : vec4f;

var<private> x_entryPointOutput_g_position : vec4f;

var<private> x_entryPointOutput_g_color : vec4f;

fn x_main_struct_VS_INPUT_vf3_vf2_vf41_(input : ptr<function, VS_INPUT>) -> VS_OUTPUT {
  var c : vec4f;
  var output : VS_OUTPUT;
  c = textureSampleLevel(txt, smp, (*(input)).g_uv, 0.0f);
  let x_41 = (*(input)).g_position;
  output.g_position = vec4f(x_41.x, x_41.y, x_41.z, 1.0f);
  let x_53 = (output.g_position.xy + c.xy);
  output.g_position.x = x_53.x;
  output.g_position.y = x_53.y;
  output.g_color = (*(input)).g_color;
  let x_66 = output;
  return x_66;
}

fn main_1() {
  var input_1 : VS_INPUT;
  var flattenTemp : VS_OUTPUT;
  var param : VS_INPUT;
  input_1.g_position = input_g_position;
  input_1.g_uv = input_g_uv;
  input_1.g_color = input_g_color;
  param = input_1;
  let x_85 = x_main_struct_VS_INPUT_vf3_vf2_vf41_(&(param));
  flattenTemp = x_85;
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
fn main(@location(0) input_g_position_param : vec3f, @location(1) input_g_uv_param : vec2f, @location(2) input_g_color_param : vec4f) -> main_out {
  input_g_position = input_g_position_param;
  input_g_uv = input_g_uv_param;
  input_g_color = input_g_color_param;
  main_1();
  return main_out(x_entryPointOutput_g_position, x_entryPointOutput_g_color);
}
