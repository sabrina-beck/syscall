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
  char* value = malloc(MAX_SIZE * sizeof(char));
  int r, key, lifespan;

  printf("\nKey: ");
  scanf("%d", &key);
  printf("Value: ");
  scanf("%s", value);
  printf("Lifespan (seconds): ");
  scanf("%d", &lifespan);
  r = syscall(380, key, value, lifespan);
  if (r == 0)
    printf("Success\n");
  else
    printf("Error\n");
  free(value);
}

void getKey() {
  char* value = malloc(MAX_SIZE * sizeof(char));
  int r, key, size;
  printf("\nKey: ");
  scanf("%d", &key);
  printf("Size: ");
  scanf("%d", &size);
  r = syscall(381, key, size, value);
  if (r == 0)
    printf("Success. Value: %s\n", value);
  else
    printf("Error finding the key: %d\n", key);
  free(value);
}

int main() {
  int option;

  do {
    option = getOptionFromMenu();

    switch(option) {
      case 1:
        setNewKey();
        break;
      case 2:
        getKey();
        break;
    }
    printf("\n\n");
  } while (option != 0);

  return 0;
}
