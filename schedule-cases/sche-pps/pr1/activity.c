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

#include <core/thread.h>
#include <libc/stdio.h>
#include <types.h>

void *job1() {
  while (1) {
    // printf("P1T1: Hello, prio 66\n");
  }
}

void *job2() {
  while (1) {
    // printf("P1T2: World, prio 88\n");
  }
}
