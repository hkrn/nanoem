/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: effect.proto */

#ifndef PROTOBUF_C_effect_2eproto__INCLUDED
#define PROTOBUF_C_effect_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1004000 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif

#include "effect_dx9ms.pb-c.h"

typedef struct Fx9__Effect__Uniform Fx9__Effect__Uniform;
typedef struct Fx9__Effect__SamplerState Fx9__Effect__SamplerState;
typedef struct Fx9__Effect__Sampler Fx9__Effect__Sampler;
typedef struct Fx9__Effect__Attribute Fx9__Effect__Attribute;
typedef struct Fx9__Effect__Symbol Fx9__Effect__Symbol;
typedef struct Fx9__Effect__RenderState Fx9__Effect__RenderState;
typedef struct Fx9__Effect__Texture Fx9__Effect__Texture;
typedef struct Fx9__Effect__Semantic Fx9__Effect__Semantic;
typedef struct Fx9__Effect__Shader Fx9__Effect__Shader;
typedef struct Fx9__Effect__Metadata Fx9__Effect__Metadata;
typedef struct Fx9__Effect__Vector4i Fx9__Effect__Vector4i;
typedef struct Fx9__Effect__Vector4f Fx9__Effect__Vector4f;
typedef struct Fx9__Effect__Parameter Fx9__Effect__Parameter;
typedef struct Fx9__Effect__Annotation Fx9__Effect__Annotation;
typedef struct Fx9__Effect__Pass Fx9__Effect__Pass;
typedef struct Fx9__Effect__Technique Fx9__Effect__Technique;
typedef struct Fx9__Effect__Include Fx9__Effect__Include;
typedef struct Fx9__Effect__Effect Fx9__Effect__Effect;


/* --- enums --- */

typedef enum _Fx9__Effect__Uniform__Type {
  FX9__EFFECT__UNIFORM__TYPE__UT_FLOAT = 0,
  FX9__EFFECT__UNIFORM__TYPE__UT_INT = 1,
  FX9__EFFECT__UNIFORM__TYPE__UT_BOOL = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__UNIFORM__TYPE)
} Fx9__Effect__Uniform__Type;
typedef enum _Fx9__Effect__Sampler__Type {
  FX9__EFFECT__SAMPLER__TYPE__SAMPLER_2D = 0,
  FX9__EFFECT__SAMPLER__TYPE__SAMPLER_CUBE = 1,
  FX9__EFFECT__SAMPLER__TYPE__SAMPLER_VOLUME = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__SAMPLER__TYPE)
} Fx9__Effect__Sampler__Type;
typedef enum _Fx9__Effect__Attribute__Usage {
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_POSITION = 0,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_BLENDWEIGHT = 1,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_BLENDINDICES = 2,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_NORMAL = 3,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_POINTSIZE = 4,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_TEXCOORD = 5,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_TANGENT = 6,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_BINORMAL = 7,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_TESSFACTOR = 8,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_POSITIONT = 9,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_COLOR = 10,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_FOG = 11,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_DEPTH = 12,
  FX9__EFFECT__ATTRIBUTE__USAGE__AU_SAMPLE = 13
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__ATTRIBUTE__USAGE)
} Fx9__Effect__Attribute__Usage;
typedef enum _Fx9__Effect__Symbol__RegisterSet {
  FX9__EFFECT__SYMBOL__REGISTER_SET__RS_BOOL = 0,
  FX9__EFFECT__SYMBOL__REGISTER_SET__RS_INT4 = 1,
  FX9__EFFECT__SYMBOL__REGISTER_SET__RS_FLOAT4 = 2,
  FX9__EFFECT__SYMBOL__REGISTER_SET__RS_SAMPLER = 3
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__SYMBOL__REGISTER_SET)
} Fx9__Effect__Symbol__RegisterSet;
typedef enum _Fx9__Effect__Shader__Type {
  FX9__EFFECT__SHADER__TYPE__ST_PIXEL = 1,
  FX9__EFFECT__SHADER__TYPE__ST_VERTEX = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__SHADER__TYPE)
} Fx9__Effect__Shader__Type;
typedef enum _Fx9__Effect__Parameter__ClassCommon {
  FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_SCALAR = 0,
  FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_VECTOR = 1,
  FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_MATRIX_ROWS = 2,
  FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_MATRIX_COLUMNS = 3,
  FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_OBJECT = 4,
  FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_STRUCT = 5
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__PARAMETER__CLASS_COMMON)
} Fx9__Effect__Parameter__ClassCommon;
typedef enum _Fx9__Effect__Parameter__TypeCommon {
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_VOID = 0,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_BOOL = 1,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_INT = 2,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_FLOAT = 3,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_STRING = 4,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE = 5,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE1D = 6,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE2D = 7,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURE3D = 8,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_TEXTURECUBE = 9,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER = 10,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER1D = 11,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER2D = 12,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLER3D = 13,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_SAMPLERCUBE = 14,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_PIXELSHADER = 15,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_VERTEXSHADER = 16,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_PIXELFRAGMENT = 17,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_VERTEXFRAGMENT = 18,
  FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_UNSUPPORTED = 19
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__PARAMETER__TYPE_COMMON)
} Fx9__Effect__Parameter__TypeCommon;

