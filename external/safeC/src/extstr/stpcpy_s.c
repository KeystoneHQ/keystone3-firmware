/*------------------------------------------------------------------
 * stpcpy_s.c
 *
 * November 2020, Reini Urban
 *
 * Copyright (c) 2020 by Reini Urban
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *------------------------------------------------------------------
 */

#ifdef FOR_DOXYGEN
#include "safe_str_lib.h"
#else
#include "safeclib_private.h"
#endif

/* TODO not via the naive byte copy, but aligned long word copy
   via the (((X) - 0x0101010101010101) & ~(X) & 0x8080808080808080)
   trick */

/**
 * @def stpcpy_s(dest,dmax,src,errp)
 * @brief
 *    The stpcpy_s function copies the string pointed to by src
 *    (including the terminating null character) into the array
 *    pointed to by dest. The strings may not overlap.
 *    With SAFECLIB_STR_NULL_SLACK defined all elements following the
 *    terminating null character (if any) written by stpcpy_s in the
 *    array of dmax characters pointed to by dest are nulled when
 *    stpcpy_s returns.
 *    With modern compilers and constant arguments most errors
 *    will be caught at compile-time.
 *
 * @remark SPECIFIED IN
 *    * Since glibc 2.10:  _POSIX_C_SOURCE >= 200809L
 *    * Before glibc 2.10:  _GNU_SOURCE
 *
 * @param[out]  dest  pointer to string that will be replaced by src.
 * @param[in]   dmax  restricted maximum length of dest
 * @param[in]   src   pointer to the string that will be copied to dest
 * @param[out]  errp  EOK        success. the characters in src were
 *                               copied into dest and the result is null terminated.
 *                    ESNULLP    when dest or src is a NULL pointer
 *                    ESZEROL    when dmax = 0
 *                    ESLEMAX    when dmax > RSIZE_MAX_STR
 *                    EOVERFLOW  when dmax > size of dest (optionally, when the compiler
 *                               knows the object_size statically)
 *                    ESLEWRNG   when dmax != size of dest and --enable-error-dmax
 *                    ESUNTERM   when src is unterminated
 *                    ESOVRLP    when strings overlap
 *                    ESNOSPC    when dest < src
 *
 * @pre Neither dest, src nor errp nor shall be a null pointer.
 * @pre dmax shall be size of dest
 * @pre dmax shall not be greater than RSIZE_MAX_STR or size of dest.
 * @pre dmax shall not equal zero.
 * @pre dmax shall be greater than strnlen_s(src, dmax).
 * @pre Copying shall not take place between objects that overlap.
 *
 * @note C11 uses RSIZE_MAX, not RSIZE_MAX_STR.
 *
 * @return  stpcpy_s() returns a pointer to the end of the string dest (that is,
 *          the address of the terminating null byte) rather than the beginning.
 * @return  If there is a runtime-constraint violation, and if dest
 *          and dmax are valid, then stpcpy_s nulls dest.
 *
 * @see
 *    stpncpy_s(), strcpy_s(), strncpy_s()
 *
 */
#ifdef FOR_DOXYGEN
char *stpcpy_s(char *restrict dest, rsize_t dmax, const char *restrict src,
               errno_t *restrict errp)
#else
EXPORT char *_stpcpy_s_chk(char *restrict dest, rsize_t dmax,
                           const char *restrict src, errno_t *restrict errp,
                           const size_t destbos, const size_t srcbos)
