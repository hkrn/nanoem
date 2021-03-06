/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: effect_dx11.proto */

#ifndef PROTOBUF_C_effect_5fdx11_2eproto__INCLUDED
#define PROTOBUF_C_effect_5fdx11_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _Fx9__Effect__Hlsl__Uniform Fx9__Effect__Hlsl__Uniform;
typedef struct _Fx9__Effect__Hlsl__Constant Fx9__Effect__Hlsl__Constant;
typedef struct _Fx9__Effect__Hlsl__Sampler Fx9__Effect__Hlsl__Sampler;
typedef struct _Fx9__Effect__Hlsl__Attribute Fx9__Effect__Hlsl__Attribute;
typedef struct _Fx9__Effect__Hlsl__Symbol Fx9__Effect__Hlsl__Symbol;
typedef struct _Fx9__Effect__Hlsl__SamplerState Fx9__Effect__Hlsl__SamplerState;
typedef struct _Fx9__Effect__Hlsl__RenderState Fx9__Effect__Hlsl__RenderState;
typedef struct _Fx9__Effect__Hlsl__Texture Fx9__Effect__Hlsl__Texture;
typedef struct _Fx9__Effect__Hlsl__Shader Fx9__Effect__Hlsl__Shader;
typedef struct _Fx9__Effect__Hlsl__Pass Fx9__Effect__Hlsl__Pass;


/* --- enums --- */

typedef enum _Fx9__Effect__Hlsl__ParameterClass {
  FX9__EFFECT__HLSL__PARAMETER_CLASS__PC_SCALAR = 0,
  FX9__EFFECT__HLSL__PARAMETER_CLASS__PC_VECTOR = 1,
  FX9__EFFECT__HLSL__PARAMETER_CLASS__PC_MATRIX_ROWS = 2,
  FX9__EFFECT__HLSL__PARAMETER_CLASS__PC_MATRIX_COLUMNS = 3,
  FX9__EFFECT__HLSL__PARAMETER_CLASS__PC_OBJECT = 4,
  FX9__EFFECT__HLSL__PARAMETER_CLASS__PC_STRUCT = 5
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__HLSL__PARAMETER_CLASS)
} Fx9__Effect__Hlsl__ParameterClass;
typedef enum _Fx9__Effect__Hlsl__ParameterType {
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_VOID = 0,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_BOOL = 1,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_INT = 2,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_FLOAT = 3,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_STRING = 4,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_TEXTURE = 5,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_TEXTURE1D = 6,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_TEXTURE2D = 7,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_TEXTURE3D = 8,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_TEXTURECUBE = 9,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_SAMPLER = 10,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_SAMPLER1D = 11,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_SAMPLER2D = 12,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_SAMPLER3D = 13,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_SAMPLERCUBE = 14,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_PIXELSHADER = 15,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_VERTEXSHADER = 16,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_PIXELFRAGMENT = 17,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_VERTEXFRAGMENT = 18,
  FX9__EFFECT__HLSL__PARAMETER_TYPE__PT_UNSUPPORTED = 19
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__HLSL__PARAMETER_TYPE)
} Fx9__Effect__Hlsl__ParameterType;
typedef enum _Fx9__Effect__Hlsl__ShaderType {
  FX9__EFFECT__HLSL__SHADER_TYPE__ST_PIXEL = 1,
  FX9__EFFECT__HLSL__SHADER_TYPE__ST_VERTEX = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__HLSL__SHADER_TYPE)
} Fx9__Effect__Hlsl__ShaderType;
typedef enum _Fx9__Effect__Hlsl__UniformType {
  FX9__EFFECT__HLSL__UNIFORM_TYPE__UT_FLOAT = 0,
  FX9__EFFECT__HLSL__UNIFORM_TYPE__UT_INT = 1,
  FX9__EFFECT__HLSL__UNIFORM_TYPE__UT_BOOL = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__HLSL__UNIFORM_TYPE)
} Fx9__Effect__Hlsl__UniformType;
typedef enum _Fx9__Effect__Hlsl__SamplerType {
  FX9__EFFECT__HLSL__SAMPLER_TYPE__SAMPLER_2D = 0,
  FX9__EFFECT__HLSL__SAMPLER_TYPE__SAMPLER_CUBE = 1,
  FX9__EFFECT__HLSL__SAMPLER_TYPE__SAMPLER_VOLUME = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__HLSL__SAMPLER_TYPE)
} Fx9__Effect__Hlsl__SamplerType;
typedef enum _Fx9__Effect__Hlsl__AttributeUsage {
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_POSITION = 0,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_BLENDWEIGHT = 1,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_BLENDINDICES = 2,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_NORMAL = 3,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_POINTSIZE = 4,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_TEXCOORD = 5,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_TANGENT = 6,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_BINORMAL = 7,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_TESSFACTOR = 8,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_POSITIONT = 9,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_COLOR = 10,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_FOG = 11,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_DEPTH = 12,
  FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_SAMPLE = 13
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__HLSL__ATTRIBUTE_USAGE)
} Fx9__Effect__Hlsl__AttributeUsage;
typedef enum _Fx9__Effect__Hlsl__RegisterSet {
  FX9__EFFECT__HLSL__REGISTER_SET__RS_BOOL = 0,
  FX9__EFFECT__HLSL__REGISTER_SET__RS_INT4 = 1,
  FX9__EFFECT__HLSL__REGISTER_SET__RS_FLOAT4 = 2,
  FX9__EFFECT__HLSL__REGISTER_SET__RS_SAMPLER = 3
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__HLSL__REGISTER_SET)
} Fx9__Effect__Hlsl__RegisterSet;

