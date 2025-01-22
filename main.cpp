#include "MemoryTracker.hpp"

struct TreeNode {
    int value;
    TreeNode* left;
    TreeNode* right;
};

TreeNode* createNode(int value) {
    TreeNode* newNode = (TreeNode*)NEW(sizeof(TreeNode), "TreeNode");
    newNode->value = value;
    newNode->left = nullptr;
    newNode->right = nullptr;
    return newNode;
}

TreeNode* buildTree() {
    TreeNode* root = createNode(1);
    root->left = createNode(2);
    root->right = createNode(3);
    root->left->left = createNode(4); 
    root->left->right = createNode(5);
    root->right->left = createNode(6); 
    root->right->right = createNode(7);
    return root;
}

// User-defined struct
struct UserDefinedType {
    int id;
    float value;
    char name[50];
};

int main() {
    TreeNode* root = buildTree();
     int* intArray = (int*)NEW(6 * sizeof(int), "IntArray");
     int* ptr = (int*)malloc(10 * sizeof(int));
     float* floatArray = (float*)NEW(4 * sizeof(float), "FloatArray");
    UserDefinedType* userObject = (UserDefinedType*)NEW(sizeof(UserDefinedType), "UserDefinedType");

    userObject->id = 1;
    userObject->value = 42.42f;
    strcpy(userObject->name, "Example Object");

    MemoryTracker::printMemoryUsage();

    DELETE(root->left->right);
    DELETE(root->right->right);

    MemoryTracker::reportGroupedLeaks();
    MemoryTracker::freeLeakedMemory();
    MemoryTracker::printMemoryUsage();

    return 0;
}
