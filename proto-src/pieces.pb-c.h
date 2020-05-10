/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: pieces.proto */

#ifndef PROTOBUF_C_pieces_2eproto__INCLUDED
#define PROTOBUF_C_pieces_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003003 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _PiecesFlagDataItem PiecesFlagDataItem;
typedef struct _PiecesExcelDataItem PiecesExcelDataItem;
typedef struct _PieceExcelData PieceExcelData;
typedef struct _PieceExcelData__ItemEntry PieceExcelData__ItemEntry;
typedef struct _PiecesFlagData PiecesFlagData;
typedef struct _PiecesFlagData__ItemEntry PiecesFlagData__ItemEntry;
typedef struct _PiecesLink PiecesLink;


/* --- enums --- */

typedef enum _PiecesType {
  PIECES_TYPE__UNKNOWN = 0,
  PIECES_TYPE__USER = 1
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(PIECES_TYPE)
} PiecesType;

/* --- messages --- */

struct  _PiecesFlagDataItem
{
  ProtobufCMessage base;
  int64_t number;
  char *str;
};
#define PIECES_FLAG_DATA_ITEM__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&pieces_flag_data_item__descriptor) \
    , 0, (char *)protobuf_c_empty_string }


struct  _PiecesExcelDataItem
{
  ProtobufCMessage base;
  /*
   * number data
   */
  int64_t number;
  /*
   * string data
   */
  char *str;
  /*
   * number list data
   */
  size_t n_number_list;
  int64_t *number_list;
  /*
   * string list data
   */
  size_t n_str_list;
  char **str_list;
};
#define PIECES_EXCEL_DATA_ITEM__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&pieces_excel_data_item__descriptor) \
    , 0, (char *)protobuf_c_empty_string, 0,NULL, 0,NULL }


struct  _PieceExcelData__ItemEntry
{
  ProtobufCMessage base;
  char *key;
  PiecesExcelDataItem *value;
};
#define PIECE_EXCEL_DATA__ITEM_ENTRY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&piece_excel_data__item_entry__descriptor) \
    , (char *)protobuf_c_empty_string, NULL }


struct  _PieceExcelData
{
  ProtobufCMessage base;
  /*
   * excel data item list
   */
  size_t n_item;
  PieceExcelData__ItemEntry **item;
};
#define PIECE_EXCEL_DATA__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&piece_excel_data__descriptor) \
    , 0,NULL }


struct  _PiecesFlagData__ItemEntry
{
  ProtobufCMessage base;
  int32_t key;
  PiecesFlagDataItem *value;
};
#define PIECES_FLAG_DATA__ITEM_ENTRY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&pieces_flag_data__item_entry__descriptor) \
    , 0, NULL }


struct  _PiecesFlagData
{
  ProtobufCMessage base;
  size_t n_item;
  PiecesFlagData__ItemEntry **item;
};
#define PIECES_FLAG_DATA__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&pieces_flag_data__descriptor) \
    , 0,NULL }


struct  _PiecesLink
{
  ProtobufCMessage base;
  /*
   * data name
   */
  char *data_type;
  /*
   * c pointer
   */
  uint64_t next;
  /*
   * data
   */
  size_t n_data;
  PiecesFlagData **data;
};
#define PIECES_LINK__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&pieces_link__descriptor) \
    , (char *)protobuf_c_empty_string, 0, 0,NULL }


/* PiecesFlagDataItem methods */
void   pieces_flag_data_item__init
                     (PiecesFlagDataItem         *message);
size_t pieces_flag_data_item__get_packed_size
                     (const PiecesFlagDataItem   *message);
size_t pieces_flag_data_item__pack
                     (const PiecesFlagDataItem   *message,
                      uint8_t             *out);
size_t pieces_flag_data_item__pack_to_buffer
                     (const PiecesFlagDataItem   *message,
                      ProtobufCBuffer     *buffer);
PiecesFlagDataItem *
       pieces_flag_data_item__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   pieces_flag_data_item__free_unpacked
                     (PiecesFlagDataItem *message,
                      ProtobufCAllocator *allocator);
/* PiecesExcelDataItem methods */
void   pieces_excel_data_item__init
                     (PiecesExcelDataItem         *message);
