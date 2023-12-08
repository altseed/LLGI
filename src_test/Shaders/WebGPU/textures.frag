struct PS_Input {
  Pos : vec4f,
  UV : vec2f,
  Color : vec4f,
}

@internal(disable_validation__binding_point_collision) @group(1) @binding(0) var g_texture1 : texture_2d<f32>;

@internal(disable_validation__binding_point_collision) @group(4) @binding(0) var g_sampler1 : sampler;

@internal(disable_validation__binding_point_collision) @group(1) @binding(1) var g_texture2 : texture_2d_array<f32>;

@internal(disable_validation__binding_point_collision) @group(4) @binding(1) var g_sampler2 : sampler;

@internal(disable_validation__binding_point_collision) @group(1) @binding(2) var g_texture3 : texture_3d<f32>;

@internal(disable_validation__binding_point_collision) @group(4) @binding(2) var g_sampler3 : sampler;

var<private> Input_Pos : vec4f;

var<private> Input_UV : vec2f;

var<private> Input_Color : vec4f;

var<private> x_entryPointOutput : vec4f;

fn x_main_struct_PS_Input_vf4_vf2_vf41_(Input : PS_Input) -> vec4f {
  if ((Input.UV.x < 0.30000001192092895508f)) {
    let x_35 = textureSample(g_texture1, g_sampler1, Input.UV);
    return x_35;
  } else {
    if ((Input.UV.x < 0.60000002384185791016f)) {
      let x_51 = Input.UV;
      let x_57 = textureSample(g_texture2, g_sampler2, vec3f(x_51.x, x_51.y, 1.0f).xy, i32(round(vec3f(x_51.x, x_51.y, 1.0f).z)));
      return x_57;
    }
  }
  let x_67 = Input.UV;
  let x_72 = textureSample(g_texture3, g_sampler3, vec3f(x_67.x, x_67.y, 0.5f));
  return x_72;
}

fn main_1() {
  var Input_1 : PS_Input;
  let x_80 = Input_Pos;
  Input_1.Pos = x_80;
  let x_85 = Input_UV;
  Input_1.UV = x_85;
  let x_90 = Input_Color;
  Input_1.Color = x_90;
  let x_94 = Input_1;
  let x_95 = x_main_struct_PS_Input_vf4_vf2_vf41_(x_94);
  x_entryPointOutput = x_95;
  return;
}

struct main_out {
  @location(0)
  x_entryPointOutput_1 : vec4f,
}

@fragment
fn main(@builtin(position) Input_Pos_param : vec4f, @location(0) Input_UV_param : vec2f, @location(1) Input_Color_param : vec4f) -> main_out {
  Input_Pos = Input_Pos_param;
  Input_UV = Input_UV_param;
  Input_Color = Input_Color_param;
  main_1();
  return main_out(x_entryPointOutput);
}
