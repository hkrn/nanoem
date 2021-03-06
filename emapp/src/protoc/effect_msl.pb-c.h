/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: effect_msl.proto */

#ifndef PROTOBUF_C_effect_5fmsl_2eproto__INCLUDED
#define PROTOBUF_C_effect_5fmsl_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _Fx9__Effect__Msl__Uniform Fx9__Effect__Msl__Uniform;
typedef struct _Fx9__Effect__Msl__Constant Fx9__Effect__Msl__Constant;
typedef struct _Fx9__Effect__Msl__Sampler Fx9__Effect__Msl__Sampler;
typedef struct _Fx9__Effect__Msl__Attribute Fx9__Effect__Msl__Attribute;
typedef struct _Fx9__Effect__Msl__Symbol Fx9__Effect__Msl__Symbol;
typedef struct _Fx9__Effect__Msl__SamplerState Fx9__Effect__Msl__SamplerState;
typedef struct _Fx9__Effect__Msl__RenderState Fx9__Effect__Msl__RenderState;
typedef struct _Fx9__Effect__Msl__Texture Fx9__Effect__Msl__Texture;
typedef struct _Fx9__Effect__Msl__Shader Fx9__Effect__Msl__Shader;
typedef struct _Fx9__Effect__Msl__Pass Fx9__Effect__Msl__Pass;


/* --- enums --- */

typedef enum _Fx9__Effect__Msl__ParameterClass {
  FX9__EFFECT__MSL__PARAMETER_CLASS__PC_SCALAR = 0,
  FX9__EFFECT__MSL__PARAMETER_CLASS__PC_VECTOR = 1,
  FX9__EFFECT__MSL__PARAMETER_CLASS__PC_MATRIX_ROWS = 2,
  FX9__EFFECT__MSL__PARAMETER_CLASS__PC_MATRIX_COLUMNS = 3,
  FX9__EFFECT__MSL__PARAMETER_CLASS__PC_OBJECT = 4,
  FX9__EFFECT__MSL__PARAMETER_CLASS__PC_STRUCT = 5
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__MSL__PARAMETER_CLASS)
} Fx9__Effect__Msl__ParameterClass;
typedef enum _Fx9__Effect__Msl__ParameterType {
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_VOID = 0,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_BOOL = 1,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_INT = 2,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_FLOAT = 3,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_STRING = 4,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_TEXTURE = 5,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_TEXTURE1D = 6,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_TEXTURE2D = 7,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_TEXTURE3D = 8,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_TEXTURECUBE = 9,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_SAMPLER = 10,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_SAMPLER1D = 11,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_SAMPLER2D = 12,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_SAMPLER3D = 13,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_SAMPLERCUBE = 14,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_PIXELSHADER = 15,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_VERTEXSHADER = 16,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_PIXELFRAGMENT = 17,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_VERTEXFRAGMENT = 18,
  FX9__EFFECT__MSL__PARAMETER_TYPE__PT_UNSUPPORTED = 19
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__MSL__PARAMETER_TYPE)
} Fx9__Effect__Msl__ParameterType;
typedef enum _Fx9__Effect__Msl__ShaderType {
  FX9__EFFECT__MSL__SHADER_TYPE__ST_PIXEL = 1,
  FX9__EFFECT__MSL__SHADER_TYPE__ST_VERTEX = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__MSL__SHADER_TYPE)
} Fx9__Effect__Msl__ShaderType;
typedef enum _Fx9__Effect__Msl__UniformType {
  FX9__EFFECT__MSL__UNIFORM_TYPE__UT_FLOAT = 0,
  FX9__EFFECT__MSL__UNIFORM_TYPE__UT_INT = 1,
  FX9__EFFECT__MSL__UNIFORM_TYPE__UT_BOOL = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__MSL__UNIFORM_TYPE)
} Fx9__Effect__Msl__UniformType;
typedef enum _Fx9__Effect__Msl__SamplerType {
  FX9__EFFECT__MSL__SAMPLER_TYPE__SAMPLER_2D = 0,
  FX9__EFFECT__MSL__SAMPLER_TYPE__SAMPLER_CUBE = 1,
  FX9__EFFECT__MSL__SAMPLER_TYPE__SAMPLER_VOLUME = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__MSL__SAMPLER_TYPE)
} Fx9__Effect__Msl__SamplerType;
typedef enum _Fx9__Effect__Msl__AttributeUsage {
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_POSITION = 0,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_BLENDWEIGHT = 1,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_BLENDINDICES = 2,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_NORMAL = 3,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_POINTSIZE = 4,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_TEXCOORD = 5,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_TANGENT = 6,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_BINORMAL = 7,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_TESSFACTOR = 8,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_POSITIONT = 9,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_COLOR = 10,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_FOG = 11,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_DEPTH = 12,
  FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_SAMPLE = 13
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__MSL__ATTRIBUTE_USAGE)
} Fx9__Effect__Msl__AttributeUsage;
typedef enum _Fx9__Effect__Msl__RegisterSet {
  FX9__EFFECT__MSL__REGISTER_SET__RS_BOOL = 0,
  FX9__EFFECT__MSL__REGISTER_SET__RS_INT4 = 1,
  FX9__EFFECT__MSL__REGISTER_SET__RS_FLOAT4 = 2,
  FX9__EFFECT__MSL__REGISTER_SET__RS_SAMPLER = 3
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__MSL__REGISTER_SET)
} Fx9__Effect__Msl__RegisterSet;

