#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	close(1);
	execlp("ls", "ls", NULL);
	return 0;
}
