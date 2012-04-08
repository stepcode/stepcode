#ifndef SCL_TRACE_FPRINTF_H
#define SCL_TRACE_FPRINTF_H

/** \file scl_trace_fprintf.h
 * Used to track the source file and line where generated code is printed from
 * When enabled, comments are printed into the generated files for every 'fprintf':
 * / * source: scl/src/fedex_plus/selects.c:1375 * /
 * To enable, configure with 'cmake .. -DSCL_TRACE_FPRINTF=ON'
 *
 * This header must be included *after* all other headers, otherwise the compiler will
 * report errors in system headers.
 * \sa trace_fprintf
**/

/** used to find where generated c++ originates from in fedex_plus
 * To enable, configure with 'cmake .. -DSCL_TRACE_FPRINTF=ON'
 */

#ifdef __cplusplus
extern "C" {
#endif
    void trace_fprintf (char const *sourcefile, int line, FILE* file, const char *format, ...);
#ifdef __cplusplus
}
#endif

#ifdef SCL_TRACE_FPRINTF
#   define fprintf(...) trace_fprintf(__FILE__, __LINE__, __VA_ARGS__)
#endif /* SCL_TRACE_FPRINTF */

#endif /* SCL_TRACE_FPRINTF_H */
