/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


/*
 * NaCl Safety Macro Definitions
 */
#ifndef NATIVE_CLIENT_SRC_INCLUDE_NACL_MACROS_H_
#define NATIVE_CLIENT_SRC_INCLUDE_NACL_MACROS_H_ 1

#include <stdio.h>
#include <stdlib.h>

/*****************************************************************************
 * Safety macros                                                             *
 *****************************************************************************/

#define NACL_ARRAY_SIZE_UNSAFE(arr) ((sizeof arr)/sizeof arr[0])

/*
 * ASSERT_IS_ARRAY(arr) generates a somewhat opaque compile-time
 * error if arr is a non-array pointer.  This protects against
 * situations where one writes code like:
 *
 * foo.h:  struct Foo { char buffer[BUFFERSIZE]; size_t sofar; ... };
 *
 * foo.c:  got = read(d, fp->buffer + fp->sofar, sizeof fp->buffer - fp->sofar);
 *         if (-1 == got) { ... }
 *         fp->sofar += got;
 *
 *         for (ix = 0; ix < sizeof arr/sizeof arr[0]; ++ix) { ... }
 *
 * and have it break and create a security problem when somebody later
 * changes Foo to dynamically allocate buffer, viz,
 *
 * foo.h:  struct Foo { char *buffer; size_t sofar; ... };
 *
 * and now sizeof fp->buffer is 4 or 8, with size_t (type of sizeof)
 * being unsigned, when fp->sofar is larger than 4 or 8, getting an
 * enormous maximum read size being used.  Such bugs can remain
 * undiscovered when conforming implementations of protocol engines
 * are used where the actual amount sent is small and would never
 * cause a buffer overflow, but an adversarial implementation would be
 * able to clobber the heap.  The solution is to write:
 *
 * foo.c:  NACL_ASSERT_IS_ARRAY(fp->buffer);
 *         got = read(d, fp->buffer + fp->sofar, sizeof fp->buffer - fp->sofar);
 *         if (-1 == got) { ... }
 *         fp->sofar += got;
 *
 *         for (ix = 0; ix < NACL_ARRAY_SIZE(arr); ++ix) { ... }
 *
 * and when foo.h is modified, it will generate a compile-time error
 * alerting the engineer makin the change that the read code will need to
 * be modified.
 *
 * NB: The -pedantic flag is REQUIRED for the C version to catch the
 *     error.  No special warning flags are required for the C++
 *     version to work.
 */

/*
 * The C version uses the fact that __builtin_types_compatible_p can
 * be used to discriminate between T * and T *const.  (Note that this
 * difference is not a top-level qualifier difference as mentioned in
 * the gcc info node; that would apply to T * versus T const *.)  In
 * the assertion statement version (NACL_ASSERT_IS_ARRAY), we use this
 * to allocate an array, and ISO C forbids a zero-sized (or
 * negative-sized) array.  In the expression version (ARRAY_SIZE), we
 * assign to a global void * -- assigning a zero is fine, but
 * assigning a 1 results in a warning that making a pointer from an
 * integer is verboten.  When ARRAY_SIZE is used in a loop control
 * context, e.g.,
 *
 * for (ix = 0; ix < ARRAY_SIZE(arr); ++ix) { ... }
 *
 * with -O the optimizer recognizes that the store can be moved out of
 * the loop, so the performance impact should be minimal.
 */
# if __GNUC__
#  define NACL_ASSERT_IS_ARRAY(arr)                           \
  do {                                                        \
    char __is_array__[1-2*__builtin_types_compatible_p(       \
        __typeof__(&arr[0]),                                  \
        __typeof__(arr))];                                    \
    /* dead code, but gets rid of unused-variable warnings */ \
    if (0 == sizeof __is_array__) {                           \
      abort();                                                \
    }                                                         \
  } while (0)

static inline void *NaClArrayCheckHelper(void *arg) {
  /*
   * Doing runtime checks is not really necessary -- this code is in
   * fact unreachable code that gets optimized out when used with the
   * NACL_ARRAY_SIZE definition below.
   *
   * The runtime check is only useful when the build system is using
   * the inappropriate flags (e.g., missing -pedantic -Werror or
   * -pedantic-error), in which case instead of a compile-time error,
   * we'd get a runtime error.
   */
  if (NULL != arg) {
    abort();
  }
  return arg;
}

#  define NACL_ARRAY_SIZE(arr)                                         \
  (NaClArrayCheckHelper(                                               \
      __builtin_types_compatible_p(__typeof__(&arr[0]),                \
                                   __typeof__(arr))),                  \
  NACL_ARRAY_SIZE_UNSAFE(arr))
