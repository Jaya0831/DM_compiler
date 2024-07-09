#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "global_addr.h"
#include "global_alloc.h"


// registerStruct
#define TreeNodeStructID 3

// Definition of the binary tree node
typedef struct TreeNode {
    int key;
    // original: struct TreeNode *left, *right;
    global_addr_t left, right;
} TreeNode;


// // Function to create a new tree node
// TreeNode* createNode(int key) {
//     TreeNode* newNodePtr = (TreeNode*)malloc(sizeof(TreeNode));
//     newNodePtr->key = key;
//     newNodePtr->left = newNodePtr->right = NULL;
//     return newNodePtr;
// }

// DM-version
// original: TreeNode* createNode(int key) {
global_addr_t createNode(int key) {
    // original: TreeNode* newNodePtr = (TreeNode*)malloc(sizeof(TreeNode));
    global_addr_t newNodeGAddr = disagg_alloc(TreeNodeStructID, sizeof(TreeNode), 1);

    // original: newNodePtr->key = key;
    // For the real implementation, we send (newNodeGAddr + offset(key)) to cacheRequest directly
    cache_token token = cache_request(newNodeGAddr);
    TreeNode* newNodeLPtr = (TreeNode*)cache_access_mut(token);
    newNodeLPtr->key = key;

    // Without pointer lowering, we need to check cache for every access
    newNodeLPtr->left.val = newNodeLPtr->right.val = NULL;

    // original
    return newNodeGAddr;
}


// // Function to insert a new key into the binary tree
// TreeNode* insert(TreeNode* rootPtr, int key) {
//     if (rootPtr == NULL) {
//         return createNode(key);
//     }
//     if (key < rootPtr->key) {
//         rootPtr->left = insert(rootPtr->left, key);
//     } else if (key > rootPtr->key) {
//         rootPtr->right = insert(rootPtr->right, key);
//     }
//     return rootPtr;
// }

// original: TreeNode* insert(TreeNode* rootPtr, int key) {
global_addr_t insert(global_addr_t rootGAddr, int key) {
    // original: if (rootPtr == NULL) {
    if (rootGAddr.val == NULL) {
        return createNode(key);
    }

    // original: if (key < rootPtr->key) {
    cache_token token = cache_request(rootGAddr);
    TreeNode* rootLPtr = (TreeNode*)cache_access(token);
    if (key < rootLPtr->key) {

        // original: rootPtr->left = insert(rootPtr->left, key);
        // Without pointer lowering, we need to check cache for every access
        TreeNode* rootLPtr = (TreeNode*)cache_access_mut(token);
        rootLPtr->left = insert(rootLPtr->left, key);

    } else if (key > rootLPtr->key) { // Without pointer lowering, we need to check cache for every access
        
        // original: rootPtr->right = insert(rootPtr->right, key);
        TreeNode* rootLPtr = (TreeNode*)cache_access_mut(token);
        rootLPtr->right = insert(rootLPtr->right, key);
    }
    return rootGAddr;
}

// // Function to search for a key in the binary tree
// TreeNode* search(TreeNode* nodePtr, int key) {
//     while (nodePtr != NULL) {
//         if (nodePtr->key == key) {
//             return nodePtr;
//         } else if (key < nodePtr->key) {
//             nodePtr = nodePtr->left;
//         } else {
//             nodePtr = nodePtr->right;
//         }
//     }
//     return NULL;
// }

// original: TreeNode* search(TreeNode* nodePtr, int key) {
global_addr_t search(global_addr_t nodeGAddr, int key) {
    // original: while (nodePtr != NULL) {
    while (nodeGAddr.val != NULL) {

        // original: if (nodePtr->key == key) {
        cache_token token = cache_request(nodeGAddr);
        TreeNode* nodeLPtr = (TreeNode*)cache_access(token);
        if (nodeLPtr->key == key) {

            return nodeGAddr;

        // Without pointer lowering, we need to check cache for every access
        } else if (key < nodeLPtr->key) {

            // original: nodePtr = nodePtr->right;
            TreeNode* nodeLPtr = (TreeNode*)cache_access_mut(token);
            nodeGAddr = nodeLPtr->left;

        } else {

            // Without pointer lowering, we need to check cache for every access
            nodeGAddr = nodeLPtr->right;
        }
    }
    // original return NULL;
    global_addr_t null_tmp={.val = NULL};
    return null_tmp;
}