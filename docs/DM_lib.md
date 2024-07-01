### Type
(static information)

// TODO: how to manage array/struct/... (data structure)
// TODO: auto load default types

```c
int registerType(){

}
```

### Address Dependence
(static information)

```c
typedef GlobalAddress (*Dependence) (GlobalAddress gaddr);
 
typedef struct AddrDependence {
    uint64_t type_id1;
    Dependence computation;
    uint64_t type_id2;
}AddrDependence;

int registerAddrDependence(uint64_t type_id1, Dependence computation, uint64_t type_id2){
    // add new AddrDependence
}
```
__We do not need to maintain the dependency information between objects during runtime in version1__
// TODO: 

### GlobalAddress
reference: dex/include/GlobalAddress.h

1. A 48-bit space to carry a local memory address of a particular server
2. `memory_server_id` bits to identify and differentiate memory nodes in Disaggregated Memory settings (Dex)
3. `Type_id` bits to indicate the `Type` of the object (similar to Mira's `cache_id` bits)

GlobalAddress:
- [memory_server_id:8, cache_id:8, address:48]

### Global Allocation
```c
GlobalAddress disaggAlloc(int type_id, int count){
    GlobalAddress gaddr = tryAlloc(type_id, count);
    if(gaddr){
        return gaddr;
    }
    else{
        requestChunk(thread_id);
        return tryAlloc(type_id, count);
    }
}

void disaggFree(GlobalAddress gaddr, uint64_t size){
    // Mira does not implement disaggFree in its code
    // To support `disaggFree`, we need to implement garbage collection
}
```

### Caching
// TODO: cache structure?
```c
// load->cache_request&cache_access
// store->cache_request&cache_access_mut
// TODO: check or guarantee the data is still in the cache when the actual local access occurs

cache_token cacheRequest(GlobalAddress gaddr){
    // refer to Mira/runtime/libcommon/lib/cache.c
    // gaddr: the global address the program request
    // return the corresponding local cache token
    // interact with the CacheManager to fetch and evict objects

    if(isLocal(gaddr)){
        return toCacheToken(gaddr);
    }
    
    cache_token=checkCache(gaddr);
    if(cache_token){
        // markNoEvict(cache_token);
        return cache_token
    }
    else{
        // markNoEvict(cache_token);
        return remoteFetch(gaddr);
    }
}

void* cacheAccess(cache_token token){
    // refer to Mira/runtime/libcommon/lib/cache.c
    // return the corresponding local address

    return toLocalAddr(token);
}

void* cacheAccessMut(cache_token token){
    // refer to Mira/runtime/libcommon/lib/cache.c
    // return the corresponding local address

    markMut(token);
    return toLocalAddr(token);
}
```

### Offloading
```c
typedef void (*rpc_service_t)(void *arg, void *ret);


cache_token cacheRequestOfld(bool *suc, uint64_t func_id, void *arg, void *ret){ // maybe some extra arguments that indicate the objects we need to protect
    if(isLocal(gaddr)){
        return toCacheToken(gaddr);
    }
    if(cache_token){
        // markNoEvict(cache_token);
        return cache_token
    }
    else{
        // try offload
        *suc = ofldFunc(func_id, arg, ret); // maybe some extra arguments that indicate the objects we need to protect
        // markNoEvict(cache_token);
        return remoteFetch(gaddr);
    }
}

bool ofldFunc(uint64_t func_id, void *arg, void *ret){
    // To guarantee the consistency, we need to set I/O flags & flush related data, and propagate data back
    // rpc call
}
```

// TODO: I/O flag for cache & ofld
// TODO: functions related to ofld
