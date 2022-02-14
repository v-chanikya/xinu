#include <xinu.h>
#include <future.h>
#include <stddef.h>
#include <future_prodcons.h>
#include <stdlib.h>
#include <run_cmd.h>

sid32 print_sem;

void future_prodcons(int nargs, char *args[]) {

  print_sem = semcreate(1);
  future_t* f_exclusive;
  f_exclusive = future_alloc(FUTURE_EXCLUSIVE, sizeof(int), 1);
  char *val;

  // First, try to iterate through the arguments and make sure they are all valid based on the requirements
  // (you may assume the argument after "s" there is always a number)
  if (nargs <= 2)
      goto fail;
  int i = 2;
  while (i < nargs) {
    if (! ((strncmp(args[i], "g", 1) == 0) 
            || (strncmp(args[i], "s", 1) == 0)
            || (atoi(args[i]) > 0)
            || (atoi(args[i]) == 0 && strncmp(args[i], "0", 1) == 0))){
        goto fail;
    }
    i++;
  }

  int num_args = i;  // keeping number of args to create the array
  i = 2; // reseting the index
  val  =  (char *) getmem(num_args); // initializing the array to keep the "s" numbers

  // Iterate again through the arguments and create the following processes based on the passed argument ("g" or "s VALUE")
  while (i < nargs) {
    if (strcmp(args[i], "g") == 0) {
      char id[10];
      sprintf(id, "fcons%d",i);
      resume(create(future_cons, 2048, 20, id, 1, f_exclusive));
    }
    if (strcmp(args[i], "s") == 0) {
      i++;
      uint8 number = atoi(args[i]);
      val[i] = number;
      resume(create(future_prod, 2048, 20, "fprod1", 2, f_exclusive, &val[i]));
      sleepms(5);
    }
    i++;
  }
  sleepms(100);
cleanup:
  if (val)
      freemem(val, num_args);
  future_free(f_exclusive);
  signal(run_complete);
  return;

fail:
  printf("Syntax: run futest [-pc [g ...] [s VALUE ...]|-f]\n");
  goto cleanup;
}

uint future_prod(future_t *fut, char *value) {
  int status;
  wait(print_sem);
  printf("Producing %d\n", *value);
  signal(print_sem);
  status = (int) future_set(fut, value);
  if (status < 1) {
    wait(print_sem);
    printf("future_set failed\n");
    signal(print_sem);
    return -1;
  }
  return OK;
}

uint future_cons(future_t *fut) {
  char *i = NULL;
  int status;
  status = (int) future_get(fut, i);
  if (status < 1) {
    wait(print_sem);
    printf("future_get failed\n");
    signal(print_sem);
    return -1;
  }
  wait(print_sem);
  printf("Consumed %d\n", *i);
  signal(print_sem);
  return OK;
}
