struct VS_INPUT {
  Position : vec3f,
  UV : vec2f,
  Color : vec4f,
}

struct VS_OUTPUT {
  Position : vec4f,
  UV : vec2f,
  Color : vec4f,
}

struct CB {
  /* @offset(0) */
  offset : vec4f,
}

@internal(disable_validation__binding_point_collision) @group(0) @binding(0) var<uniform> x_31 : CB;

var<private> input_Position : vec3f;

var<private> input_UV : vec2f;

var<private> input_Color : vec4f;

var<private> x_entryPointOutput_Position : vec4f;

var<private> x_entryPointOutput_UV : vec2f;

var<private> x_entryPointOutput_Color : vec4f;

fn x_main_struct_VS_INPUT_vf3_vf2_vf41_(input : ptr<function, VS_INPUT>) -> VS_OUTPUT {
  var output : VS_OUTPUT;
  let x_23 = (*(input)).Position;
  output.Position = (vec4f(x_23.x, x_23.y, x_23.z, 1.0f) + x_31.offset);
  output.UV = (*(input)).UV;
  output.Color = (*(input)).Color;
  let x_47 = output;
  return x_47;
}

fn main_1() {
  var input_1 : VS_INPUT;
  var flattenTemp : VS_OUTPUT;
  var param : VS_INPUT;
  input_1.Position = input_Position;
  input_1.UV = input_UV;
  input_1.Color = input_Color;
  param = input_1;
  let x_66 = x_main_struct_VS_INPUT_vf3_vf2_vf41_(&(param));
  flattenTemp = x_66;
  x_entryPointOutput_Position = flattenTemp.Position;
  x_entryPointOutput_UV = flattenTemp.UV;
  x_entryPointOutput_Color = flattenTemp.Color;
  return;
}

struct main_out {
  @builtin(position)
  x_entryPointOutput_Position_1 : vec4f,
  @location(0)
  x_entryPointOutput_UV_1 : vec2f,
  @location(1)
  x_entryPointOutput_Color_1 : vec4f,
}

@vertex
fn main(@location(0) input_Position_param : vec3f, @location(1) input_UV_param : vec2f, @location(2) input_Color_param : vec4f) -> main_out {
  input_Position = input_Position_param;
  input_UV = input_UV_param;
  input_Color = input_Color_param;
  main_1();
  return main_out(x_entryPointOutput_Position, x_entryPointOutput_UV, x_entryPointOutput_Color);
}
