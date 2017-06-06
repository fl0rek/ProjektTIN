/*
 * 					HEADER_HEAD
 * author: Mikolaj Florkiewicz
 * 					HEADER_TAIL
 */
#ifndef __dbg_h__
#define __dbg_h__

// source:
// http://c.learncodethehardway.org/book/ex20.html

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

#ifdef NDEBUG
#define debug1(M)
#define debug(M, ...)
#else
#define debug1(M) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__)
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err1(M) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno())
#define log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define log_warn1(M) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno())

#define log_info1(M) fprintf(stderr, "[INFO %d] (%s:%d) " M "\n", getpid(), __FILE__, __LINE__)
#define log_info(M, ...) fprintf(stderr, "[INFO %d] (%s:%d) " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__)

#define check1(A, M) if(!(A)) { log_err1(M); errno=0; goto error; }
#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define check_warn1(A, M) if(!(A)) { log_warn1(M); errno=0; }
#define check_warn(A, M, ...) if(!(A)) { log_warn(M, ##__VA_ARGS__); errno=0; }

#define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define check_mem(A) check1((A), "Out of memory.")

#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }

#define UNUSED(x) (void)(x)

#endif
