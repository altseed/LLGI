struct PS_INPUT {
  Position : vec4f,
  UV : vec2f,
  Color : vec4f,
}

struct PS_OUTPUT {
  Color0 : vec4f,
  Color1 : vec4f,
}

@internal(disable_validation__binding_point_collision) @group(4) @binding(0) var txt : texture_2d<f32>;

@internal(disable_validation__binding_point_collision) @group(4) @binding(0) var smp : sampler;

var<private> input_Position : vec4f;

var<private> input_UV : vec2f;

var<private> input_Color : vec4f;

var<private> x_entryPointOutput_Color0 : vec4f;

var<private> x_entryPointOutput_Color1 : vec4f;

fn x_main_struct_PS_INPUT_vf4_vf2_vf41_(input : ptr<function, PS_INPUT>) -> PS_OUTPUT {
  var c : vec4f;
  var output : PS_OUTPUT;
  let x_32 = (*(input)).UV;
  let x_33 = textureSample(txt, smp, x_32);
  c = x_33;
  c.w = 255.0f;
  let x_42 = c;
  output.Color0 = x_42;
  let x_47 = c.x;
  c.x = (1.0f - x_47);
  let x_52 = c.y;
  c.y = (1.0f - x_52);
  let x_57 = c.z;
  c.z = (1.0f - x_57);
  let x_60 = c;
  output.Color1 = x_60;
  let x_62 = output;
  return x_62;
}

fn main_1() {
  var input_1 : PS_INPUT;
  var flattenTemp : PS_OUTPUT;
  var param : PS_INPUT;
  let x_68 = input_Position;
  input_1.Position = x_68;
  let x_72 = input_UV;
  input_1.UV = x_72;
  let x_76 = input_Color;
  input_1.Color = x_76;
  let x_80 = input_1;
  param = x_80;
  let x_81 = x_main_struct_PS_INPUT_vf4_vf2_vf41_(&(param));
  flattenTemp = x_81;
  let x_85 = flattenTemp.Color0;
  x_entryPointOutput_Color0 = x_85;
  let x_88 = flattenTemp.Color1;
  x_entryPointOutput_Color1 = x_88;
  return;
}

struct main_out {
  @location(0)
  x_entryPointOutput_Color0_1 : vec4f,
  @location(1)
  x_entryPointOutput_Color1_1 : vec4f,
}

@fragment
fn main(@builtin(position) input_Position_param : vec4f, @location(0) input_UV_param : vec2f, @location(1) input_Color_param : vec4f) -> main_out {
  input_Position = input_Position_param;
  input_UV = input_UV_param;
  input_Color = input_Color_param;
  main_1();
  return main_out(x_entryPointOutput_Color0, x_entryPointOutput_Color1);
}
