#ifndef __DATA_TYPE_H__
#define __DATA_TYPE_H__
// FIXME: rename 'Type'

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// types we care about
typedef enum {
    STRUCT,
    // TODO:
} ObjectType;

typedef enum {
    INT,
    FLOAT,
    DOUBLE,
    POINTER,
    ARRAY,
    STRUCT,
    UNKNOWN
    // TODO:
} Type;

typedef struct LLVMStruct;

typedef struct {
    Type type;
    size_t size;
    size_t alignment;

    // to handle Additional Parametric Types
    union {
        ElementType *pointeeType; // for POINTER type
        struct {
            ElementType *elementType;
            size_t elementNum;
        } arrayInfo; //for ARRAY type
        LLVMStruct *structType; // for nested STRUCT type
        // TODO: other additional parametric types
    } info;
} ElementType;

typedef struct {
    ElementType elementType;
    char *name;  // name of the element (if applicable)
    size_t offset; // offset of the element within the struct
} LLVMStructElement;

typedef struct {
    char *name;  // name of the struct
    size_t elementNum;  // number of elements in the struct
    LLVMStructElement *elements;  // array of elements
} LLVMStruct;

// we do not consider `LLVMArray` type in v1.0
typedef struct {
    char *name;
    size_t elementNum;
    ElementType *elementType;
} LLVMArray;

// TODO: registerStuct & registerArray
int addLLVMStruct(uint8_t typeID, LLVMStruct structLayout){}
int addLLVMArray(uint8_t typeID, LLVMArray arrayInfo){}



#ifdef __cplusplus
}
#endif
#endif