/* --- messages --- */

struct  Fx9__Effect__Uniform
{
  ProtobufCMessage base;
  Fx9__Effect__Uniform__Type type;
  uint32_t index;
  uint32_t num_elements;
  uint32_t constant_index;
  char *name;
};
#define FX9__EFFECT__UNIFORM__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__uniform__descriptor) \
    , FX9__EFFECT__UNIFORM__TYPE__UT_FLOAT, 0, 0, 0, NULL }


struct  Fx9__Effect__SamplerState
{
  ProtobufCMessage base;
  uint32_t key;
  uint32_t value;
};
#define FX9__EFFECT__SAMPLER_STATE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__sampler_state__descriptor) \
    , 0, 0 }


struct  Fx9__Effect__Sampler
{
  ProtobufCMessage base;
  Fx9__Effect__Sampler__Type type;
  uint32_t index;
  char *sampler_name;
  char *texture_name;
  size_t n_sampler_states;
  Fx9__Effect__SamplerState **sampler_states;
};
#define FX9__EFFECT__SAMPLER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__sampler__descriptor) \
    , FX9__EFFECT__SAMPLER__TYPE__SAMPLER_2D, 0, NULL, NULL, 0,NULL }


struct  Fx9__Effect__Attribute
{
  ProtobufCMessage base;
  Fx9__Effect__Attribute__Usage usage;
  uint32_t index;
  char *name;
};
#define FX9__EFFECT__ATTRIBUTE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__attribute__descriptor) \
    , FX9__EFFECT__ATTRIBUTE__USAGE__AU_POSITION, 0, NULL }


struct  Fx9__Effect__Symbol
{
  ProtobufCMessage base;
  char *name;
  Fx9__Effect__Symbol__RegisterSet register_set;
  uint32_t register_index;
  uint32_t register_count;
};
#define FX9__EFFECT__SYMBOL__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__symbol__descriptor) \
    , NULL, FX9__EFFECT__SYMBOL__REGISTER_SET__RS_BOOL, 0, 0 }


struct  Fx9__Effect__RenderState
{
  ProtobufCMessage base;
  uint32_t key;
  uint32_t value;
};
#define FX9__EFFECT__RENDER_STATE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__render_state__descriptor) \
    , 0, 0 }


struct  Fx9__Effect__Texture
{
  ProtobufCMessage base;
  uint32_t index;
  char *name;
};
#define FX9__EFFECT__TEXTURE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__texture__descriptor) \
    , 0, NULL }


struct  Fx9__Effect__Semantic
{
  ProtobufCMessage base;
  uint32_t index;
  char *input_name;
  char *output_name;
  char *parameter_name;
};
#define FX9__EFFECT__SEMANTIC__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__semantic__descriptor) \
    , 0, NULL, NULL, NULL }


typedef enum {
  FX9__EFFECT__SHADER__BODY__NOT_SET = 0,
  FX9__EFFECT__SHADER__BODY_GLSL = 8,
  FX9__EFFECT__SHADER__BODY_MSL = 9,
  FX9__EFFECT__SHADER__BODY_HLSL = 10,
  FX9__EFFECT__SHADER__BODY_SPIRV = 11,
  FX9__EFFECT__SHADER__BODY_WGSL = 12
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__SHADER__BODY__CASE)
} Fx9__Effect__Shader__BodyCase;

