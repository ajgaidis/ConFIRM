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

#if !(defined(INTEL_X86) || defined(AMD64))
#error This benchmark contains x86/x64-specific assembly code that may be \
incompatible with the current hardware architecture.
#endif

void JITCALLBACKPTR();

typedef void(*CALLBACKPTR)();

BYTE JIT_code[] = {
    // Prologue
    0x55,            // push ebp/rbp
    0x48,            // dec  eax 
                     // if compiled for 32bit otherwise extend ebp and esp to rbp and 
                     // rsp for next ins
    0x8B, 0xEC,      // mov  ebp, esp
    0x53,            // push ebx/rbx
    0x56,            // push esi/rsi
    0x57,            // push edi/rdi

    // Computed call to a function in the untrusted module
    0x48,            // dec  eax if compiled for 32bit otherwise extend ebx to rbx
                     // for next ins
    0xbb,            // mov  ebx, 0x00000000
    0x00,
    0x00,
    0x00,
    0x00,
    0x90,
    0x90,
    0x90,
    0x90,            // 8 bytes of address need to be patched -- NOP is added so
                     // that it works for both 32-bit and 64-bit machine
    0xff, 0x13,      // call ptr [ebx/rbx]

    // Epilogue
    0x5F,            // pop edi/rdi
    0x5E,            // pop esi/rsi
    0x5b,            // pop ebx/rbx
    0x48,            // dec  eax if compiled for 32bit otherwise extend ebp and esp
                     // to rbp and rsp for next ins
    0x8B, 0xE5,      // mov esp, ebp
    0x5D,            // pop ebp/rbp
    0xC3             // retn
};

int main()
{
    uint8_t *pJIT;
    int page_size;

    page_size = getpagesize();

    // Allocate a page of memory (RWX)    
    pJIT = (uint8_t *)mmap(
        NULL, 
        page_size, 
        PROT_READ | PROT_WRITE | PROT_EXEC, 
        MAP_ANON | MAP_PRIVATE, 
        -1, 
        0
    );
    if (pJIT == MAP_FAILED)
    {
        printf("Memory allocation failed for pJIT: %s\n", strerror(errno));
        exit(1);
    }

    // Patch JIT_code
    CALLBACKPTR pJITCALLBACKPTR = &JITCALLBACKPTR;
    void *ppJITCALLBACKPTR = &pJITCALLBACKPTR;
    memcpy(&(JIT_code[9]), &ppJITCALLBACKPTR, sizeof(pJITCALLBACKPTR));

    // Copy JIT_code to pJIT
    memcpy(pJIT, JIT_code, sizeof(JIT_code));

    // Call pJIT 
    ((CALLBACKPTR)pJIT)();

    // Free the page allocated
    munmap(pJIT, page_size);

    printf("jit test passed.\n");
    return 0;
}

void JITCALLBACKPTR()
{
    printf("This is a message in JITCALLBACKPTR()\n");
}
