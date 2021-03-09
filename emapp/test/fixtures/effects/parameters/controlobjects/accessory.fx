bool accessory_visible : CONTROLOBJECT < string name = "test.x"; >;
float accessory_scaling : CONTROLOBJECT < string name = "test.x"; >;
float3 accessory_offset_3 : CONTROLOBJECT < string name = "test.x"; >;
float4 accessory_offset_4 : CONTROLOBJECT < string name = "test.x"; >;
float4 accessory_world_matrix : CONTROLOBJECT < string name = "test.x"; >;
float accessory_x : CONTROLOBJECT < string name = "test.x"; string item = "X"; >;
float accessory_y : CONTROLOBJECT < string name = "test.x"; string item = "Y"; >;
float accessory_z : CONTROLOBJECT < string name = "test.x"; string item = "Z"; >;
float3 accessory_xyz : CONTROLOBJECT < string name = "test.x"; string item = "XYZ"; >;
float accessory_rx : CONTROLOBJECT < string name = "test.x"; string item = "Rx"; >;
float accessory_ry : CONTROLOBJECT < string name = "test.x"; string item = "Ry"; >;
float accessory_rz : CONTROLOBJECT < string name = "test.x"; string item = "Rz"; >;
float3 accessory_rxyz : CONTROLOBJECT < string name = "test.x"; string item = "Rxyz"; >;
float accessory_si : CONTROLOBJECT < string name = "test.x"; string item = "Si"; >;
float accessory_tr : CONTROLOBJECT < string name = "test.x"; string item = "Tr"; >;
/* below variables cannot be available */
float3 model_bone_3 : CONTROLOBJECT < string name = "test.x"; string item = "センター"; >;
float4 model_bone_4 : CONTROLOBJECT < string name = "test.x"; string item = "センター"; >;
float4x4 model_bone_4x4 : CONTROLOBJECT < string name = "test.x"; string item = "センター"; >;
float model_morph : CONTROLOBJECT < string name = "test.x"; string item = "あ"; >;

#include "../shaders.fx"