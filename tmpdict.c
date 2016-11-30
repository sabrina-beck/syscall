#include <linux/unistd.h>
#include <linux/linkage.h>
#include <linux/kernel.h> 
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>

#define HASH_SIZE 51
#define STRING_MAX_LEN 100

/********** Linked List **********/

typedef struct Node {
    int key;
    char* value;
    unsigned int lifespan;
    unsigned long createdAt;
    struct Node* next;
} Node;

typedef struct LinkedList {
    Node* head;
} LinkedList;

Node* newElement(int key, char* value, unsigned int lifespan) {
    Node* node = kmalloc(sizeof(Node), GFP_KERNEL);
    node->key = key;
    node->value = value;
    node->lifespan = lifespan;
    node->createdAt = jiffies;
    node->next = NULL;
    return node;
}

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

void add(LinkedList* list, Node* newNode) {
    Node* currentNode;

    if(list->head == NULL) {
        list->head = newNode;
        return;
    }

    currentNode = list->head;
    while(currentNode->next != NULL) {
        currentNode = currentNode->next;
    }

    currentNode->next = newNode;
}

bool remove(LinkedList* list, Node* node) {
    Node* currentNode = list->head;

    if(node == NULL || currentNode == NULL) {
        return true;
    }

    if(list->head == node) {
        list->head = list->head->next;
        kfree(node->value);
        kfree(node);
        return true;
    }

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
typedef struct HashTable {
    LinkedList table[HASH_SIZE];
} HashTable;

bool isExpired(Node* node) {
    return (node->createdAt / HZ) + node->lifespan < (jiffies / HZ);
}

//search for the Knuth's multiplicative method
unsigned int hashCode(int key) {
    if(key < 0) {
        key = -key;
    }

    return key % HASH_SIZE;
}

/********** Auxiliary **********/
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

/* Retorna 0 se a operação foi bem sucedida e -1 caso contrário. */
asmlinkage long sys_settmpkey(int key, char* value, unsigned int lifespan) {
    Node* newNode;
    Node* node;
    unsigned int code;
    LinkedList* hashCodeBucket;
    char* kernelValue;

    code = hashCode(key);
    hashCodeBucket = &(hashTable.table[code]);

    node = findByKey(hashCodeBucket, key);
    
    if(node != NULL && !isExpired(node)) {
        return -1;
    }

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

/* Copia em value o valor da chave key com no máximo n caracteres.
   Retorna 0 se encontrou chave válida e -1 caso contrário. */
asmlinkage long sys_gettmpkey(int key, int n, char* value) {
    Node* node;
    unsigned int code;
    LinkedList* hashCodeBucket;

    code = hashCode(key);
    hashCodeBucket = &(hashTable.table[code]);
    
    node = findByKey(hashCodeBucket, key);

    if(node == NULL) { // didn't found
        return 1;
    }

    if(isExpired(node)) {
        remove(hashCodeBucket, node);
        return 1;
    }

    return copy_to_user(value, node->value, n);
}