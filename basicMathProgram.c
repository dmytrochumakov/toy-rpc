#include <stdbool.h>
#include <stdio.h>

#include "isPrimeRPCClient.h"

int main(void) {
  printf("Please enter a number: ");

  int inputNumber;
  scanf("%d", &inputNumber);

  if (isPrimeRPC(inputNumber)) {
    printf("%d is prime\n", inputNumber);
  } else {
    printf("%d is not prime\n", inputNumber);
  }

  return 0;
}
