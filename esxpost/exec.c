
#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 0

#if DEBUG
#define dprintf(format, args...) printf("%s(%d): " format,__FUNCTION__,__LINE__, ## args)
#else
#define dprintf(format, args...) /* noop */
#endif

void striplf(char *line) {
	int len;

	len = strlen(line);
	if (!len) return;
	while(line[len-1] == '\r' || line[len-1] == '\n') line[--len] = 0;
}

int exec(char *prog, char **args) {
	pid_t pid;
	int fdout;
	char *log = "yes";

	/* Open log */
	if (log) {
//		fdout = open(log,O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0644);
		fdout = fileno(stdout);
		if (fdout < 0) {
			perror("open log");
			return 1;
		}
	}

#if 0
	/* Fork the process */
	pid = fork();

	/* If pid < 0, error */
	if (pid < 0) {
		perror("fork");
		return 1;
	}

	/* If pid > 0, parent */
	else if (pid > 0)
		_exit(0);

	/* Set the session ID */
	setsid();
#endif

	/* Fork again */
	pid = fork();
	if (pid < 0) {
		perror("fork");
		return 1;
	}

	/* Parent */
	else if (pid != 0) {
		int status;

		/* Wait for child to finish */
		dprintf("pid: %d\n", pid);
		waitpid(pid, &status, 0);

		/* Get exit status */
		dprintf("WIFEXITED: %d\n", WIFEXITED(status));
		if (WIFEXITED(status)) dprintf("WEXITSTATUS: %d\n", WEXITSTATUS(status));
		status = (WIFEXITED(status) ? WEXITSTATUS(status) : 1);
		dprintf("status: %d\n", status);

//		doit = (status == 0 && onerr == 1 ? 0 : 1);
		_exit(0);
	}

	/* Close stdin */
	close(0);

	/* If we have a log, make that stdout */
	if (log) {
		dup2(fdout, STDOUT_FILENO);
		close(fdout);
	} else {
		close(1);
	}

	/* Redirect stderr to stdout */
	dup2(1, 2);

	/* Exec prog */
	execvp(prog,args);
	exit(1);
}

int do_exec(char *cmd) {
#if 0
	char *prog, char *args[32];
	char *p;
	int i;
	char temp[256];

	i = 0;
#if 0
        /* SSH to the host and run mtp-post */
        dprintf("calling mtp-esx-post...\n");
        sprintf(cmd,"%s %s %s", SSH_SCRIPT, info->fqdn, (info->ng2 ? "NG2" : ""));
        dprintf("cmd: %s\n", cmd);
        system(cmd);

        /* Tell build mgr we're done */
        sprintf(cmd,"/usr/bin/curl --user \'%s:%s\' --insecure \"https://g9w0317g.americas.hpqcorp.net//serverbuildws/server.asmx/LockBuild?ServerName=%s&User=\'%s\'\"",VIM_USER,opspass,info->name,VIM_USER);
        dprintf("cmd: %s\n", cmd);
        system(cmd);
#endif
#endif
	return 0;
}
#endif
