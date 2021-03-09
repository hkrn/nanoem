bool model_visible : CONTROLOBJECT < string name = "test.pmx"; >;
float model_scaling : CONTROLOBJECT < string name = "test.pmx"; >;
float3 model_offset_3 : CONTROLOBJECT < string name = "test.pmx"; >;
float4 model_offset_4 : CONTROLOBJECT < string name = "test.pmx"; >;
float4x4 model_world_matrix : CONTROLOBJECT < string name = "test.pmx"; >;
float3 model_bone_3 : CONTROLOBJECT < string name = "test.pmx"; string item = "センター"; >;
float4 model_bone_4 : CONTROLOBJECT < string name = "test.pmx"; string item = "センター"; >;
float4x4 model_bone_4x4 : CONTROLOBJECT < string name = "test.pmx"; string item = "センター"; >;
float model_morph : CONTROLOBJECT < string name = "test.pmx"; string item = "あ"; >;
/* below variables cannot be available */
float model_x : CONTROLOBJECT < string name = "test.pmx"; string item = "X"; >;
float model_y : CONTROLOBJECT < string name = "test.pmx"; string item = "Y"; >;
float model_z : CONTROLOBJECT < string name = "test.pmx"; string item = "Z"; >;
float3 model_xyz : CONTROLOBJECT < string name = "test.pmx"; string item = "XYZ"; >;
float model_rx : CONTROLOBJECT < string name = "test.pmx"; string item = "Rx"; >;
float model_ry : CONTROLOBJECT < string name = "test.pmx"; string item = "Ry"; >;
float model_rz : CONTROLOBJECT < string name = "test.pmx"; string item = "Rz"; >;
float3 model_rxyz : CONTROLOBJECT < string name = "test.pmx"; string item = "Rxyz"; >;
float model_si : CONTROLOBJECT < string name = "test.pmx"; string item = "Si"; >;
float model_tr : CONTROLOBJECT < string name = "test.pmx"; string item = "Tr"; >;

#include "../shaders.fx"