struct  Fx9__Effect__Shader
{
  ProtobufCMessage base;
  Fx9__Effect__Shader__Type type;
  size_t n_uniforms;
  Fx9__Effect__Uniform **uniforms;
  size_t n_samplers;
  Fx9__Effect__Sampler **samplers;
  size_t n_textures;
  Fx9__Effect__Texture **textures;
  size_t n_inputs;
  Fx9__Effect__Attribute **inputs;
  size_t n_outputs;
  Fx9__Effect__Attribute **outputs;
  size_t n_symbols;
  Fx9__Effect__Symbol **symbols;
  size_t n_semantics;
  Fx9__Effect__Semantic **semantics;
  char *uniform_block_name;
  Fx9__Effect__Shader__BodyCase body_case;
  union {
    char *glsl;
    char *msl;
    char *hlsl;
    ProtobufCBinaryData spirv;
    char *wgsl;
  };
};
#define FX9__EFFECT__SHADER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__shader__descriptor) \
    , FX9__EFFECT__SHADER__TYPE__ST_PIXEL, 0,NULL, 0,NULL, 0,NULL, 0,NULL, 0,NULL, 0,NULL, 0,NULL, NULL, FX9__EFFECT__SHADER__BODY__NOT_SET, {0} }


struct  Fx9__Effect__Metadata
{
  ProtobufCMessage base;
  char *name;
  char *value;
};
#define FX9__EFFECT__METADATA__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__metadata__descriptor) \
    , NULL, NULL }


struct  Fx9__Effect__Vector4i
{
  ProtobufCMessage base;
  int32_t x;
  int32_t y;
  int32_t z;
  int32_t w;
};
#define FX9__EFFECT__VECTOR4I__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__vector4i__descriptor) \
    , 0, 0, 0, 0 }


struct  Fx9__Effect__Vector4f
{
  ProtobufCMessage base;
  float x;
  float y;
  float z;
  float w;
};
#define FX9__EFFECT__VECTOR4F__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__vector4f__descriptor) \
    , 0, 0, 0, 0 }


typedef enum {
  FX9__EFFECT__PARAMETER__CLASS__NOT_SET = 0,
  FX9__EFFECT__PARAMETER__CLASS_CLASS_DX9MS = 9
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__PARAMETER__CLASS__CASE)
} Fx9__Effect__Parameter__ClassCase;

typedef enum {
  FX9__EFFECT__PARAMETER__TYPE__NOT_SET = 0,
  FX9__EFFECT__PARAMETER__TYPE_TYPE_DX9MS = 10
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__PARAMETER__TYPE__CASE)
} Fx9__Effect__Parameter__TypeCase;

struct  Fx9__Effect__Parameter
{
  ProtobufCMessage base;
  char *name;
  char *semantic;
  uint32_t num_rows;
  uint32_t num_columns;
  uint32_t num_elements;
  size_t n_annotations;
  Fx9__Effect__Annotation **annotations;
  size_t n_struct_members;
  Fx9__Effect__Parameter **struct_members;
  uint32_t flags;
  ProtobufCBinaryData value;
  protobuf_c_boolean has_class_common;
  Fx9__Effect__Parameter__ClassCommon class_common;
  protobuf_c_boolean has_type_common;
  Fx9__Effect__Parameter__TypeCommon type_common;
  Fx9__Effect__Parameter__ClassCase class_case;
  union {
    Fx9__Effect__Dx9ms__ParameterClass class_dx9ms;
  };
  Fx9__Effect__Parameter__TypeCase type_case;
  union {
    Fx9__Effect__Dx9ms__ParameterType type_dx9ms;
  };
};
#define FX9__EFFECT__PARAMETER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__parameter__descriptor) \
    , NULL, NULL, 0, 0, 0, 0,NULL, 0,NULL, 0, {0,NULL}, 0, FX9__EFFECT__PARAMETER__CLASS_COMMON__PC_SCALAR, 0, FX9__EFFECT__PARAMETER__TYPE_COMMON__PT_VOID, FX9__EFFECT__PARAMETER__CLASS__NOT_SET, {0}, FX9__EFFECT__PARAMETER__TYPE__NOT_SET, {0} }


