struct PS_INPUT {
  g_position : vec4f,
  g_color : vec4f,
}

var<private> input_g_position : vec4f;

var<private> input_g_color : vec4f;

var<private> x_entryPointOutput : vec4f;

fn x_main_struct_PS_INPUT_vf4_vf41_(input : ptr<function, PS_INPUT>) -> vec4f {
  let x_18 = (*(input)).g_color;
  return x_18;
}

fn main_1() {
  var input_1 : PS_INPUT;
  var param : PS_INPUT;
  input_1.g_position = input_g_position;
  input_1.g_color = input_g_color;
  param = input_1;
  let x_34 = x_main_struct_PS_INPUT_vf4_vf41_(&(param));
  x_entryPointOutput = x_34;
  return;
}

struct main_out {
  @location(0)
  x_entryPointOutput_1 : vec4f,
}

@fragment
fn main(@builtin(position) input_g_position_param : vec4f, @location(0) input_g_color_param : vec4f) -> main_out {
  input_g_position = input_g_position_param;
  input_g_color = input_g_color_param;
  main_1();
  return main_out(x_entryPointOutput);
}
