#include  <fcntl.h>
#include <unistd.h>
#include <be/kernel/OS.h>

int main(int argc, char **argv)
{
	int pid,fd1,fd2;
	sem_id sem1,sem2;
	sem1=create_sem(1,"piperun");
	sem2=create_sem(1,"piperun");
	if (sem1<B_NO_ERROR ||sem2<B_NO_ERROR) {
		perror("create_sem");
		exit(1);
	}
	acquire_sem(sem1);
	acquire_sem(sem2);
	pid=fork();
	if (pid==0) {
		fd1=open("/pipe/1",O_WRONLY|O_CREAT,0666);
		if (fd1==-1) {
			perror("writer: /pipe/1");
			_exit(1);
		}
		release_sem(sem1);
		/* wait for other side to open the pipe 1 */
		acquire_sem(sem2);
		/* wait for creation of pipe 2 */
		acquire_sem(sem1);
		fd2=open("/pipe/2",O_RDONLY);
		if (fd2==-1) {
			perror("/pipe/2");
			_exit(1);
		}
		release_sem(sem2);
		dup2(fd2,0);
		dup2(fd1,1);
		system(argv[2]);

		_exit(1);
	}
	acquire_sem(sem1);
	fd1=open("/pipe/1",O_RDONLY);
	release_sem(sem2);
	if (fd1==-1) {
		perror("/pipe/1");
		exit(1);
	}
	fd2=open("/pipe/2",O_WRONLY|O_CREAT,0666);
	if (fd2==-1) {
		perror("writer: /pipe/2");
		exit(1);
	}
	release_sem(sem1);
	/* wait for child to open ... */
	acquire_sem(sem2);
	dup2(fd1,0);
	dup2(fd2,1);
	system(argv[1]);
	exit(0);
}
