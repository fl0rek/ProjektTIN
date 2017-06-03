#pragma once
#define __fdebug_hh__

#ifndef __cplusplus
#warning you probably should be using C version of this header
#endif

#ifdef __fdebug_h__
#error cannot include both fdebug.hh and fdebug.h
#endif

// based on
// http://c.learncodethehardway.org/book/ex20.html
// repurposed for c++ and exceptions by florek

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>


#include <stdexcept>

#ifdef NDEBUG
#define debug1(M)
#define debug(M, ...)
#else
#define debug1(M) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__)
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err1(M) fprintf(stderr, "[ERROR %d] (%s:%d: errno: %s) " M "\n", getpid(), __FILE__, __LINE__, clean_errno())
#define log_err(M, ...) fprintf(stderr, "[ERROR %d] (%s:%d: errno: %s) " M "\n", getpid(), __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define check(A, E, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); throw new E();}
#define check1(A, E, M) if(!(A)) { log_err1(M); throw new E();}

#define debug_check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__);}
#define debug_check1(A, M) if(!(A)) { log_err1(M);}

#define log_warn(M, ...) fprintf(stderr, "[WARN %d] (%s:%d: errno: %s) " M "\n", getpid(), __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define log_warn1(M) fprintf(stderr, "[WARN %d] (%s:%d: errno: %s) " M "\n", getpid(), __FILE__, __LINE__, clean_errno())

#define log_info1(M) fprintf(stderr, "[INFO %d] (%s:%d) " M "\n", getpid(), __FILE__, __LINE__)
#define log_info(M, ...) fprintf(stderr, "[INFO %d] (%s:%d) " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__)

#define RUNTIME_EXC(NAME, WHAT) class NAME : public std::runtime_error { \
public: \
	NAME() : runtime_error(WHAT) {} \
}

class ShouldNotOccourException : public std::runtime_error {
public:
	ShouldNotOccourException()
		: runtime_error("Entered fragment of code that should not be reachable") {}
};

#define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); throw new ShouldNotOccourException();}

#define UNUSED(x) (void)(x)