/* --- messages --- */

struct  _Fx9__Effect__Msl__Uniform
{
  ProtobufCMessage base;
  Fx9__Effect__Msl__UniformType type;
  uint32_t index;
  uint32_t num_elements;
  uint32_t constant_index;
  char *name;
};
#define FX9__EFFECT__MSL__UNIFORM__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__msl__uniform__descriptor) \
    , FX9__EFFECT__MSL__UNIFORM_TYPE__UT_FLOAT, 0, 0, 0, NULL }


struct  _Fx9__Effect__Msl__Constant
{
  ProtobufCMessage base;
  Fx9__Effect__Msl__UniformType type;
  uint32_t index;
};
#define FX9__EFFECT__MSL__CONSTANT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__msl__constant__descriptor) \
    , FX9__EFFECT__MSL__UNIFORM_TYPE__UT_FLOAT, 0 }


struct  _Fx9__Effect__Msl__Sampler
{
  ProtobufCMessage base;
  Fx9__Effect__Msl__SamplerType type;
  uint32_t index;
  char *name;
};
#define FX9__EFFECT__MSL__SAMPLER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__msl__sampler__descriptor) \
    , FX9__EFFECT__MSL__SAMPLER_TYPE__SAMPLER_2D, 0, NULL }


struct  _Fx9__Effect__Msl__Attribute
{
  ProtobufCMessage base;
  Fx9__Effect__Msl__AttributeUsage usage;
  uint32_t index;
  char *name;
};
#define FX9__EFFECT__MSL__ATTRIBUTE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__msl__attribute__descriptor) \
    , FX9__EFFECT__MSL__ATTRIBUTE_USAGE__AU_POSITION, 0, NULL }


struct  _Fx9__Effect__Msl__Symbol
{
  ProtobufCMessage base;
  char *name;
  Fx9__Effect__Msl__RegisterSet register_set;
  uint32_t register_index;
  uint32_t register_count;
};
#define FX9__EFFECT__MSL__SYMBOL__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__msl__symbol__descriptor) \
    , NULL, FX9__EFFECT__MSL__REGISTER_SET__RS_BOOL, 0, 0 }


struct  _Fx9__Effect__Msl__SamplerState
{
  ProtobufCMessage base;
  uint32_t key;
  uint32_t value;
};
#define FX9__EFFECT__MSL__SAMPLER_STATE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__msl__sampler_state__descriptor) \
    , 0, 0 }


struct  _Fx9__Effect__Msl__RenderState
{
  ProtobufCMessage base;
  uint32_t key;
  uint32_t value;
};
#define FX9__EFFECT__MSL__RENDER_STATE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__msl__render_state__descriptor) \
    , 0, 0 }


struct  _Fx9__Effect__Msl__Texture
{
  ProtobufCMessage base;
  char *name;
  uint32_t sampler_index;
  size_t n_sampler_states;
  Fx9__Effect__Msl__SamplerState **sampler_states;
};
#define FX9__EFFECT__MSL__TEXTURE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__msl__texture__descriptor) \
    , NULL, 0, 0,NULL }


struct  _Fx9__Effect__Msl__Shader
{
  ProtobufCMessage base;
  uint32_t major_version;
  uint32_t minor_version;
  uint32_t num_intructions;
  Fx9__Effect__Msl__ShaderType type;
  char *code;
  size_t n_uniforms;
  Fx9__Effect__Msl__Uniform **uniforms;
  size_t n_constants;
  Fx9__Effect__Msl__Constant **constants;
  size_t n_samplers;
  Fx9__Effect__Msl__Sampler **samplers;
  size_t n_attributes;
  Fx9__Effect__Msl__Attribute **attributes;
  size_t n_outputs;
  Fx9__Effect__Msl__Attribute **outputs;
  size_t n_symbols;
  Fx9__Effect__Msl__Symbol **symbols;
};
#define FX9__EFFECT__MSL__SHADER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__msl__shader__descriptor) \
    , 0, 0, 0, FX9__EFFECT__MSL__SHADER_TYPE__ST_PIXEL, NULL, 0,NULL, 0,NULL, 0,NULL, 0,NULL, 0,NULL, 0,NULL }


