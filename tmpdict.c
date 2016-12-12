#include <linux/unistd.h>
#include <linux/linkage.h>
#include <linux/kernel.h> 
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>

#define HASH_SIZE 51
#define STRING_MAX_LEN 100

/********** Linked List **********/

/*
 * Represents a linked list node
 */
typedef struct Node {
    int key;
    char* value;
    unsigned int lifespan;
    unsigned long createdAt;
    struct Node* next;
} Node;

/*
 * Represents a linked list
 */
typedef struct LinkedList {
    Node* head;
} LinkedList;

/*
 * Creates a new linked list node
 */
Node* newElement(int key, char* value, unsigned int lifespan) {
    Node* node = kmalloc(sizeof(Node), GFP_KERNEL);
    node->key = key;
    node->value = value;
    node->lifespan = lifespan;
    node->createdAt = jiffies;
    node->next = NULL;
    return node;
}

/*
 * Returns a node that belongs to the linked list and it is
 * identified by the desired key
 */
Node* findByKey(LinkedList* list, int key) {
    Node* currentNode = list->head;
    while(currentNode != NULL) {
        if(currentNode->key == key) {
            return currentNode;
        }
        currentNode = currentNode->next;
    }
    return NULL;
}

/*
 * Adds a new node to the linked list
 */
void add(LinkedList* list, Node* newNode) {
    Node* currentNode;

    // add to the head
    if(list->head == NULL) {
        list->head = newNode;
        return;
    }

    // add in the middle
    currentNode = list->head;
    while(currentNode->next != NULL) {
        currentNode = currentNode->next;
    }

    currentNode->next = newNode;
}

/*
 * Removes a node from the linked list
 */
bool remove(LinkedList* list, Node* node) {
    Node* currentNode = list->head;

    if(node == NULL || currentNode == NULL) {
        return true;
    }

    // removes from the head
    if(list->head == node) {
        list->head = list->head->next;
        kfree(node->value);
        kfree(node);
        return true;
    }

    // removes from the middle
    while(currentNode != NULL) {
        if(currentNode->next == node) {
            currentNode->next = node->next;
            kfree(node->value);
            kfree(node);
            return true;
        }
        currentNode = currentNode->next;
    }

    return false;
}

/********** Hash Map **********/
/*
 * Represents a hash table with conflits solved by linked lists
 */
typedef struct HashTable {
    LinkedList table[HASH_SIZE];
} HashTable;

/*
 * Checks if a node is expired
 */
bool isExpired(Node* node) {
    return (node->createdAt / HZ) + node->lifespan < (jiffies / HZ);
}

/*
 * Hash code used as index of the hash table
 * works for integer keys, negative and positive
 */
unsigned int hashCode(int key) {
    if(key < 0) {
        key = -key;
    }

    return key % HASH_SIZE;
}

/********** Auxiliary **********/
/*
 * Copies a string from the user memory to the kernel memory
 */
char* toKernel(char* value) {
    long size = strnlen_user(value, STRING_MAX_LEN);

    char* kernelValue = kmalloc(size * sizeof(char), GFP_KERNEL);
    
    int copyResult = strncpy_from_user(kernelValue, value, size);
    if(copyResult == -EFAULT) {
        return NULL;
    }

    return kernelValue;
}

/********** System calls **********/
HashTable hashTable;

/*
 * Sets a new temporary key in the hash table
 * Returns 0 if the operation is successfull
 * Returns -1 if not.
 */
asmlinkage long sys_settmpkey(int key, char* value, unsigned int lifespan) {
    Node* newNode;
    Node* node;
    unsigned int code;
    LinkedList* hashCodeBucket;
    char* kernelValue;

    code = hashCode(key);
    hashCodeBucket = &(hashTable.table[code]);

    node = findByKey(hashCodeBucket, key);
    
    // if the key already exist and is not expired, we don't allow to overwrite it
    if(node != NULL && !isExpired(node)) {
        return -1;
    }

    // if the key already exist an it is expired, it should be removed and the new
    // one should be created
    if(node != NULL) {
        remove(hashCodeBucket, node);
    }

    kernelValue = toKernel(value);
    if(kernelValue == NULL) {
        return -1;
    }

    newNode = newElement(key, kernelValue, lifespan);
    add(hashCodeBucket, newNode);

    return 0;
}

/*
 * Copies into value the key value in the hash table with a maximum of n characters
 * Returns 0 if the key is found and is valid and -1 if it isnt
 */
asmlinkage long sys_gettmpkey(int key, int n, char* value) {
    Node* node;
    unsigned int code;
    LinkedList* hashCodeBucket;

    code = hashCode(key);
    hashCodeBucket = &(hashTable.table[code]);
    
    node = findByKey(hashCodeBucket, key);

    if(node == NULL) {
        return -1;
    }

    if(isExpired(node)) {
        remove(hashCodeBucket, node);
        return -1;
    }

    return copy_to_user(value, node->value, n);
}