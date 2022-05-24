/*************************************************************************************
* Copyright (c) 2019 Xiaoyang Xu, Masoud Ghaffarinia, Wenhao Wang, and Kevin Hamlen *
* The University of Texas at Dallas                                                 *
*                                                                                   *
* Permission is hereby granted, free of charge, to any person obtaining a copy of   *
* this software and associated documentation files (the "Software"), to deal in     *
* the Software without restriction, including without limitation the rights to      *
* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of  *
* the Software, and to permit persons to whom the Software is furnished to do so,   *
* subject to the following conditions:                                              *
*                                                                                   *
* The above copyright notice and this permission notice shall be included in all    *
* copies or substantial portions of the Software.                                   *
*                                                                                   *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR        *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS  *
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR    *
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER    *
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN           *
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
*************************************************************************************/

/* This file is part of the ConFIRM test suite, whose initial documentation can be
found in the following publication:

Xiaoyang Xu, Masoud Ghaffarinia, Wenhao Wang, Kevin W. Hamlen, and Zhiqiang Lin.
"ConFIRM: Evaluating Compatibility and Relevance of Control-flow Integrity
Protections for Modern Software."  In Proceedings of the 28th USENIX Security
Symposium, August 2019. */

#include "setup.h"

void callee1(bool *flag1);
void callee2(bool *flag2);
// min estimated max size of callee1 and callee 2
#define ESTIMATED_FUNC_SIZE 100
static bool flag_1 = FALSE;
static bool flag_2 = FALSE;

typedef void(*FUNCPTR)(bool *);

int main()
{
    DWORD page_size;
    unsigned int size1;
    unsigned int size2;
    uint8_t *caller;
    int retP;
    int retF;

    page_size = getpagesize();

    // Allocate a page of memory (RW)
    caller = (uint8_t *)mmap(
        NULL,
	page_size,
        PROT_READ | PROT_WRITE,
        MAP_ANON | MAP_PRIVATE,
        -1,
        0
    );
    if (caller == MAP_FAILED)
    {
        printf("Memory allocation failed for caller: %s\n", strerror(errno));
        exit(1);
    }

    size1 = ((((unsigned long)callee1) + (ESTIMATED_FUNC_SIZE - 1)) / page_size + 1) * page_size - ((unsigned long)callee1);

    // Copy code to caller
    memcpy(caller, (void *)&callee1, size1);

    // Change memory protection to executable only.
    retP = mprotect(caller, page_size, PROT_EXEC);
    if (retP == -1)
    {
        printf("Virtual/Memory Protect failed: %s\n", strerror(errno));
        exit(1);
    }

    // Invoke caller, in which callee1 is called.
    ((FUNCPTR)caller)(&flag_1);

    // Change memory protection to RWX.
    retP = mprotect(caller, page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
    if (retP == -1)
    {
        printf("Virtual/Memory Protect failed: %s\n", strerror(errno));
        exit(1);
    }

    size2 = ((((unsigned long)callee2) + (ESTIMATED_FUNC_SIZE - 1)) / page_size + 1) * page_size - ((unsigned long)callee2);

    // Copy code to caller.
    memcpy(caller, (void *)&callee2, size2);

    // Invoke caller. This time caller2 is called.
    ((FUNCPTR)caller)(&flag_2);

    // Free the page allocated.
    retF = munmap(caller, page_size);
    if (retF == -1)
    {
        printf("VirtualFree/munmap failed freeing caller: %s\n", strerror(errno));
        exit(1);
    }

    if (flag_1 && flag_2)
    {
        printf("mem test passed\n");
    }
    else
    {
        printf("mem test failed\n");
    }

    return 0;
}

void callee1(bool *flag1)
{
    *flag1 = TRUE;
}

void callee2(bool *flag2)
{
    *flag2 = TRUE;
}
