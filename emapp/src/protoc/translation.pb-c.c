/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: translation.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "translation.pb-c.h"
void   nanoem__translation__phrase__init
                     (Nanoem__Translation__Phrase         *message)
{
  static const Nanoem__Translation__Phrase init_value = NANOEM__TRANSLATION__PHRASE__INIT;
  *message = init_value;
}
size_t nanoem__translation__phrase__get_packed_size
                     (const Nanoem__Translation__Phrase *message)
{
  assert(message->base.descriptor == &nanoem__translation__phrase__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t nanoem__translation__phrase__pack
                     (const Nanoem__Translation__Phrase *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &nanoem__translation__phrase__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t nanoem__translation__phrase__pack_to_buffer
                     (const Nanoem__Translation__Phrase *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &nanoem__translation__phrase__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Nanoem__Translation__Phrase *
       nanoem__translation__phrase__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Nanoem__Translation__Phrase *)
     protobuf_c_message_unpack (&nanoem__translation__phrase__descriptor,
                                allocator, len, data);
}
void   nanoem__translation__phrase__free_unpacked
                     (Nanoem__Translation__Phrase *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &nanoem__translation__phrase__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   nanoem__translation__unit__init
                     (Nanoem__Translation__Unit         *message)
{
  static const Nanoem__Translation__Unit init_value = NANOEM__TRANSLATION__UNIT__INIT;
  *message = init_value;
}
size_t nanoem__translation__unit__get_packed_size
                     (const Nanoem__Translation__Unit *message)
{
  assert(message->base.descriptor == &nanoem__translation__unit__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t nanoem__translation__unit__pack
                     (const Nanoem__Translation__Unit *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &nanoem__translation__unit__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t nanoem__translation__unit__pack_to_buffer
                     (const Nanoem__Translation__Unit *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &nanoem__translation__unit__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Nanoem__Translation__Unit *
       nanoem__translation__unit__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Nanoem__Translation__Unit *)
     protobuf_c_message_unpack (&nanoem__translation__unit__descriptor,
                                allocator, len, data);
}
void   nanoem__translation__unit__free_unpacked
                     (Nanoem__Translation__Unit *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &nanoem__translation__unit__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   nanoem__translation__bundle__init
                     (Nanoem__Translation__Bundle         *message)
{
  static const Nanoem__Translation__Bundle init_value = NANOEM__TRANSLATION__BUNDLE__INIT;
  *message = init_value;
}
size_t nanoem__translation__bundle__get_packed_size
                     (const Nanoem__Translation__Bundle *message)
{
  assert(message->base.descriptor == &nanoem__translation__bundle__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t nanoem__translation__bundle__pack
                     (const Nanoem__Translation__Bundle *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &nanoem__translation__bundle__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t nanoem__translation__bundle__pack_to_buffer
                     (const Nanoem__Translation__Bundle *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &nanoem__translation__bundle__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Nanoem__Translation__Bundle *
       nanoem__translation__bundle__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Nanoem__Translation__Bundle *)
     protobuf_c_message_unpack (&nanoem__translation__bundle__descriptor,
                                allocator, len, data);
}
void   nanoem__translation__bundle__free_unpacked
                     (Nanoem__Translation__Bundle *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &nanoem__translation__bundle__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor nanoem__translation__phrase__field_descriptors[2] =
{
  {
    "id",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(Nanoem__Translation__Phrase, id),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "text",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(Nanoem__Translation__Phrase, text),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned nanoem__translation__phrase__field_indices_by_name[] = {
  0,   /* field[0] = id */
  1,   /* field[1] = text */
};
static const ProtobufCIntRange nanoem__translation__phrase__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor nanoem__translation__phrase__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "nanoem.translation.Phrase",
  "Phrase",
  "Nanoem__Translation__Phrase",
  "nanoem.translation",
  sizeof(Nanoem__Translation__Phrase),
  2,
  nanoem__translation__phrase__field_descriptors,
  nanoem__translation__phrase__field_indices_by_name,
  1,  nanoem__translation__phrase__number_ranges,
  (ProtobufCMessageInit) nanoem__translation__phrase__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor nanoem__translation__unit__field_descriptors[2] =
{
  {
    "language",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_ENUM,
    0,   /* quantifier_offset */
    offsetof(Nanoem__Translation__Unit, language),
    &nanoem__common__language__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "phrases",
    2,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    offsetof(Nanoem__Translation__Unit, n_phrases),
    offsetof(Nanoem__Translation__Unit, phrases),
    &nanoem__translation__phrase__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned nanoem__translation__unit__field_indices_by_name[] = {
  0,   /* field[0] = language */
  1,   /* field[1] = phrases */
};
static const ProtobufCIntRange nanoem__translation__unit__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor nanoem__translation__unit__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "nanoem.translation.Unit",
  "Unit",
  "Nanoem__Translation__Unit",
  "nanoem.translation",
  sizeof(Nanoem__Translation__Unit),
  2,
  nanoem__translation__unit__field_descriptors,
  nanoem__translation__unit__field_indices_by_name,
  1,  nanoem__translation__unit__number_ranges,
  (ProtobufCMessageInit) nanoem__translation__unit__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor nanoem__translation__bundle__field_descriptors[1] =
{
  {
    "units",
    1,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    offsetof(Nanoem__Translation__Bundle, n_units),
    offsetof(Nanoem__Translation__Bundle, units),
    &nanoem__translation__unit__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned nanoem__translation__bundle__field_indices_by_name[] = {
  0,   /* field[0] = units */
};
static const ProtobufCIntRange nanoem__translation__bundle__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor nanoem__translation__bundle__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "nanoem.translation.Bundle",
  "Bundle",
  "Nanoem__Translation__Bundle",
  "nanoem.translation",
  sizeof(Nanoem__Translation__Bundle),
  1,
  nanoem__translation__bundle__field_descriptors,
  nanoem__translation__bundle__field_indices_by_name,
  1,  nanoem__translation__bundle__number_ranges,
  (ProtobufCMessageInit) nanoem__translation__bundle__init,
  NULL,NULL,NULL    /* reserved[123] */
};