#endif
{
    rsize_t orig_dmax = dmax;
    char* orig_dest = dest;
    const char *overlap_bumper;
    size_t slen;

    if (unlikely(errp == NULL)) {
        invoke_safe_str_constraint_handler("stpcpy_s: errp is null",
                                           (void *)dest, ESNULLP);
        return NULL;
    }
    if (unlikely(dest == NULL)) {
        invoke_safe_str_constraint_handler("stpcpy_s: dest is null",
                                           (void *)dest, ESNULLP);
        *errp = RCNEGATE(ESNULLP);
        return NULL;
    }
    if (unlikely(dmax == 0)) {
        invoke_safe_str_constraint_handler("stpcpy_s: dmax is 0", (void *)dest,
                                           ESNULLP);
        *errp = RCNEGATE(ESZEROL);
        return NULL;
    }
    if (destbos == BOS_UNKNOWN) {
        if (unlikely(dmax > RSIZE_MAX_STR)) {
            invoke_safe_str_constraint_handler("stpcpy_s: dmax exceeds max",
                                               (void *)dest, ESLEMAX);
            *errp = RCNEGATE(ESLEMAX);
            return NULL;
        }
        BND_CHK_PTR_BOUNDS(dest, dmax);
    } else {
        if (unlikely(dmax > destbos)) {
            if (dmax > RSIZE_MAX_STR) {
                handle_error(dest, destbos, "stpcpy_s: dmax exceeds max",
                             ESLEMAX);
                *errp = RCNEGATE(ESLEMAX);
                return NULL;
            } else {
                *errp = handle_str_bos_overload("stpcpy_s: dmax exceeds dest",
                                               (char *)dest, destbos);
                return NULL;
            }
        }
    }
    if (unlikely(src == NULL)) {
        handle_error(dest, _BOS_KNOWN(dest) ? BOS(dest) : dmax,
                     "strpcpy_s: src is null", ESNULLP);
        *errp = RCNEGATE(ESNULLP);
        return NULL;
    }
    slen = 0;

    if (unlikely(dest == src)) {
        /* walk to the terminating null character */
        while (dmax > 0) {
            if (*dest == '\0') {
                goto eok;
            }
            dmax--;
            dest++;
        }
        goto enospc;
    }

    if (dest < src) {
        overlap_bumper = src;

        while (dmax > 0) {
            if (unlikely(dest == overlap_bumper)) {
                handle_error(orig_dest, orig_dmax,
                             "stpcpy_s: "
                             "overlapping objects",
                             ESOVRLP);
                *errp = RCNEGATE(ESOVRLP);
                return NULL;
            }

            *dest = *src;
            if (*dest == '\0')
                goto eok;
            dmax--;
            slen++;
            dest++;
            src++;
            /* sentinel srcbos -1 = ULONG_MAX */
            if (unlikely(slen >= srcbos)) {
                invoke_safe_str_constraint_handler("stpcpy_s: src unterminated",
                                                   (void *)src, ESUNTERM);
                *errp = RCNEGATE(ESUNTERM);
                return NULL;
            }
        }
    } else {
        overlap_bumper = dest;

        while (dmax > 0) {
            if (unlikely(src == overlap_bumper)) {
                handle_error(orig_dest, orig_dmax,
                             "stpcpy_s: "
                             "overlapping objects",
                             ESOVRLP);
                *errp = RCNEGATE(ESOVRLP);
                return NULL;
            }

            *dest = *src;
            if (*dest == '\0') {
              eok:
#ifdef SAFECLIB_STR_NULL_SLACK
                /* null slack to clear any data */
                if (dmax > 0x20)
                    memset(dest, 0, dmax);
                else {
                    while (dmax) {
                        *dest = '\0';
                        dmax--;
                        dest++;
                    }
                }
#endif
                *errp = RCNEGATE(EOK);
                return dest;
            }

            dmax--;
            slen++;
            dest++;
            src++;
            if (unlikely(slen >= srcbos)) {
                invoke_safe_str_constraint_handler("stpcpy_s: src unterminated",
                                                   (void *)src, ESUNTERM);
                *errp = RCNEGATE(ESUNTERM);
                return NULL;
            }
        }
    }

  enospc:
    /*
     * the entire src must have been copied, if not reset dest
     * to null the string. (only with SAFECLIB_STR_NULL_SLACK)
     */
    handle_error(orig_dest, orig_dmax,
                 "stpcpy_s: not enough space for src",
                 ESNOSPC);
    *errp = RCNEGATE(ESNOSPC);
    return NULL;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(_stpcpy_s_chk);
#endif /* __KERNEL__ */

