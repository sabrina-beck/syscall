/* 
 * Teste da nova chamada de sistema
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  int size = 5;
  char* ptr2 = malloc(size * sizeof(char));
  
  char* ptr = "boo\n";
  int r = syscall(380, 1, ptr, 100000); 
  printf("Retorno da chamada de sistema: %d.\n", r);

  r = syscall(381, 1, 4, ptr2); 
  printf("Retorno da chamada de sistema: %d.\n", r);
  printf("Retorno da string: %s.\n", ptr2);

  free(ptr2);

  ptr = "bla\n";
  r = syscall(380, 2, ptr, 100000); 
  printf("Retorno da chamada de sistema: %d.\n", r);

  ptr2 = malloc(size * sizeof(char));
  r = syscall(381, 2, 4, ptr2); 
  printf("Retorno da chamada de sistema: %d.\n", r);
  printf("Retorno da string: %s.\n", ptr2);

  free(ptr2);
  return r;
}