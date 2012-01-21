/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * Checks that CPU ID features match instructions found in executable.
 *
 * Note: The following functions are used to define a validator function
 * for collecting this information. See header file ncvalidator_iter.h
 * for more information on how to register these functions as a validator
 * function.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVAL_REG_SFI_NC_CPU_CHECKS_H__
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVAL_REG_SFI_NC_CPU_CHECKS_H__

#include <stdio.h>

/* Defines a validator state. */
struct NaClValidatorState;

/* Defines an instruction iterator that processes a code segment. */
struct NaClInstIter;

/* Defines the set of cpu feature checks applied to the code. */
struct NaClCpuCheckState;

/* Creates a CPU feature struct (all fields initialized to false), to be used
 * to record what features need to be squashed out of the executable.
 */
struct NaClCpuCheckState* NaClCpuCheckMemoryCreate(
    struct NaClValidatorState* state);

/* Destroys the CPU feature list generated by NcCpuCheckMemoryCreate. */
void NaClCpuCheckMemoryDestroy(struct NaClValidatorState* state,
                               struct NaClCpuCheckState* checked_features);

/* Check that cpu features match instructions for native client rules. */
void NaClCpuCheck(struct NaClValidatorState* state,
                  struct NaClInstIter* iter,
                  struct NaClCpuCheckState* checked_features);

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVAL_REG_SFI_NC_CPU_CHECKS_H__ */