#ifndef _job_h_
#define _job_h_


#include <limits.h>
#include <signal.h>
typedef enum {
	STOPPED,
	TERM,
	BG,
	FG,
} JobStatus;

typedef struct {
	char *name;
	pid_t *pids;
	unsigned int npids;
	pid_t pgid;
	pid_t pgid_2;
	JobStatus status;
} Job;

#endif /*_job_h_ */