/* --- messages --- */

struct  _Fx9__Effect__Hlsl__Uniform
{
  ProtobufCMessage base;
  Fx9__Effect__Hlsl__UniformType type;
  uint32_t index;
  uint32_t num_elements;
  uint32_t constant_index;
  char *name;
};
#define FX9__EFFECT__HLSL__UNIFORM__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__hlsl__uniform__descriptor) \
    , FX9__EFFECT__HLSL__UNIFORM_TYPE__UT_FLOAT, 0, 0, 0, NULL }


struct  _Fx9__Effect__Hlsl__Constant
{
  ProtobufCMessage base;
  Fx9__Effect__Hlsl__UniformType type;
  uint32_t index;
};
#define FX9__EFFECT__HLSL__CONSTANT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__hlsl__constant__descriptor) \
    , FX9__EFFECT__HLSL__UNIFORM_TYPE__UT_FLOAT, 0 }


struct  _Fx9__Effect__Hlsl__Sampler
{
  ProtobufCMessage base;
  Fx9__Effect__Hlsl__SamplerType type;
  uint32_t index;
  char *name;
};
#define FX9__EFFECT__HLSL__SAMPLER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__hlsl__sampler__descriptor) \
    , FX9__EFFECT__HLSL__SAMPLER_TYPE__SAMPLER_2D, 0, NULL }


struct  _Fx9__Effect__Hlsl__Attribute
{
  ProtobufCMessage base;
  Fx9__Effect__Hlsl__AttributeUsage usage;
  uint32_t index;
  char *name;
};
#define FX9__EFFECT__HLSL__ATTRIBUTE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__hlsl__attribute__descriptor) \
    , FX9__EFFECT__HLSL__ATTRIBUTE_USAGE__AU_POSITION, 0, NULL }


struct  _Fx9__Effect__Hlsl__Symbol
{
  ProtobufCMessage base;
  char *name;
  Fx9__Effect__Hlsl__RegisterSet register_set;
  uint32_t register_index;
  uint32_t register_count;
};
#define FX9__EFFECT__HLSL__SYMBOL__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__hlsl__symbol__descriptor) \
    , NULL, FX9__EFFECT__HLSL__REGISTER_SET__RS_BOOL, 0, 0 }


