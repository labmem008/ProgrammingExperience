# Simple Shell
### External Resources
1. https://stackoverflow.com/questions/6245477/bad-file-descriptor
2. https://stackoverflow.com/questions/7171722/how-can-i-handle-sigchld/7171836#7171836
3. https://www.youtube.com/results?search_query=sigaction+zombie
4. https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/exec.html
### How a program is launched by a shell?
1. The parent process will create a child process.
2. The child process executes the program specified by user.
3. Parent process waits for the child to finish.
### Background/Foreground Management
```c
void handle_child(pid_t pid, int* wstatus) {
    waitpid(pid, wstatus, 0);
}
void handle_background_child() {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_NOCLDWAIT; // so that no zombie is created
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("ERROR");
        exit(1);
    }
}
```
From `man sigaction - SA_NOCLDWAIT`:

> If `signum` is `SIGCHLD`, do not transform children into zombies when they terminate.  See also `waitpid(2)`.  This flag is meaningful only when establishing a handler for `SIGCHLD`, or when setting that signal's disposition to `SIG_DFL`.

