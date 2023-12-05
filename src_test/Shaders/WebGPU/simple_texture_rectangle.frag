struct PS_INPUT {
  Position : vec4f,
  UV : vec2f,
  Color : vec4f,
}

@internal(disable_validation__binding_point_collision) @group(4) @binding(0) var txt : texture_2d<f32>;

@internal(disable_validation__binding_point_collision) @group(4) @binding(0) var smp : sampler;

var<private> input_Position : vec4f;

var<private> input_UV : vec2f;

var<private> input_Color : vec4f;

var<private> x_entryPointOutput : vec4f;

fn x_main_struct_PS_INPUT_vf4_vf2_vf41_(input : ptr<function, PS_INPUT>) -> vec4f {
  var c : vec4f;
  let x_31 = (*(input)).UV;
  let x_32 = textureSample(txt, smp, x_31);
  c = x_32;
  c.w = 255.0f;
  let x_38 = c;
  return x_38;
}

fn main_1() {
  var input_1 : PS_INPUT;
  var param : PS_INPUT;
  let x_45 = input_Position;
  input_1.Position = x_45;
  let x_49 = input_UV;
  input_1.UV = x_49;
  let x_53 = input_Color;
  input_1.Color = x_53;
  let x_58 = input_1;
  param = x_58;
  let x_59 = x_main_struct_PS_INPUT_vf4_vf2_vf41_(&(param));
  x_entryPointOutput = x_59;
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