typedef enum {
  FX9__EFFECT__ANNOTATION__VALUE__NOT_SET = 0,
  FX9__EFFECT__ANNOTATION__VALUE_BVAL = 2,
  FX9__EFFECT__ANNOTATION__VALUE_FVAL = 3,
  FX9__EFFECT__ANNOTATION__VALUE_IVAL = 4,
  FX9__EFFECT__ANNOTATION__VALUE_SVAL = 5,
  FX9__EFFECT__ANNOTATION__VALUE_IVAL4 = 6,
  FX9__EFFECT__ANNOTATION__VALUE_FVAL4 = 7,
  FX9__EFFECT__ANNOTATION__VALUE_SVAL_UTF8 = 8
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__ANNOTATION__VALUE__CASE)
} Fx9__Effect__Annotation__ValueCase;

struct  Fx9__Effect__Annotation
{
  ProtobufCMessage base;
  char *name;
  Fx9__Effect__Annotation__ValueCase value_case;
  union {
    protobuf_c_boolean bval;
    float fval;
    int32_t ival;
    char *sval;
    Fx9__Effect__Vector4i *ival4;
    Fx9__Effect__Vector4f *fval4;
    char *sval_utf8;
  };
};
#define FX9__EFFECT__ANNOTATION__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__annotation__descriptor) \
    , NULL, FX9__EFFECT__ANNOTATION__VALUE__NOT_SET, {0} }


typedef enum {
  FX9__EFFECT__PASS__IMPLEMENTATION__NOT_SET = 0,
  FX9__EFFECT__PASS__IMPLEMENTATION_IMPLEMENTATION_DX9MS = 3
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(FX9__EFFECT__PASS__IMPLEMENTATION__CASE)
} Fx9__Effect__Pass__ImplementationCase;

struct  Fx9__Effect__Pass
{
  ProtobufCMessage base;
  char *name;
  size_t n_annotations;
  Fx9__Effect__Annotation **annotations;
  Fx9__Effect__Shader *vertex_shader;
  Fx9__Effect__Shader *pixel_shader;
  size_t n_render_states;
  Fx9__Effect__RenderState **render_states;
  Fx9__Effect__Pass__ImplementationCase implementation_case;
  union {
    Fx9__Effect__Dx9ms__Pass *implementation_dx9ms;
  };
};
#define FX9__EFFECT__PASS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__pass__descriptor) \
    , NULL, 0,NULL, NULL, NULL, 0,NULL, FX9__EFFECT__PASS__IMPLEMENTATION__NOT_SET, {0} }


struct  Fx9__Effect__Technique
{
  ProtobufCMessage base;
  char *name;
  size_t n_passes;
  Fx9__Effect__Pass **passes;
  size_t n_annotations;
  Fx9__Effect__Annotation **annotations;
};
#define FX9__EFFECT__TECHNIQUE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__technique__descriptor) \
    , NULL, 0,NULL, 0,NULL }


struct  Fx9__Effect__Include
{
  ProtobufCMessage base;
  char *location;
};
#define FX9__EFFECT__INCLUDE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__include__descriptor) \
    , NULL }


struct  Fx9__Effect__Effect
{
  ProtobufCMessage base;
  size_t n_metadata;
  Fx9__Effect__Metadata **metadata;
  size_t n_parameters;
  Fx9__Effect__Parameter **parameters;
  size_t n_techniques;
  Fx9__Effect__Technique **techniques;
  size_t n_compat_included_paths;
  char **compat_included_paths;
  size_t n_includes;
  Fx9__Effect__Include **includes;
};
#define FX9__EFFECT__EFFECT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&fx9__effect__effect__descriptor) \
    , 0,NULL, 0,NULL, 0,NULL, 0,NULL, 0,NULL }


/* Fx9__Effect__Uniform methods */
void   fx9__effect__uniform__init
                     (Fx9__Effect__Uniform         *message);
