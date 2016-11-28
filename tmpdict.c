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
    int size;
} LinkedList;

Node* newElement(int key, char* value, unsigned int lifespan) {
    Node* node = kmalloc(sizeof(Node), GFP_KERNEL);
    node->key = key;
    node->value = value;
    node->lifespan = lifespan;
    node->createdAt = jiffies;
    return node;
}

LinkedList* newLinkedList(void) {
    LinkedList* list = kmalloc(sizeof(LinkedList), GFP_KERNEL);
    list->head = NULL;
    list->size = 0;
    return list;
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
    list->size += 1;
}

bool remove(LinkedList* list, Node* node) {
    Node* currentNode = list->head;

    if(node == NULL || currentNode == NULL) {
        return true;
    }

    if(list->head == node) {
        list->head = list->head->next;
        kfree(node);
        return true;
    }

    while(currentNode != NULL) {
        if(currentNode->next == node) {
            currentNode->next = node->next;
            list->size -= 1;
            kfree(node);
            return true;
        }
        currentNode = currentNode->next;
    }

    return false;
}

/********** Hash Map **********/
typedef struct HashTable {
    LinkedList** table;
} HashTable;

HashTable* newHashTable(unsigned int size) {
    int i;
    HashTable* hashTable = kmalloc(sizeof(HashTable), GFP_KERNEL);

    LinkedList** table = kmalloc(size * sizeof(LinkedList*), GFP_KERNEL);
    
    for(i = 0; i < size; i++) { 
        table[i] = newLinkedList();
    }

    hashTable->table = table;
    return hashTable;
}

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
HashTable* hashTable;

/* Retorna 0 se a operação foi bem sucedida e -1 caso contrário. */
asmlinkage long sys_settmpkey(int key, char* value, unsigned int lifespan) {
    Node* newNode;
    Node* node;
    unsigned int code;
    LinkedList* hashCodeBucket;
    char* kernelValue;

    //FIXME find better way to initialize the hash table
    if(hashTable == NULL) {
        hashTable = newHashTable(HASH_SIZE);
    }

    code = hashCode(key);
    hashCodeBucket = hashTable->table[code];

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

    //FIXME find better way to initialize the hash table
    if(hashTable == NULL) {
        hashTable = newHashTable(HASH_SIZE);
    }

    code = hashCode(key);
    hashCodeBucket = hashTable->table[code];    
    
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