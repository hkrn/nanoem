#define THREAD_IMPLEMENTATION
#if !defined(_WIN32)
#include <errno.h>
#include <stdint.h>
#include <string.h> /* for memset */
#include <strings.h>
#include <pthread.h> /* for pthread_setname_np */
#endif
#include "thread.h"
