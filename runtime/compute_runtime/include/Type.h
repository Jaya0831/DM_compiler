#ifndef __DATA_TYPE_H__
#define __DATA_TYPE_H__
// FIXME: rename 'Type'

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum Type {
    StructType,
    // TODO: other types
};


int registerType(Type type){}
int registerStruct(){}



#ifdef __cplusplus
}
#endif
#endif