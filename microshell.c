#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

void ft_err(char *str)
{
	while (*str)
		write(2, str++, 1);
}

void set_pipe(int has_pipe, int *fd, int end)
{
	if (has_pipe && (dup2(fd[end], end)==-1 || close(fd[0])==-1 || close(fd[1])==-1))
		ft_err("error: fatal\n"), exit(1);
}

int ft_cd(char **argv, int i)
{
	if (i!=2)
		return ft_err("error: cd: bad arguments\n"), 1;
	if (chdir(argv[1])==-1)
		return ft_err("error: cd: cannot change directory to "), ft_err(argv[1]), ft_err("\n"), 1;
	return 0;
}

int	exec(char **argv, int i, char **envp)
{
	int has_pipe, fd[2], pid, status;
	has_pipe=argv[i] && !strcmp(argv[i], "|");

	if (!has_pipe && !strcmp(*argv, "cd"))
		return ft_cd(argv, i);
	if (has_pipe && pipe(fd)==-1)
		ft_err("error: fatal\n"), exit(1);
	if ((pid=fork())==-1)
		ft_err("error: fatal\n"), exit(1);
	if (!pid)
	{
		argv[i]=0;
		set_pipe(has_pipe, fd, 1);
		if (!strcmp(*argv, "cd"))
			exit(ft_cd(argv, i));
		execve(*argv, argv, envp);
		ft_err("error: cannot execute "), ft_err(*argv), ft_err("\n"), exit(1);
	}
	waitpid(pid, &status, 0);
	set_pipe(has_pipe, fd, 0);
	return WIFEXITED(status) && WEXITSTATUS(status);
}

int main(int argc, char **argv, char **envp)
{
	(void)argc;
	int i=0, status=0;

	while (argv[i])
	{
		argv+=i+1;
		i=0;
		while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
			i++;
		if (i) status=exec(argv, i, envp);
	}
	return status;
}
