#include <stdio.h>
#include <stdlib.h>



typedef struct {

}CacheToken;

typedef struct {
    
}GlobalAddress;

// Definition of the binary tree node
typedef struct TreeNode {
    int key;
    struct TreeNode *left, *right;
} TreeNode;

// Function to create a new tree node
TreeNode* createNode(int key) {
    GlobalAddress newNode = disaggAlloc(sizeof(TreeNode), 1);
    CacheToken checkRequest() 
    newNode->key = key;
    newNode->left = newNode->right = NULL;
    return newNode;
}

// Function to insert a new key into the binary tree
TreeNode* insert(TreeNode* root, int key) {
    if (root == NULL) {
        return createNode(key);
    }
    if (key < root->key) {
        root->left = insert(root->left, key);
    } else if (key > root->key) {
        root->right = insert(root->right, key);
    }
    return root;
}

// Function to search for a key in the binary tree
TreeNode* search(TreeNode* node, int key) {
    while (node != NULL) {
        if (node->key == key) {
            return node;
        } else if (key < node->key) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return NULL;
}