struct  _Fx9__Effect__Msl__Pass
{
  ProtobufCMessage base;
  Fx9__Effect__Msl__Shader *vertex_shader;
  Fx9__Effect__Msl__Shader *pixel_shader;
  size_t n_textures;
  Fx9__Effect__Msl__Texture **textures;
  size_t n_render_states;
  Fx9__Effect__Msl__RenderState **render_states;
  size_t n_vertex_textures;
  Fx9__Effect__Msl__Texture **vertex_textures;
};
#define FX9__EFFECT__MSL__PASS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__msl__pass__descriptor) \
    , NULL, NULL, 0,NULL, 0,NULL, 0,NULL }


/* Fx9__Effect__Msl__Uniform methods */
void   fx9__effect__msl__uniform__init
                     (Fx9__Effect__Msl__Uniform         *message);
size_t fx9__effect__msl__uniform__get_packed_size
                     (const Fx9__Effect__Msl__Uniform   *message);
size_t fx9__effect__msl__uniform__pack
                     (const Fx9__Effect__Msl__Uniform   *message,
                      uint8_t             *out);
size_t fx9__effect__msl__uniform__pack_to_buffer
                     (const Fx9__Effect__Msl__Uniform   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Msl__Uniform *
       fx9__effect__msl__uniform__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__msl__uniform__free_unpacked
                     (Fx9__Effect__Msl__Uniform *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Msl__Constant methods */
void   fx9__effect__msl__constant__init
                     (Fx9__Effect__Msl__Constant         *message);
size_t fx9__effect__msl__constant__get_packed_size
                     (const Fx9__Effect__Msl__Constant   *message);
size_t fx9__effect__msl__constant__pack
                     (const Fx9__Effect__Msl__Constant   *message,
                      uint8_t             *out);
size_t fx9__effect__msl__constant__pack_to_buffer
                     (const Fx9__Effect__Msl__Constant   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Msl__Constant *
       fx9__effect__msl__constant__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__msl__constant__free_unpacked
                     (Fx9__Effect__Msl__Constant *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Msl__Sampler methods */
void   fx9__effect__msl__sampler__init
                     (Fx9__Effect__Msl__Sampler         *message);
size_t fx9__effect__msl__sampler__get_packed_size
                     (const Fx9__Effect__Msl__Sampler   *message);
size_t fx9__effect__msl__sampler__pack
                     (const Fx9__Effect__Msl__Sampler   *message,
                      uint8_t             *out);
size_t fx9__effect__msl__sampler__pack_to_buffer
                     (const Fx9__Effect__Msl__Sampler   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Msl__Sampler *
       fx9__effect__msl__sampler__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__msl__sampler__free_unpacked
                     (Fx9__Effect__Msl__Sampler *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Msl__Attribute methods */
void   fx9__effect__msl__attribute__init
                     (Fx9__Effect__Msl__Attribute         *message);
size_t fx9__effect__msl__attribute__get_packed_size
                     (const Fx9__Effect__Msl__Attribute   *message);
size_t fx9__effect__msl__attribute__pack
                     (const Fx9__Effect__Msl__Attribute   *message,
                      uint8_t             *out);
size_t fx9__effect__msl__attribute__pack_to_buffer
                     (const Fx9__Effect__Msl__Attribute   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Msl__Attribute *
       fx9__effect__msl__attribute__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__msl__attribute__free_unpacked
                     (Fx9__Effect__Msl__Attribute *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Msl__Symbol methods */
void   fx9__effect__msl__symbol__init
                     (Fx9__Effect__Msl__Symbol         *message);
size_t fx9__effect__msl__symbol__get_packed_size
                     (const Fx9__Effect__Msl__Symbol   *message);
size_t fx9__effect__msl__symbol__pack
                     (const Fx9__Effect__Msl__Symbol   *message,
                      uint8_t             *out);
size_t fx9__effect__msl__symbol__pack_to_buffer
                     (const Fx9__Effect__Msl__Symbol   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Msl__Symbol *
       fx9__effect__msl__symbol__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__msl__symbol__free_unpacked
                     (Fx9__Effect__Msl__Symbol *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Msl__SamplerState methods */
void   fx9__effect__msl__sampler_state__init
                     (Fx9__Effect__Msl__SamplerState         *message);
size_t fx9__effect__msl__sampler_state__get_packed_size
                     (const Fx9__Effect__Msl__SamplerState   *message);
size_t fx9__effect__msl__sampler_state__pack
                     (const Fx9__Effect__Msl__SamplerState   *message,
                      uint8_t             *out);
size_t fx9__effect__msl__sampler_state__pack_to_buffer
                     (const Fx9__Effect__Msl__SamplerState   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Msl__SamplerState *
       fx9__effect__msl__sampler_state__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__msl__sampler_state__free_unpacked
                     (Fx9__Effect__Msl__SamplerState *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Msl__RenderState methods */
void   fx9__effect__msl__render_state__init
                     (Fx9__Effect__Msl__RenderState         *message);
size_t fx9__effect__msl__render_state__get_packed_size
                     (const Fx9__Effect__Msl__RenderState   *message);
size_t fx9__effect__msl__render_state__pack
                     (const Fx9__Effect__Msl__RenderState   *message,
                      uint8_t             *out);
size_t fx9__effect__msl__render_state__pack_to_buffer
                     (const Fx9__Effect__Msl__RenderState   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Msl__RenderState *
       fx9__effect__msl__render_state__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__msl__render_state__free_unpacked
                     (Fx9__Effect__Msl__RenderState *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Msl__Texture methods */
void   fx9__effect__msl__texture__init
                     (Fx9__Effect__Msl__Texture         *message);
size_t fx9__effect__msl__texture__get_packed_size
                     (const Fx9__Effect__Msl__Texture   *message);
size_t fx9__effect__msl__texture__pack
                     (const Fx9__Effect__Msl__Texture   *message,
                      uint8_t             *out);
size_t fx9__effect__msl__texture__pack_to_buffer
                     (const Fx9__Effect__Msl__Texture   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Msl__Texture *
       fx9__effect__msl__texture__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__msl__texture__free_unpacked
                     (Fx9__Effect__Msl__Texture *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Msl__Shader methods */
void   fx9__effect__msl__shader__init
                     (Fx9__Effect__Msl__Shader         *message);
size_t fx9__effect__msl__shader__get_packed_size
                     (const Fx9__Effect__Msl__Shader   *message);
size_t fx9__effect__msl__shader__pack
                     (const Fx9__Effect__Msl__Shader   *message,
                      uint8_t             *out);
size_t fx9__effect__msl__shader__pack_to_buffer
                     (const Fx9__Effect__Msl__Shader   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Msl__Shader *
       fx9__effect__msl__shader__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__msl__shader__free_unpacked
                     (Fx9__Effect__Msl__Shader *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Msl__Pass methods */
void   fx9__effect__msl__pass__init
                     (Fx9__Effect__Msl__Pass         *message);
size_t fx9__effect__msl__pass__get_packed_size
                     (const Fx9__Effect__Msl__Pass   *message);
size_t fx9__effect__msl__pass__pack
                     (const Fx9__Effect__Msl__Pass   *message,
                      uint8_t             *out);
size_t fx9__effect__msl__pass__pack_to_buffer
                     (const Fx9__Effect__Msl__Pass   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Msl__Pass *
       fx9__effect__msl__pass__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__msl__pass__free_unpacked
                     (Fx9__Effect__Msl__Pass *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Fx9__Effect__Msl__Uniform_Closure)
                 (const Fx9__Effect__Msl__Uniform *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Msl__Constant_Closure)
                 (const Fx9__Effect__Msl__Constant *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Msl__Sampler_Closure)
                 (const Fx9__Effect__Msl__Sampler *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Msl__Attribute_Closure)
                 (const Fx9__Effect__Msl__Attribute *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Msl__Symbol_Closure)
                 (const Fx9__Effect__Msl__Symbol *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Msl__SamplerState_Closure)
                 (const Fx9__Effect__Msl__SamplerState *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Msl__RenderState_Closure)
                 (const Fx9__Effect__Msl__RenderState *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Msl__Texture_Closure)
                 (const Fx9__Effect__Msl__Texture *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Msl__Shader_Closure)
                 (const Fx9__Effect__Msl__Shader *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Msl__Pass_Closure)
                 (const Fx9__Effect__Msl__Pass *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCEnumDescriptor    fx9__effect__msl__parameter_class__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__msl__parameter_type__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__msl__shader_type__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__msl__uniform_type__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__msl__sampler_type__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__msl__attribute_usage__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__msl__register_set__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__msl__uniform__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__msl__constant__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__msl__sampler__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__msl__attribute__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__msl__symbol__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__msl__sampler_state__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__msl__render_state__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__msl__texture__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__msl__shader__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__msl__pass__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_effect_5fmsl_2eproto__INCLUDED */