# else  /* __GNUC__ */

/*
 * Not gcc.  So far, we only compile NaCl under gcc and visual studio,
 * but if/when a new compiler is introduced that's capable of doing
 * compile-time checking (or we figure out how to do it w/ visual
 * studio), check for those compilers here, and enable the
 * corresponding compile-failure tests in
 * src/src/service_runtime/build.scons.
 */

#  define NACL_ASSERT_IS_ARRAY(arr)
#  define NACL_ARRAY_SIZE(arr) NACL_ARRAY_SIZE_UNSAFE(arr)
# endif  /* __GNUC__ */

/*
 * NACL_ASSERT_IS_POINTER(arr) generates a somewhat opaque compile-time
 * error if lvalue is not a pointer lvalue but is instead an actual
 * array (which is a T * const object).  This is complementary to
 * NACL_ASSERT_IS_ARRAY.
 */
#define NACL_ASSERT_IS_POINTER(ptr) do { if (0) { ++ptr; } } while (0)

/*
 * NACL_ASSERT_SAME_SIZE(t1, t2) verifies that the two types have the same size
 * (as reported by sizeof).  When the check fails it generates a somewhat
 * opaque warning, mitigated by the variable's name.
 *
 * Examples:
 *   NACL_ASSERT_SAME_SIZE(void *, char *);  // Likely to succeed!
 *   NACL_ASSERT_SAME_SIZE(char, long);  // Unlikely to succeed
 */
#define NACL_ASSERT_SAME_SIZE(t1, t2) \
  do { char tested_types_are_not_the_same_size[sizeof(t1) == sizeof(t2)]; \
       (void) tested_types_are_not_the_same_size; } while (0)


/*
 * NACL_COMPILE_TIME_ASSERT(boolexp) verifies that the argument
 * boolexp is true.  The check occurs at compile time, assuming
 * -pedantic flag or similar is used so that the ISO C forbidden
 * zero-sized array generates an error.  This is standard with NaCl
 * code.  If the wrong compilation flags are used, then we would get a
 * run-time abort.
 *
 * Example:
 *
 * NACL_COMPILE_TIME_ASSERT(NACL_MAX_VAL(int32_t) <= SIZE_T_MAX)
 *
 * to explicitly state the assumption that an int32_t expression -- if
 * containing a non-negative number -- will fit in a size_t variable.
 */
#define NACL_COMPILE_TIME_ASSERT(boolexp)                          \
  do {                                                             \
    char compile_time_boolean_expression_is_false[0 != (boolexp)]; \
    if (0 == sizeof compile_time_boolean_expression_is_false) {    \
      abort();                                                     \
    }                                                              \
  } while (0)

/*****************************************************************************
 * MAX/MIN macros for integral types                                         *
 ****************************************************************************/

/*
 * For NACL_MAX_VAL, T must be a type where u ## T is the unsigned
 * version of the type.
 *
 * These macros rely on -1 being signed extended to the width of T (or
 * u ## T), and on two's complement representation of integers.
 *
 * Generally, stdint.h's INT16_MAX etc can be used, but these are
 * useful for macros that take a type parameter and need the max or
 * min value for the type, since then the macro would not have to also take
 * the max or min value as additional parameter(s).
 */
#define NACL_UMAX_VAL(T)  ((T) -1)
#define NACL_MAX_VAL(T)   ((T) (((u ## T) -1) >> 1))
#define NACL_UMIN_VAL(T)  ((T) 0)
#define NACL_MIN_VAL(T)   ((T) ~NACL_MAX_VAL(T))


/*****************************************************************************
 * Readability macros                                                        *
 ****************************************************************************/

#define NACL_NANOS_PER_MICRO          (1000)
#define NACL_100_NANOS_PER_MILLI      (10 * 1000)
#define NACL_NANOS_PER_MILLI          (1000 * 1000)
#define NACL_MICROS_PER_MILLI         (1000)
#define NACL_MICROS_PER_UNIT          (1000 * 1000)
#define NACL_MILLIS_PER_UNIT          (1000)
#define NACL_UNIT_CONVERT_ROUND(v, m) (((v) + (m) - 1)/(m))

#define NACL_NO_FILE_DESC             (-1)
#define NACL_NO_URL                   ""
#define NACL_NO_FILE_PATH             ""

#define NACL_HTTP_STATUS_OK           200

#endif  /* NATIVE_CLIENT_SRC_INCLUDE_NACL_MACROS_H_ */
