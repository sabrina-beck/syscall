/*
 * Teste da nova chamada de sistema
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const int MAX_SIZE = 100;

int getOptionFromMenu() {
  int option;
  printf("Syscall menu \n");
  printf("0. Exit\n");
  printf("1. Set  key \n");
  printf("2. Get key \n");
  printf("Type your option: ");
  scanf("%d", &option);
  return option;
}

void setNewKey() {
  char value[MAX_SIZE], jl;
  int key, lifespan;
  long r;

  printf("\nKey: ");
  scanf("%d", &key);
  scanf("%c", &jl);
  printf("Value: ");
  scanf("%[^\t\n]s", value);
  printf("Lifespan (seconds): ");
  scanf("%d", &lifespan);

  r = syscall(380, key, value, lifespan);
  
  if (r == 0) {
    printf("Success\n");
  } else {
    printf("Could not set the new key, the key may already exist or we have an error\n");
  }
}

void getKey() {
  char* value = malloc((MAX_SIZE + 1) * sizeof(char));
  int key, size;
  long r;
  printf("\nKey: ");
  scanf("%d", &key);
  printf("Size: ");
  scanf("%d", &size);
  r = syscall(381, key, size, value);
  value[size] = '\0';
  if (r == 0) {
    printf("Success. Value: %s\n", value);
  } else {
    printf("Could not find the key: %d\n", key);
    printf("The key may be expired or never existed.");
  }

  free(value);
}

int main() {
  int option;

  do {
    option = getOptionFromMenu();

    switch(option) {
      case 0: break;
      case 1:
        setNewKey();
        break;
      case 2:
        getKey();
        break;
      default:
        printf("Invalid option!\n");
    }
    printf("\n\n");
  } while (option != 0);

  return 0;
}