struct  _Fx9__Effect__Hlsl__SamplerState
{
  ProtobufCMessage base;
  uint32_t key;
  uint32_t value;
};
#define FX9__EFFECT__HLSL__SAMPLER_STATE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__hlsl__sampler_state__descriptor) \
    , 0, 0 }


struct  _Fx9__Effect__Hlsl__RenderState
{
  ProtobufCMessage base;
  uint32_t key;
  uint32_t value;
};
#define FX9__EFFECT__HLSL__RENDER_STATE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__hlsl__render_state__descriptor) \
    , 0, 0 }


struct  _Fx9__Effect__Hlsl__Texture
{
  ProtobufCMessage base;
  char *name;
  uint32_t sampler_index;
  size_t n_sampler_states;
  Fx9__Effect__Hlsl__SamplerState **sampler_states;
};
#define FX9__EFFECT__HLSL__TEXTURE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__hlsl__texture__descriptor) \
    , NULL, 0, 0,NULL }


struct  _Fx9__Effect__Hlsl__Shader
{
  ProtobufCMessage base;
  uint32_t major_version;
  uint32_t minor_version;
  uint32_t num_intructions;
  Fx9__Effect__Hlsl__ShaderType type;
  char *code;
  size_t n_uniforms;
  Fx9__Effect__Hlsl__Uniform **uniforms;
  size_t n_constants;
  Fx9__Effect__Hlsl__Constant **constants;
  size_t n_samplers;
  Fx9__Effect__Hlsl__Sampler **samplers;
  size_t n_attributes;
  Fx9__Effect__Hlsl__Attribute **attributes;
  size_t n_outputs;
  Fx9__Effect__Hlsl__Attribute **outputs;
  size_t n_symbols;
  Fx9__Effect__Hlsl__Symbol **symbols;
};
#define FX9__EFFECT__HLSL__SHADER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__hlsl__shader__descriptor) \
    , 0, 0, 0, FX9__EFFECT__HLSL__SHADER_TYPE__ST_PIXEL, NULL, 0,NULL, 0,NULL, 0,NULL, 0,NULL, 0,NULL, 0,NULL }


struct  _Fx9__Effect__Hlsl__Pass
{
  ProtobufCMessage base;
  Fx9__Effect__Hlsl__Shader *vertex_shader;
  Fx9__Effect__Hlsl__Shader *pixel_shader;
  size_t n_textures;
  Fx9__Effect__Hlsl__Texture **textures;
  size_t n_render_states;
  Fx9__Effect__Hlsl__RenderState **render_states;
  size_t n_vertex_textures;
  Fx9__Effect__Hlsl__Texture **vertex_textures;
};
#define FX9__EFFECT__HLSL__PASS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__hlsl__pass__descriptor) \
    , NULL, NULL, 0,NULL, 0,NULL, 0,NULL }


/* Fx9__Effect__Hlsl__Uniform methods */
void   fx9__effect__hlsl__uniform__init
                     (Fx9__Effect__Hlsl__Uniform         *message);
size_t fx9__effect__hlsl__uniform__get_packed_size
                     (const Fx9__Effect__Hlsl__Uniform   *message);
size_t fx9__effect__hlsl__uniform__pack
                     (const Fx9__Effect__Hlsl__Uniform   *message,
                      uint8_t             *out);
