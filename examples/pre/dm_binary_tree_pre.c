#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>
#include <stddef.h>
#include "dm_lib.h"

// Definition of the binary tree node
typedef struct TreeNode {
    int key;
    dm_global_addr_t left, right;
} TreeNode;


// Struct Information
uint8_t dm_TreeNode_type_id;
dm_struct_repr dm_TreeNode_layout = {
    .name = "TreeNode",
    .elems = (struct struct_elem[]) {
        {
            .name = "key",
            .type = {
                .kind = TYPE_INT,
                .size = sizeof(int),
                .alignment = alignof(int)
            },
            .offset = offsetof(TreeNode, key)
        },
        {
            .name = "left",
            .type = {
                .kind = TYPE_POINTER,
                .size = 8,
                .alignment = 8
            },
            .offset = offsetof(TreeNode, left)
        },
        {
            .name = "right",
            .type = {
                .kind = TYPE_POINTER,
                .size = 8,
                .alignment = 8
            },
            .offset = offsetof(TreeNode, right)
        }
    },
    .elem_num = 3
};

// Address Dependency Information
dm_global_addr_t dm_TreeNode_left_func(void* lptr) {
    return ((TreeNode*)lptr)->left;
}
dm_global_addr_t dm_TreeNode_right_func(void* lptr) {
    return ((TreeNode*)lptr)->right;
}

int main(){
    // register struct TreeNode
    dm_TreeNode_type_id = dm_register_struct(dm_TreeNode_layout);

    // register `TreeNode_left` Address Dependency
    dm_register_addr_dep((dm_addr_dep){
        .type1 = dm_TreeNode_type_id,
        .type2 = dm_TreeNode_type_id,
        .dep = dm_TreeNode_left_func
    });

    // register `TreeNode_right` Address Dependency
    dm_register_addr_dep((dm_addr_dep){
        .type1 = dm_TreeNode_type_id,
        .type2 = dm_TreeNode_type_id,
        .dep = dm_TreeNode_right_func
    });
}

dm_global_addr_t createNode(int key) {
    // rationale: we care about the `type` of the object
    // pseudo code: dm_malloc(TreeNode)
    dm_global_addr_t newNodeGAddr = dm_malloc(dm_TreeNode_type_id);

    // For the real implementation, we send (newNodeGAddr + offset(key)) to cacheRequest directly
    dm_cache_token dm_token = dm_cache_request(newNodeGAddr);
    TreeNode* dm_lptr = (TreeNode*)dm_cache_access_mut(dm_token);
    dm_lptr->key = key;

    TreeNode* dm_lptr = (TreeNode*)dm_cache_access_mut(dm_token);
    dm_lptr->left.val = dm_lptr->right.val = NULL;

    return newNodeGAddr;
}

dm_global_addr_t insert(dm_global_addr_t rootGAddr, int key) {
    if (rootGAddr.val == NULL) {
        return createNode(key);
    }

    dm_cache_token dm_token = dm_cache_request(rootGAddr);
    TreeNode* dm_lptr = (TreeNode*)dm_cache_access(dm_token);
    int root_key = dm_lptr->key;

    if (key < root_key) {

        TreeNode* dm_lptr = (TreeNode*)dm_cache_access_mut(dm_token);
        dm_lptr->left = insert(dm_lptr->left, key);
        // dm_cache_store_notify(dm_token);

    } else if (key > root_key) {

        TreeNode* dm_lptr = (TreeNode*)dm_cache_access_mut(dm_token);
        dm_lptr->right = insert(dm_lptr->right, key);

    }
    return rootGAddr;
}


dm_global_addr_t search(dm_global_addr_t nodeGAddr, int key) {
    while (nodeGAddr.val != NULL) {

        dm_cache_token dm_token = dm_cache_request(nodeGAddr);
        TreeNode* dm_lptr = (TreeNode*)dm_cache_access(dm_token);
        int node_key = dm_lptr->key;

        if (node_key == key) {
            return nodeGAddr;
        } else if (key < node_key) { 

            TreeNode* dm_lptr = (TreeNode*)dm_cache_access(dm_token);
            nodeGAddr = dm_lptr->left;

        } else {

            TreeNode* dm_lptr = (TreeNode*)dm_cache_access(dm_token);
            nodeGAddr = dm_lptr->right;

        }
    }

    return (dm_global_addr_t){.val = NULL};
}


// dm_global_addr_t dm_TreeNode_left_func(dm_global_addr_t gaddr) {
//     dm_global_addr_t left_gaddr = {.val=gaddr.val+offsetof(TreeNode, left)};
//     dm_cache_token dm_token = dm_cache_request(left_gaddr);
//     dm_global_addr_t* lptr = dm_cache_access(dm_token);
//     dm_global_addr_t child_gaddr = *lptr;
//     return child_gaddr;
// }

// dm_global_addr_t dm_TreeNode_right_func(dm_global_addr_t gaddr) {
//     dm_global_addr_t right_gaddr = {.val=gaddr.val+offsetof(TreeNode, right)};
//     dm_cache_token dm_token = dm_cache_request(right_gaddr);
//     dm_global_addr_t* lptr = dm_cache_access(dm_token);
//     dm_global_addr_t child_gaddr = *lptr;
//     return child_gaddr;
// }