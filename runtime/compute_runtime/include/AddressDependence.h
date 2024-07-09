#ifndef __ADDRESSDEPENDENCE_H_
#define __ADDRESSDEPENDENCE_H_

#include <stdint.h>
#include <stdlib.h>
#include "GlobalAddress.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef GlobalAddress (*DataDependence) (GlobalAddress gaddr);

// in version1.0, we only consider 1 to 1 Address Dependence
// This `AddrDependence` is static information
typedef struct AddrDependence {
    uint64_t typeID1;
    DataDependence dataDependence;
    uint64_t TypeID2;
}AddrDependence;

int registerAddrDependence(uint64_t typeID1, DataDependence dependence, uint64_t typeID2){
    // add new AddrDependence
}

#ifdef __cplusplus
}
#endif

#endif