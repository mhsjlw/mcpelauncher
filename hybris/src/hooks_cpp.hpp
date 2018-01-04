#pragma once

// #include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif
int my_pthread_once(void *once_control, void (*init_routine)(void));
#ifdef __cplusplus
}
#endif
