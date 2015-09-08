// part of nullmon.c from newlib
/* nullmon.c - Stub or monitor services.
 *
 * Copyright (c) 1998 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
struct s_mem
{ unsigned int size;
  unsigned int icsize;
  unsigned int dcsize;
};

extern char _end[];
void
__attribute__((noinline))
get_mem_info
(struct s_mem *mem)
{
  asm volatile ("");
  //mem->size = BOARD_MEM_SIZE - (_end - _ftext);
  mem->size = (2<<20) - (_end - (char *)0x80000000);
}