size_t fx9__effect__uniform__get_packed_size
                     (const Fx9__Effect__Uniform   *message);
size_t fx9__effect__uniform__pack
                     (const Fx9__Effect__Uniform   *message,
                      uint8_t             *out);
size_t fx9__effect__uniform__pack_to_buffer
                     (const Fx9__Effect__Uniform   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Uniform *
       fx9__effect__uniform__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__uniform__free_unpacked
                     (Fx9__Effect__Uniform *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__SamplerState methods */
void   fx9__effect__sampler_state__init
                     (Fx9__Effect__SamplerState         *message);
size_t fx9__effect__sampler_state__get_packed_size
                     (const Fx9__Effect__SamplerState   *message);
size_t fx9__effect__sampler_state__pack
                     (const Fx9__Effect__SamplerState   *message,
                      uint8_t             *out);
size_t fx9__effect__sampler_state__pack_to_buffer
                     (const Fx9__Effect__SamplerState   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__SamplerState *
       fx9__effect__sampler_state__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__sampler_state__free_unpacked
                     (Fx9__Effect__SamplerState *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Sampler methods */
void   fx9__effect__sampler__init
                     (Fx9__Effect__Sampler         *message);
size_t fx9__effect__sampler__get_packed_size
                     (const Fx9__Effect__Sampler   *message);
size_t fx9__effect__sampler__pack
                     (const Fx9__Effect__Sampler   *message,
                      uint8_t             *out);
size_t fx9__effect__sampler__pack_to_buffer
                     (const Fx9__Effect__Sampler   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Sampler *
       fx9__effect__sampler__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__sampler__free_unpacked
                     (Fx9__Effect__Sampler *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Attribute methods */
void   fx9__effect__attribute__init
                     (Fx9__Effect__Attribute         *message);
size_t fx9__effect__attribute__get_packed_size
                     (const Fx9__Effect__Attribute   *message);
size_t fx9__effect__attribute__pack
                     (const Fx9__Effect__Attribute   *message,
                      uint8_t             *out);
size_t fx9__effect__attribute__pack_to_buffer
                     (const Fx9__Effect__Attribute   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Attribute *
       fx9__effect__attribute__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__attribute__free_unpacked
                     (Fx9__Effect__Attribute *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Symbol methods */
void   fx9__effect__symbol__init
                     (Fx9__Effect__Symbol         *message);
size_t fx9__effect__symbol__get_packed_size
                     (const Fx9__Effect__Symbol   *message);
size_t fx9__effect__symbol__pack
                     (const Fx9__Effect__Symbol   *message,
                      uint8_t             *out);
size_t fx9__effect__symbol__pack_to_buffer
                     (const Fx9__Effect__Symbol   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Symbol *
       fx9__effect__symbol__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__symbol__free_unpacked
                     (Fx9__Effect__Symbol *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__RenderState methods */
void   fx9__effect__render_state__init
                     (Fx9__Effect__RenderState         *message);
size_t fx9__effect__render_state__get_packed_size
                     (const Fx9__Effect__RenderState   *message);
size_t fx9__effect__render_state__pack
                     (const Fx9__Effect__RenderState   *message,
                      uint8_t             *out);
size_t fx9__effect__render_state__pack_to_buffer
                     (const Fx9__Effect__RenderState   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__RenderState *
       fx9__effect__render_state__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__render_state__free_unpacked
                     (Fx9__Effect__RenderState *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Texture methods */
void   fx9__effect__texture__init
                     (Fx9__Effect__Texture         *message);
size_t fx9__effect__texture__get_packed_size
                     (const Fx9__Effect__Texture   *message);
size_t fx9__effect__texture__pack
                     (const Fx9__Effect__Texture   *message,
                      uint8_t             *out);
size_t fx9__effect__texture__pack_to_buffer
                     (const Fx9__Effect__Texture   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Texture *
       fx9__effect__texture__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__texture__free_unpacked
                     (Fx9__Effect__Texture *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Semantic methods */
void   fx9__effect__semantic__init
                     (Fx9__Effect__Semantic         *message);
size_t fx9__effect__semantic__get_packed_size
                     (const Fx9__Effect__Semantic   *message);
size_t fx9__effect__semantic__pack
                     (const Fx9__Effect__Semantic   *message,
                      uint8_t             *out);
size_t fx9__effect__semantic__pack_to_buffer
                     (const Fx9__Effect__Semantic   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Semantic *
       fx9__effect__semantic__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__semantic__free_unpacked
                     (Fx9__Effect__Semantic *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Shader methods */
void   fx9__effect__shader__init
                     (Fx9__Effect__Shader         *message);
size_t fx9__effect__shader__get_packed_size
                     (const Fx9__Effect__Shader   *message);
size_t fx9__effect__shader__pack
                     (const Fx9__Effect__Shader   *message,
                      uint8_t             *out);
size_t fx9__effect__shader__pack_to_buffer
                     (const Fx9__Effect__Shader   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Shader *
       fx9__effect__shader__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__shader__free_unpacked
                     (Fx9__Effect__Shader *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Metadata methods */
void   fx9__effect__metadata__init
                     (Fx9__Effect__Metadata         *message);
size_t fx9__effect__metadata__get_packed_size
                     (const Fx9__Effect__Metadata   *message);
size_t fx9__effect__metadata__pack
                     (const Fx9__Effect__Metadata   *message,
                      uint8_t             *out);
size_t fx9__effect__metadata__pack_to_buffer
                     (const Fx9__Effect__Metadata   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Metadata *
       fx9__effect__metadata__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__metadata__free_unpacked
                     (Fx9__Effect__Metadata *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Vector4i methods */
void   fx9__effect__vector4i__init
                     (Fx9__Effect__Vector4i         *message);
size_t fx9__effect__vector4i__get_packed_size
                     (const Fx9__Effect__Vector4i   *message);
size_t fx9__effect__vector4i__pack
                     (const Fx9__Effect__Vector4i   *message,
                      uint8_t             *out);
size_t fx9__effect__vector4i__pack_to_buffer
                     (const Fx9__Effect__Vector4i   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Vector4i *
       fx9__effect__vector4i__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__vector4i__free_unpacked
                     (Fx9__Effect__Vector4i *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Vector4f methods */
void   fx9__effect__vector4f__init
                     (Fx9__Effect__Vector4f         *message);
size_t fx9__effect__vector4f__get_packed_size
                     (const Fx9__Effect__Vector4f   *message);
size_t fx9__effect__vector4f__pack
                     (const Fx9__Effect__Vector4f   *message,
                      uint8_t             *out);
size_t fx9__effect__vector4f__pack_to_buffer
                     (const Fx9__Effect__Vector4f   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Vector4f *
       fx9__effect__vector4f__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__vector4f__free_unpacked
                     (Fx9__Effect__Vector4f *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Parameter methods */
void   fx9__effect__parameter__init
                     (Fx9__Effect__Parameter         *message);
size_t fx9__effect__parameter__get_packed_size
                     (const Fx9__Effect__Parameter   *message);
size_t fx9__effect__parameter__pack
                     (const Fx9__Effect__Parameter   *message,
                      uint8_t             *out);
size_t fx9__effect__parameter__pack_to_buffer
                     (const Fx9__Effect__Parameter   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Parameter *
       fx9__effect__parameter__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__parameter__free_unpacked
                     (Fx9__Effect__Parameter *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Annotation methods */
void   fx9__effect__annotation__init
                     (Fx9__Effect__Annotation         *message);
size_t fx9__effect__annotation__get_packed_size
                     (const Fx9__Effect__Annotation   *message);
size_t fx9__effect__annotation__pack
                     (const Fx9__Effect__Annotation   *message,
                      uint8_t             *out);
size_t fx9__effect__annotation__pack_to_buffer
                     (const Fx9__Effect__Annotation   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Annotation *
       fx9__effect__annotation__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__annotation__free_unpacked
                     (Fx9__Effect__Annotation *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Pass methods */
void   fx9__effect__pass__init
                     (Fx9__Effect__Pass         *message);
size_t fx9__effect__pass__get_packed_size
                     (const Fx9__Effect__Pass   *message);
size_t fx9__effect__pass__pack
                     (const Fx9__Effect__Pass   *message,
                      uint8_t             *out);
size_t fx9__effect__pass__pack_to_buffer
                     (const Fx9__Effect__Pass   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Pass *
       fx9__effect__pass__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__pass__free_unpacked
                     (Fx9__Effect__Pass *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Technique methods */
void   fx9__effect__technique__init
                     (Fx9__Effect__Technique         *message);
size_t fx9__effect__technique__get_packed_size
                     (const Fx9__Effect__Technique   *message);
size_t fx9__effect__technique__pack
                     (const Fx9__Effect__Technique   *message,
                      uint8_t             *out);
size_t fx9__effect__technique__pack_to_buffer
                     (const Fx9__Effect__Technique   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Technique *
       fx9__effect__technique__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__technique__free_unpacked
                     (Fx9__Effect__Technique *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Include methods */
void   fx9__effect__include__init
                     (Fx9__Effect__Include         *message);
size_t fx9__effect__include__get_packed_size
                     (const Fx9__Effect__Include   *message);
size_t fx9__effect__include__pack
                     (const Fx9__Effect__Include   *message,
                      uint8_t             *out);
size_t fx9__effect__include__pack_to_buffer
                     (const Fx9__Effect__Include   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Include *
       fx9__effect__include__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__include__free_unpacked
                     (Fx9__Effect__Include *message,
                      ProtobufCAllocator *allocator);
/* Fx9__Effect__Effect methods */
void   fx9__effect__effect__init
                     (Fx9__Effect__Effect         *message);
size_t fx9__effect__effect__get_packed_size
                     (const Fx9__Effect__Effect   *message);
size_t fx9__effect__effect__pack
                     (const Fx9__Effect__Effect   *message,
                      uint8_t             *out);
size_t fx9__effect__effect__pack_to_buffer
                     (const Fx9__Effect__Effect   *message,
                      ProtobufCBuffer     *buffer);
Fx9__Effect__Effect *
       fx9__effect__effect__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   fx9__effect__effect__free_unpacked
                     (Fx9__Effect__Effect *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Fx9__Effect__Uniform_Closure)
                 (const Fx9__Effect__Uniform *message,
                  void *closure_data);
typedef void (*Fx9__Effect__SamplerState_Closure)
                 (const Fx9__Effect__SamplerState *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Sampler_Closure)
                 (const Fx9__Effect__Sampler *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Attribute_Closure)
                 (const Fx9__Effect__Attribute *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Symbol_Closure)
                 (const Fx9__Effect__Symbol *message,
                  void *closure_data);
typedef void (*Fx9__Effect__RenderState_Closure)
                 (const Fx9__Effect__RenderState *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Texture_Closure)
                 (const Fx9__Effect__Texture *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Semantic_Closure)
                 (const Fx9__Effect__Semantic *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Shader_Closure)
                 (const Fx9__Effect__Shader *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Metadata_Closure)
                 (const Fx9__Effect__Metadata *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Vector4i_Closure)
                 (const Fx9__Effect__Vector4i *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Vector4f_Closure)
                 (const Fx9__Effect__Vector4f *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Parameter_Closure)
                 (const Fx9__Effect__Parameter *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Annotation_Closure)
                 (const Fx9__Effect__Annotation *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Pass_Closure)
                 (const Fx9__Effect__Pass *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Technique_Closure)
                 (const Fx9__Effect__Technique *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Include_Closure)
                 (const Fx9__Effect__Include *message,
                  void *closure_data);
typedef void (*Fx9__Effect__Effect_Closure)
                 (const Fx9__Effect__Effect *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor fx9__effect__uniform__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__uniform__type__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__sampler_state__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__sampler__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__sampler__type__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__attribute__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__attribute__usage__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__symbol__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__symbol__register_set__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__render_state__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__texture__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__semantic__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__shader__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__shader__type__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__metadata__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__vector4i__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__vector4f__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__parameter__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__parameter__class_common__descriptor;
extern const ProtobufCEnumDescriptor    fx9__effect__parameter__type_common__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__annotation__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__pass__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__technique__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__include__descriptor;
extern const ProtobufCMessageDescriptor fx9__effect__effect__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_effect_2eproto__INCLUDED */
