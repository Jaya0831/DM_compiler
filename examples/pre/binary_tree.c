#include <stdio.h>
#include <stdlib.h>

// Definition of the binary tree node
typedef struct TreeNode {
    int key;
    struct TreeNode *left, *right;
} TreeNode;

// Function to create a new tree node
TreeNode* createNode(int key) {
    // rationale: don't care about the type
    TreeNode* newNodePtr = (TreeNode*)malloc(sizeof(TreeNode));
    newNodePtr->key = key;
    newNodePtr->left = newNodePtr->right = NULL;
    return newNodePtr;
}

// Function to insert a new key into the binary tree
TreeNode* insert(TreeNode* rootPtr, int key) {
    if (rootPtr == NULL) {
        return createNode(key);
    }
    int root_key = rootPtr->key;
    if (key < root_key) {
        rootPtr->left = insert(rootPtr->left, key);
    } else if (key > root_key) {
        rootPtr->right = insert(rootPtr->right, key);
    }
    return rootPtr;
}


// Function to search for a key in the binary tree
TreeNode* search(TreeNode* nodePtr, int key) {
    while (nodePtr != NULL) {
        int node_key = nodePtr->key;
        if (node_key == key) {
            return nodePtr;
        } else if (key < node_key) {
            nodePtr = nodePtr->left;
        } else {
            nodePtr = nodePtr->right;
        }
    }
    return NULL;
}