size_t fx9__effect__hlsl__uniform__pack_to_buffer
                     (const Fx9__Effect__Hlsl__Uniform   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Hlsl__Uniform *
       fx9__effect__hlsl__uniform__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__hlsl__uniform__free_unpacked
                     (Fx9__Effect__Hlsl__Uniform *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Hlsl__Constant methods */
void   fx9__effect__hlsl__constant__init
                     (Fx9__Effect__Hlsl__Constant         *message);
size_t fx9__effect__hlsl__constant__get_packed_size
                     (const Fx9__Effect__Hlsl__Constant   *message);
size_t fx9__effect__hlsl__constant__pack
                     (const Fx9__Effect__Hlsl__Constant   *message,
                      uint8_t             *out);
size_t fx9__effect__hlsl__constant__pack_to_buffer
                     (const Fx9__Effect__Hlsl__Constant   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Hlsl__Constant *
       fx9__effect__hlsl__constant__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__hlsl__constant__free_unpacked
                     (Fx9__Effect__Hlsl__Constant *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Hlsl__Sampler methods */
void   fx9__effect__hlsl__sampler__init
                     (Fx9__Effect__Hlsl__Sampler         *message);
size_t fx9__effect__hlsl__sampler__get_packed_size
                     (const Fx9__Effect__Hlsl__Sampler   *message);
size_t fx9__effect__hlsl__sampler__pack
                     (const Fx9__Effect__Hlsl__Sampler   *message,
                      uint8_t             *out);
size_t fx9__effect__hlsl__sampler__pack_to_buffer
                     (const Fx9__Effect__Hlsl__Sampler   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Hlsl__Sampler *
       fx9__effect__hlsl__sampler__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__hlsl__sampler__free_unpacked
                     (Fx9__Effect__Hlsl__Sampler *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Hlsl__Attribute methods */
void   fx9__effect__hlsl__attribute__init
                     (Fx9__Effect__Hlsl__Attribute         *message);
size_t fx9__effect__hlsl__attribute__get_packed_size
                     (const Fx9__Effect__Hlsl__Attribute   *message);
size_t fx9__effect__hlsl__attribute__pack
                     (const Fx9__Effect__Hlsl__Attribute   *message,
                      uint8_t             *out);
size_t fx9__effect__hlsl__attribute__pack_to_buffer
                     (const Fx9__Effect__Hlsl__Attribute   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Hlsl__Attribute *
       fx9__effect__hlsl__attribute__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__hlsl__attribute__free_unpacked
                     (Fx9__Effect__Hlsl__Attribute *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Hlsl__Symbol methods */
void   fx9__effect__hlsl__symbol__init
                     (Fx9__Effect__Hlsl__Symbol         *message);
size_t fx9__effect__hlsl__symbol__get_packed_size
                     (const Fx9__Effect__Hlsl__Symbol   *message);
size_t fx9__effect__hlsl__symbol__pack
                     (const Fx9__Effect__Hlsl__Symbol   *message,
                      uint8_t             *out);
size_t fx9__effect__hlsl__symbol__pack_to_buffer
                     (const Fx9__Effect__Hlsl__Symbol   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Hlsl__Symbol *
       fx9__effect__hlsl__symbol__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__hlsl__symbol__free_unpacked
                     (Fx9__Effect__Hlsl__Symbol *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Hlsl__SamplerState methods */
void   fx9__effect__hlsl__sampler_state__init
                     (Fx9__Effect__Hlsl__SamplerState         *message);
size_t fx9__effect__hlsl__sampler_state__get_packed_size
                     (const Fx9__Effect__Hlsl__SamplerState   *message);
size_t fx9__effect__hlsl__sampler_state__pack
                     (const Fx9__Effect__Hlsl__SamplerState   *message,
                      uint8_t             *out);
size_t fx9__effect__hlsl__sampler_state__pack_to_buffer
                     (const Fx9__Effect__Hlsl__SamplerState   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Hlsl__SamplerState *
       fx9__effect__hlsl__sampler_state__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__hlsl__sampler_state__free_unpacked
                     (Fx9__Effect__Hlsl__SamplerState *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Hlsl__RenderState methods */
void   fx9__effect__hlsl__render_state__init
                     (Fx9__Effect__Hlsl__RenderState         *message);
size_t fx9__effect__hlsl__render_state__get_packed_size
                     (const Fx9__Effect__Hlsl__RenderState   *message);
size_t fx9__effect__hlsl__render_state__pack
                     (const Fx9__Effect__Hlsl__RenderState   *message,
                      uint8_t             *out);
size_t fx9__effect__hlsl__render_state__pack_to_buffer
                     (const Fx9__Effect__Hlsl__RenderState   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Hlsl__RenderState *
       fx9__effect__hlsl__render_state__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__hlsl__render_state__free_unpacked
                     (Fx9__Effect__Hlsl__RenderState *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Hlsl__Texture methods */
void   fx9__effect__hlsl__texture__init
                     (Fx9__Effect__Hlsl__Texture         *message);
size_t fx9__effect__hlsl__texture__get_packed_size
                     (const Fx9__Effect__Hlsl__Texture   *message);
size_t fx9__effect__hlsl__texture__pack
                     (const Fx9__Effect__Hlsl__Texture   *message,
                      uint8_t             *out);
size_t fx9__effect__hlsl__texture__pack_to_buffer
                     (const Fx9__Effect__Hlsl__Texture   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Hlsl__Texture *
       fx9__effect__hlsl__texture__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__hlsl__texture__free_unpacked
                     (Fx9__Effect__Hlsl__Texture *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Hlsl__Shader methods */
void   fx9__effect__hlsl__shader__init
                     (Fx9__Effect__Hlsl__Shader         *message);
size_t fx9__effect__hlsl__shader__get_packed_size
                     (const Fx9__Effect__Hlsl__Shader   *message);
size_t fx9__effect__hlsl__shader__pack
                     (const Fx9__Effect__Hlsl__Shader   *message,
                      uint8_t             *out);
size_t fx9__effect__hlsl__shader__pack_to_buffer
                     (const Fx9__Effect__Hlsl__Shader   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Hlsl__Shader *
       fx9__effect__hlsl__shader__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__hlsl__shader__free_unpacked
                     (Fx9__Effect__Hlsl__Shader *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Hlsl__Pass methods */
void   fx9__effect__hlsl__pass__init
                     (Fx9__Effect__Hlsl__Pass         *message);
size_t fx9__effect__hlsl__pass__get_packed_size
                     (const Fx9__Effect__Hlsl__Pass   *message);
size_t fx9__effect__hlsl__pass__pack
                     (const Fx9__Effect__Hlsl__Pass   *message,
                      uint8_t             *out);
size_t fx9__effect__hlsl__pass__pack_to_buffer
                     (const Fx9__Effect__Hlsl__Pass   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Hlsl__Pass *
       fx9__effect__hlsl__pass__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__hlsl__pass__free_unpacked
                     (Fx9__Effect__Hlsl__Pass *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Fx9__Effect__Hlsl__Uniform_Closure)
                 (const Fx9__Effect__Hlsl__Uniform *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Hlsl__Constant_Closure)
                 (const Fx9__Effect__Hlsl__Constant *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Hlsl__Sampler_Closure)
                 (const Fx9__Effect__Hlsl__Sampler *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Hlsl__Attribute_Closure)
                 (const Fx9__Effect__Hlsl__Attribute *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Hlsl__Symbol_Closure)
                 (const Fx9__Effect__Hlsl__Symbol *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Hlsl__SamplerState_Closure)
                 (const Fx9__Effect__Hlsl__SamplerState *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Hlsl__RenderState_Closure)
                 (const Fx9__Effect__Hlsl__RenderState *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Hlsl__Texture_Closure)
                 (const Fx9__Effect__Hlsl__Texture *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Hlsl__Shader_Closure)
                 (const Fx9__Effect__Hlsl__Shader *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Hlsl__Pass_Closure)
                 (const Fx9__Effect__Hlsl__Pass *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCEnumDescriptor    fx9__effect__hlsl__parameter_class__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__hlsl__parameter_type__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__hlsl__shader_type__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__hlsl__uniform_type__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__hlsl__sampler_type__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__hlsl__attribute_usage__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__hlsl__register_set__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__hlsl__uniform__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__hlsl__constant__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__hlsl__sampler__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__hlsl__attribute__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__hlsl__symbol__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__hlsl__sampler_state__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__hlsl__render_state__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__hlsl__texture__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__hlsl__shader__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__hlsl__pass__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_effect_5fdx11_2eproto__INCLUDED */
