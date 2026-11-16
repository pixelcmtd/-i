#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    const char *file;
    const char *slash;
    const char suffix[] = ".i-XXXXXX";
    struct stat st;
    struct sigaction action, child_action;
    sigset_t signals, oldmask, pending;
    const int interrupts[] = { SIGINT, SIGTERM, SIGHUP };
    char *tmp = NULL;
    size_t dirlen, i;
    pid_t pid, waited;
    int input = -1, output = -1, status, result = 1, created = 0;

    if (argc < 3) {
        fprintf(stderr, "usage: %s [file] [command [arg ...]]\n", argv[0]);
        return 2;
    }

    sigemptyset(&signals);
    sigaddset(&signals, SIGCHLD);
    for (i = 0; i < sizeof(interrupts) / sizeof(interrupts[0]); i++) {
        if (sigaction(interrupts[i], NULL, &action) < 0) {
            perror("-i: sigaction");
            goto done;
        }
        if (action.sa_handler != SIG_IGN)
            sigaddset(&signals, interrupts[i]);
    }
    if (sigprocmask(SIG_BLOCK, &signals, &oldmask) < 0) {
        perror("-i: sigprocmask");
        goto done;
    }
    action.sa_handler = SIG_DFL;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGCHLD, &action, &child_action) < 0) {
        perror("-i: sigaction");
        goto done;
    }

    file = argv[1];
    input = open(file, O_RDONLY | O_NONBLOCK);
    if (input < 0) {
        perror(file);
        goto done;
    }
    if (fstat(input, &st) < 0) {
        perror(file);
        goto done;
    }
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "-i: %s is not a regular file\n", file);
        goto done;
    }

    slash = strrchr(file, '/');
    dirlen = slash ? (size_t)(slash - file + 1) : 0;
    tmp = malloc(dirlen + sizeof(suffix));
    if (!tmp) {
        perror("-i: malloc");
        goto done;
    }
    memcpy(tmp, file, dirlen);
    memcpy(tmp + dirlen, suffix, sizeof(suffix));
    output = mkstemp(tmp);
    if (output < 0) {
        perror("-i: mkstemp");
        goto done;
    }
    created = 1;

    pid = fork();
    if (pid < 0) {
        perror("-i: fork");
        goto done;
    }
    if (pid == 0) {
        if (sigaction(SIGCHLD, &child_action, NULL) < 0 ||
            sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
            perror("-i: restore signals");
            _exit(126);
        }
        if (dup2(input, STDIN_FILENO) < 0) {
            perror("-i: dup2");
            _exit(126);
        }
        if (input != STDIN_FILENO)
            close(input);
        if (dup2(output, STDOUT_FILENO) < 0) {
            perror("-i: dup2");
            _exit(126);
        }
        if (output != STDOUT_FILENO)
            close(output);
        execvp(argv[2], argv + 2);
        status = errno;
        perror(argv[2]);
        _exit(status == ENOENT ? 127 : 126);
    }

    close(input);
    input = -1;
    while ((waited = waitpid(pid, &status, WNOHANG)) != pid) {
        int sig, error;

        if (waited < 0) {
            if (errno == EINTR)
                continue;
            perror("-i: waitpid");
            goto done;
        }
        error = sigwait(&signals, &sig);
        if (error != 0) {
            errno = error;
            perror("-i: sigwait");
            goto done;
        }
        if (sig != SIGCHLD) {
            result = 128 + sig;
            if (kill(pid, SIGKILL) < 0 && errno != ESRCH) {
                perror("-i: kill");
                goto done;
            }
            while (waitpid(pid, &status, 0) < 0 && errno == EINTR)
                ;
            goto done;
        }
    }
    result = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
    if (result != 0)
        goto done;

    if (fchown(output, st.st_uid, st.st_gid) < 0) {
        perror("-i: fchown");
        result = 1;
        goto done;
    }
    if (fchmod(output, st.st_mode & 07777) < 0) {
        perror("-i: fchmod");
        result = 1;
        goto done;
    }
    if (close(output) < 0) {
        output = -1;
        perror("-i: close");
        result = 1;
        goto done;
    }
    output = -1;
    if (sigpending(&pending) < 0) {
        perror("-i: sigpending");
        result = 1;
        goto done;
    }
    for (i = 0; i < sizeof(interrupts) / sizeof(interrupts[0]); i++) {
        if (sigismember(&signals, interrupts[i]) &&
            sigismember(&pending, interrupts[i])) {
            result = 128 + interrupts[i];
            goto done;
        }
    }
    if (rename(tmp, file) < 0) {
        perror("-i: rename");
        result = 1;
        goto done;
    }
    result = 0;

done:
    if (input >= 0)
        close(input);
    if (output >= 0)
        close(output);
    if (created && result != 0)
        unlink(tmp);
    free(tmp);
    return result;
}
