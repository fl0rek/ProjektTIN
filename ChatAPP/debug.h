#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <errno.h>
#include <string>

void pthreadCreateError(int err)
{
    switch(err)
    {
        case EAGAIN:
        {
            perror("Insufficient resources\n");
            break;
        }
        case EINVAL:
        {
            perror("Invalid settings in attr\n");
            break;
        }
        case EPERM:{
            perror("No permission to set the scheduling policy\n");
            break;
        }
        default:
            perror("Creating thread failed - exiting\n");
    }
    exit(1);
}
void pthreadJoinError(int err)
{
    switch(err)
    {
        case EDEADLK:
        {
            perror("Deadlock was detected\n");
            break;
        }
        case EINVAL:
        {
            perror("Thread is not a joinable thread\n");
            break;
        }
        case ESRCH:{
            perror("No thread with the ID thread could be found\n");
            break;
        }
        default:
            perror("Creating thread failed - exiting\n");
    }
    exit(1);
}

#endif // DEBUG_H
