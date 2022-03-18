/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: google/type/latlng.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "latlng.pb-c.h"
void   google__type__lat_lng__init
                     (Google__Type__LatLng         *message)
{
  static const Google__Type__LatLng init_value = GOOGLE__TYPE__LAT_LNG__INIT;
  *message = init_value;
}
size_t google__type__lat_lng__get_packed_size
                     (const Google__Type__LatLng *message)
{
  assert(message->base.descriptor == &google__type__lat_lng__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t google__type__lat_lng__pack
                     (const Google__Type__LatLng *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &google__type__lat_lng__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t google__type__lat_lng__pack_to_buffer
                     (const Google__Type__LatLng *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &google__type__lat_lng__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Google__Type__LatLng *
       google__type__lat_lng__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Google__Type__LatLng *)
     protobuf_c_message_unpack (&google__type__lat_lng__descriptor,
                                allocator, len, data);
}
void   google__type__lat_lng__free_unpacked
                     (Google__Type__LatLng *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &google__type__lat_lng__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor google__type__lat_lng__field_descriptors[2] =
{
  {
    "latitude",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_DOUBLE,
    0,   /* quantifier_offset */
    offsetof(Google__Type__LatLng, latitude),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "longitude",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_DOUBLE,
    0,   /* quantifier_offset */
    offsetof(Google__Type__LatLng, longitude),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned google__type__lat_lng__field_indices_by_name[] = {
  0,   /* field[0] = latitude */
  1,   /* field[1] = longitude */
};
static const ProtobufCIntRange google__type__lat_lng__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor google__type__lat_lng__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "google.type.LatLng",
  "LatLng",
  "Google__Type__LatLng",
  "google.type",
  sizeof(Google__Type__LatLng),
  2,
  google__type__lat_lng__field_descriptors,
  google__type__lat_lng__field_indices_by_name,
  1,  google__type__lat_lng__number_ranges,
  (ProtobufCMessageInit) google__type__lat_lng__init,
  NULL,NULL,NULL    /* reserved[123] */
};