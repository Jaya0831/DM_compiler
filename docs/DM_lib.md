### Type
(static information)
__We only consider `struct` in version1.0__
```c
// Type of the runtime objects
typedef enum {
    STRUCT
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

// TODO: registerStruct & registerArray
// called by registerStruct & registerArray
int addLLVMStruct(uint8_t typeID, LLVMStruct structLayout){}
// we do not consider `array` type in v1.0
int addLLVMArray(uint8_t typeID, LLVMArray arrayInfo){}
```

### Address Dependence
(static information)

```c
typedef GlobalAddress (*DataDependence) (GlobalAddress gaddr);

// in version1.0, we only consider 1 to 1 Address Dependence
typedef struct AddrDependence {
    uint64_t typeID1;
    DataDependence dataDependence;
    uint64_t TypeID2;
}AddrDependence;

int registerAddrDependence(uint64_t typeID1, DataDependence dependence, uint64_t typeID2){
    // add new AddrDependence
}
```
__We do not need to maintain the dynamic dependency information between objects during runtime in version1.0__

### GlobalAddress
reference: dex/include/GlobalAddress.h

1. A 56-bit space to carry a local memory address of a particular server
2. `Type_id` bits to indicate the `Type` of the object (similar to Mira's `cache_id` bits)

GlobalAddress:
- [Type_id:8, address:56]

### Global Allocation
```c
GlobalAddress disaggAlloc(uint8_t typeID, size_t size, int count){
    GlobalAddress gaddr = tryAlloc(typeID, count);
    if(gaddr){
        return gaddr;
    }
    else{
        requestChunk();
        return tryAlloc(typeID, count);
    }
}

void disaggFree(GlobalAddress gaddr, size_t size){
    // no need in version1.0
}
```

### Caching
```c
// load->cache_request&cache_access
// store->cache_request&cache_access_mut
// TODO: check or guarantee the data is still in the cache when the actual local access occurs

CacheToken cacheRequest(GlobalAddress gaddr){
    // refer to Mira/runtime/libcommon/lib/cache.c
    // gaddr: the global address the program request
    // return the corresponding local cache token
    // interact with the CacheManager to fetch and evict objects

    if(isLocal(gaddr)){
        return toCacheToken(gaddr);
    }
    
    CacheToken=checkCache(gaddr);
    if(CacheToken){
        // markNoEvict(CacheToken);
        return CacheToken
    }
    else{
        // markNoEvict(CacheToken);
        return remoteFetch(gaddr);
    }
}

void* cacheAccess(CacheToken token){
    // refer to Mira/runtime/libcommon/lib/cache.c
    // return the corresponding local address

    return toLocalAddr(token);
}

void* cacheAccessMut(CacheToken token){
    // refer to Mira/runtime/libcommon/lib/cache.c
    // return the corresponding local address

    markMut(token);
    return toLocalAddr(token);
}
```

### Offloading
```c
typedef void (*RPCService)(void *arg, void *ret);


CacheToken cacheRequestOfld(bool *suc, uint64_t funcID, void *arg, void *ret){ // maybe some extra arguments that indicate the objects we need to protect
    if(isLocal(gaddr)){
        return toCacheToken(gaddr);
    }
    if(CacheToken){
        // markNoEvict(CacheToken);
        return CacheToken
    }
    else{
        // try offload
        *suc = ofldFunc(funcID, arg, ret); // maybe some extra arguments that indicate the objects we need to protect
        // markNoEvict(CacheToken);
        return remoteFetch(gaddr);
    }
}

bool ofldFunc(uint64_t funcID, void *arg, void *ret){
    // To guarantee the consistency, we need to set I/O flags & flush related data, and propagate data back
    // rpc call
}
```

// TODO: I/O flag for cache & ofld
// TODO: functions related to ofld
