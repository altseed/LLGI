struct PS_INPUT {
  Position : vec4f,
  UV : vec2f,
  Color : vec4f,
}

alias RTArr = array<f32>;

struct compute_1 {
  /* @offset(0) */
  x_data : RTArr,
}

struct CB {
  /* @offset(0) */
  offset : vec4f,
}

@internal(disable_validation__binding_point_collision) @group(1) @binding(0) var<storage, read> compute_2 : compute_1;

var<private> input_Position : vec4f;

var<private> input_UV : vec2f;

var<private> input_Color : vec4f;

var<private> x_entryPointOutput : vec4f;

@internal(disable_validation__binding_point_collision) @group(0) @binding(301) var<uniform> x_60 : CB;

fn x_main_struct_PS_INPUT_vf4_vf2_vf41_(input : ptr<function, PS_INPUT>) -> vec4f {
  var c : vec4f;
  c = ((*(input)).Color + vec4f(compute_2.x_data[0i]));
  c.w = 1.0f;
  let x_36 = c;
  return x_36;
}

fn main_1() {
  var input_1 : PS_INPUT;
  var param : PS_INPUT;
  input_1.Position = input_Position;
  input_1.UV = input_UV;
  input_1.Color = input_Color;
  param = input_1;
  let x_57 = x_main_struct_PS_INPUT_vf4_vf2_vf41_(&(param));
  x_entryPointOutput = x_57;
  return;
}

struct main_out {
  @location(0)
  x_entryPointOutput_1 : vec4f,
}

@fragment
fn main(@builtin(position) input_Position_param : vec4f, @location(0) input_UV_param : vec2f, @location(1) input_Color_param : vec4f) -> main_out {
  input_Position = input_Position_param;
  input_UV = input_UV_param;
  input_Color = input_Color_param;
  main_1();
  return main_out(x_entryPointOutput);
}