size_t pieces_excel_data_item__get_packed_size
                     (const PiecesExcelDataItem   *message);
size_t pieces_excel_data_item__pack
                     (const PiecesExcelDataItem   *message,
                      uint8_t             *out);
size_t pieces_excel_data_item__pack_to_buffer
                     (const PiecesExcelDataItem   *message,
                      ProtobufCBuffer     *buffer);
PiecesExcelDataItem *
       pieces_excel_data_item__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   pieces_excel_data_item__free_unpacked
                     (PiecesExcelDataItem *message,
                      ProtobufCAllocator *allocator);
/* PieceExcelData__ItemEntry methods */
void   piece_excel_data__item_entry__init
                     (PieceExcelData__ItemEntry         *message);
/* PieceExcelData methods */
void   piece_excel_data__init
                     (PieceExcelData         *message);
size_t piece_excel_data__get_packed_size
                     (const PieceExcelData   *message);
size_t piece_excel_data__pack
                     (const PieceExcelData   *message,
                      uint8_t             *out);
size_t piece_excel_data__pack_to_buffer
                     (const PieceExcelData   *message,
                      ProtobufCBuffer     *buffer);
PieceExcelData *
       piece_excel_data__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   piece_excel_data__free_unpacked
                     (PieceExcelData *message,
                      ProtobufCAllocator *allocator);
/* PiecesFlagData__ItemEntry methods */
void   pieces_flag_data__item_entry__init
                     (PiecesFlagData__ItemEntry         *message);
/* PiecesFlagData methods */
void   pieces_flag_data__init
                     (PiecesFlagData         *message);
size_t pieces_flag_data__get_packed_size
                     (const PiecesFlagData   *message);
size_t pieces_flag_data__pack
                     (const PiecesFlagData   *message,
                      uint8_t             *out);
size_t pieces_flag_data__pack_to_buffer
                     (const PiecesFlagData   *message,
                      ProtobufCBuffer     *buffer);
PiecesFlagData *
       pieces_flag_data__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   pieces_flag_data__free_unpacked
                     (PiecesFlagData *message,
                      ProtobufCAllocator *allocator);
/* PiecesLink methods */
void   pieces_link__init
                     (PiecesLink         *message);
size_t pieces_link__get_packed_size
                     (const PiecesLink   *message);
size_t pieces_link__pack
                     (const PiecesLink   *message,
                      uint8_t             *out);
size_t pieces_link__pack_to_buffer
                     (const PiecesLink   *message,
                      ProtobufCBuffer     *buffer);
PiecesLink *
       pieces_link__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   pieces_link__free_unpacked
                     (PiecesLink *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*PiecesFlagDataItem_Closure)
                 (const PiecesFlagDataItem *message,
                  void *closure_data);
typedef void (*PiecesExcelDataItem_Closure)
                 (const PiecesExcelDataItem *message,
                  void *closure_data);
typedef void (*PieceExcelData__ItemEntry_Closure)
                 (const PieceExcelData__ItemEntry *message,
                  void *closure_data);
typedef void (*PieceExcelData_Closure)
                 (const PieceExcelData *message,
                  void *closure_data);
typedef void (*PiecesFlagData__ItemEntry_Closure)
                 (const PiecesFlagData__ItemEntry *message,
                  void *closure_data);
typedef void (*PiecesFlagData_Closure)
                 (const PiecesFlagData *message,
                  void *closure_data);
typedef void (*PiecesLink_Closure)
                 (const PiecesLink *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCEnumDescriptor    pieces_type__descriptor;
extern const ProtobufCMessageDescriptor pieces_flag_data_item__descriptor;
extern const ProtobufCMessageDescriptor pieces_excel_data_item__descriptor;
extern const ProtobufCMessageDescriptor piece_excel_data__descriptor;
extern const ProtobufCMessageDescriptor piece_excel_data__item_entry__descriptor;
extern const ProtobufCMessageDescriptor pieces_flag_data__descriptor;
extern const ProtobufCMessageDescriptor pieces_flag_data__item_entry__descriptor;
extern const ProtobufCMessageDescriptor pieces_link__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_pieces_2eproto__INCLUDED */
