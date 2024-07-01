#include <stdio.h>
#include <stdlib.h>

// Definition of the binary tree node
typedef struct TreeNode {
    int key;
    struct TreeNode *left, *right;
} TreeNode;

// Function to create a new tree node
TreeNode* createNode(int key) {
    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
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


// TreeNode* insert(TreeNode* node0, int key) {
//     if (node0 == NULL) {
//         return createNode(key);
//     }
//     if (key < node0->key) {
//         node0->left = insert(node0->left, key);
//     } else if (key > node0->key) {
//         node0->right = insert(node0->right, key);
//     }
//     return node0;
// }

// Function to search for a key in the binary tree
// TreeNode* search(TreeNode* root, int key) {
//     if (root == NULL || root->key == key) {
//         return root;
//     }
//     if (key < root->key) {
//         return search(root->left, key);
//     }
//     return search(root->right, key);
// }

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

    // TreeNode *node0, *node1;
    // if (node0->key == key) {
    //     return node0;
    // } else if (key < node0->key) {
    //     node1 = node0->left;
    // } else {
    //     node1 = node0->right;
    // }
    
    // if (node1->key == key) {
    //     return node1;
    // }
    // // ... ...

    return NULL;
}

// Function to find the minimum value node in the tree
TreeNode* findMin(TreeNode* node) {
    TreeNode* current = node;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}

// Function to delete a key from the binary tree
TreeNode* deleteNode(TreeNode* root, int key) {
    if (root == NULL) return root;

    if (key < root->key) {
        root->left = deleteNode(root->left, key);
    } else if (key > root->key) {
        root->right = deleteNode(root->right, key);
    } else {
        if (root->left == NULL) {
            TreeNode* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            TreeNode* temp = root->left;
            free(root);
            return temp;
        }

        TreeNode* temp = findMin(root->right);
        root->key = temp->key;
        root->right = deleteNode(root->right, temp->key);
    }
    return root;
}

// Function to perform in-order traversal of the binary tree
void inorderTraversal(TreeNode* root) {
    if (root != NULL) {
        inorderTraversal(root->left);
        printf("%d ", root->key);
        inorderTraversal(root->right);
    }
}

// Main function to demonstrate the binary tree operations
int main() {
    TreeNode* root = NULL;
    root = insert(root, 50);
    insert(root, 30);
    insert(root, 20);
    insert(root, 40);
    insert(root, 70);
    insert(root, 60);
    insert(root, 80);

    printf("In-order traversal: ");
    inorderTraversal(root);
    printf("\n");

    printf("Deleting 20\n");
    root = deleteNode(root, 20);
    printf("In-order traversal: ");
    inorderTraversal(root);
    printf("\n");

    printf("Deleting 30\n");
    root = deleteNode(root, 30);
    printf("In-order traversal: ");
    inorderTraversal(root);
    printf("\n");

    printf("Deleting 50\n");
    root = deleteNode(root, 50);
    printf("In-order traversal: ");
    inorderTraversal(root);
    printf("\n");

    TreeNode* searchedNode = search(root, 60);
    if (searchedNode != NULL) {
        printf("Node with key 60 found.\n");
    } else {
        printf("Node with key 60 not found.\n");
    }

    return 0;
}