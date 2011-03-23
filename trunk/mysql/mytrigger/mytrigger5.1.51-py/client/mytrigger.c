#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
#include <libgen.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#define MYTRIGGER_OK 0
#define MYTRIGGER_ERR -1
#define WORKER_NAME "mytrigger-worker"
#ifndef MAX_PATH
#define MAX_PATH 1024
#endif
#define DEFAULT_DAEMONIZE 1
#define DELAY_ERROR       3000 * 1000
#define DELAY_RESTARTING   500 * 1000

#define MYTRIGGER_EXIT 1
#define MYTRIGGER_RESTART 2

int is_daemonize = DEFAULT_DAEMONIZE;
pid_t child_pid;

int
daemonize(int nochdir, int noclose)
{
	struct sigaction osa, sa;
	int fd;
	pid_t newgrp;
	int oerrno;
	int osa_ok;

	/* A SIGHUP may be thrown when the parent exits below. */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	osa_ok = sigaction(SIGHUP, &sa, &osa);

	switch (fork()) {
	case -1:
		return (-1);
	case 0:
		break;
	default:
		exit(0);
	}

	newgrp = setsid();
	oerrno = errno;
	if (osa_ok != -1)
		sigaction(SIGHUP, &osa, NULL);

	if (newgrp == -1) {
		errno = oerrno;
		return (-1);
	}

	if (!nochdir) {
		(void)chdir("/");
	}

	if (!noclose && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
		(void)dup2(fd, STDIN_FILENO);
		(void)dup2(fd, STDOUT_FILENO);
		(void)dup2(fd, STDERR_FILENO);
		if (fd > 2) {
			(void)close(fd);
		}
	}
	return (0);
}

static pid_t
process_launch(char *path, char **argv)
{
	pid_t pid;

	/* Execute the server */
	pid = fork();
	if(pid == 0) {
		argv[0] = path;
		execvp(path, argv);

		printf("ERROR: Could not execute %s\n", path);
		exit(1);
	}
	
	return pid;
}

static int
process_wait(pid_t pid)
{
	pid_t	re;
	int		rc;
	int		exitcode = 0;

	while(1) {
		re = waitpid(pid, &exitcode, 0);
		if (re > 0) {
			break;
		} else if (errno == EINTR) {
			break;
		} else if (errno == ECHILD) {
			return (MYTRIGGER_OK);
		} else {
			return (MYTRIGGER_ERR);
		}
	}

	if (WIFEXITED(exitcode)) {
		rc = WEXITSTATUS(exitcode);
		/* Child terminated normally */ 
		printf("PID %d: exited rc=%d\n", pid, rc);
		if (rc == MYTRIGGER_EXIT) {
			printf("mytrigger stop!\n");	
			exit(0);
		}
		if(rc != 0) {
			return (MYTRIGGER_ERR);
		}
	}  else if(WIFSIGNALED(exitcode)) {
		/* XXX coredump->restart? */
		/* Child process terminated by a signal */
		printf("PID %d: received a signal=%d[%s]\n", 
			pid, WTERMSIG(exitcode), sys_siglist[WTERMSIG(exitcode)]);
	}

	return (MYTRIGGER_OK);
}

static void 
signals_handler(int sig, siginfo_t *si, void *context) 
{
	(void)(context);

	switch(sig) {
		case SIGUSR1:
			/* Restart: the tough way */
			kill(child_pid, SIGINT);
			process_wait(child_pid);
			break;

		case SIGHUP:
			/* Graceful restart */
			kill(child_pid, SIGHUP);
			break;

		case SIGINT:
		case SIGTERM:
			/* Kill child and exit */
			kill(child_pid, SIGTERM);
			process_wait(child_pid);
			exit(0);

		case SIGCHLD:
			/* Child exited */
			process_wait(si->si_pid);
			break;

		default:
			/* Forward the signal */
			kill(child_pid, sig);
	}
}

static void
set_signals(void)
{
	struct sigaction act;

	/* Signals it handles
	 */
	memset(&act, 0, sizeof(act));

	act.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &act, NULL);
	
	/* Signals it handles
	 */
	act.sa_sigaction = signals_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;

	sigaction(SIGHUP,  &act, NULL);
	sigaction(SIGINT,  &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGUSR1, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);
	sigaction(SIGCHLD, &act, NULL);
}

int
main(int argc, char **argv)
{
	int		ret;
	char worker_path[MAX_PATH];

	snprintf(worker_path, MAX_PATH, "%s/%s", dirname(argv[0]), WORKER_NAME);
	printf("start %s...\n", worker_path);
	
	//ret = chdir(dirname(argv[0]));
	//printf("ret=%d\n", ret);
	set_signals();

	if (is_daemonize)
		daemonize(1, 1);

	while(1) {
		child_pid = process_launch(worker_path, argv);
		if (child_pid < 0) {
			printf("Couldn't launch \n");
			exit(1);
		}
		printf("Mytrigger Child process id: %d\n", child_pid);

		ret = process_wait(child_pid);

		usleep ((ret == 0) ?
			DELAY_RESTARTING :
			DELAY_ERROR);
	}

	return (0);
}

