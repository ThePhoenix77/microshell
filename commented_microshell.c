#include <string.h>  // for strcmp
#include <unistd.h>  // for write, chdir, fork, execve, pipe, close
#include <sys/wait.h>  // for waitpid and macros
#include <stdlib.h>  // for exit

// Function to display an error message
void ft_err(char *str)
{
	while (*str)
		write(2, str++, 1);  // Write each character of the error message to standard error (fd 2)
}

// Function to set up pipes if needed
void set_pipe(int has_pipe, int *fd, int end)
{
	// If has_pipe is true, duplicate the appropriate file descriptor
	// for the end of the pipe, then close both ends
	if (has_pipe && (dup2(fd[end], end) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
	{
		ft_err("error: fatal\n");
		exit(1);  // Exit if there's an error setting up the pipe
	}
}

// Function to handle the `cd` command
int ft_cd(char **argv, int i)
{
	// Check if there are exactly 2 arguments; if not, return an error
	if (i != 2)
		return ft_err("error: cd: bad arguments\n"), 1;

	// Try changing directory to the second argument (argv[1])
	if (chdir(argv[1]) == -1)
		return ft_err("error: cd: cannot change directory to "), ft_err(argv[1]), ft_err("\n"), 1;

	return 0;  // Return 0 on success
}

// Function to execute a command
int exec(char **argv, int i, char **envp)
{
	int has_pipe, fd[2], pid, status;
	has_pipe = argv[i] && !strcmp(argv[i], "|");  // Check if the next argument is a pipe ("|")

	// If the command is `cd` and there’s no pipe, handle it here
	if (!has_pipe && !strcmp(*argv, "cd"))
		return ft_cd(argv, i);

	// Set up a pipe if needed
	if (has_pipe && pipe(fd) == -1)
	{
		ft_err("error: fatal\n");
		exit(1);
	}

	// Fork a new process
	if ((pid = fork()) == -1)
	{
		ft_err("error: fatal\n");
		exit(1);
	}

	// In the child process
	if (!pid)
	{
		argv[i] = 0;  // Terminate the argv array to isolate the command
		set_pipe(has_pipe, fd, 1);  // Set up the pipe for writing if needed

		// If the command is `cd`, handle it and exit
		if (!strcmp(*argv, "cd"))
			exit(ft_cd(argv, i));

		// Execute the command using execve
		execve(*argv, argv, envp);

		// If execve fails, output an error message and exit
		ft_err("error: cannot execute ");
		ft_err(*argv);
		ft_err("\n");
		exit(1);
	}

	// In the parent process
	waitpid(pid, &status, 0);  // Wait for the child process to finish
	set_pipe(has_pipe, fd, 0);  // Set up the pipe for reading if needed

	return WIFEXITED(status) && WEXITSTATUS(status);  // Return the child's exit status if it exited normally
}

// Main function to handle command-line arguments and execute them sequentially
int main(int argc, char **argv, char **envp)
{
	(void)argc;  // Suppress unused variable warning for argc
	int i = 0, status = 0;

	while (argv[i])
	{
		argv += i + 1;  // Move argv to the next command
		i = 0;

		// Find the next pipe ("|") or command separator (";")
		while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
			i++;

		// If there’s a command to execute, call exec
		if (i)
			status = exec(argv, i, envp);
	}

	return status;  // Return the exit status of the last executed command
}
