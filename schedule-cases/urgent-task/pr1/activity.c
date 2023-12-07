/*
 *                               POK header
 *
 * The following file is a part of the POK project. Any modification should
 * be made according to the POK licence. You CANNOT use this file or a part
 * of a file for your own project.
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2022 POK team
 */

#include "activity.h"
#include <stddef.h>
#include <stdio.h>

void *general_task() {
  printf("[GENERAL] I'm general task.\n");
  while (1) {
  }
}

void *urgent_task() {
  printf("[URGENT] I'm general task.\n");
  while (1) {
  }
}

void *trouble_task() {
  printf("[TROUBLE] I'm trouble task.\n");
  while (1) {
  }
}