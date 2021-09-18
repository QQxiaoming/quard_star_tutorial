#include <trace-cmd/trace-cmd.h>

int main()
{
	tracecmd_open_head("trace.dat", 0);
	return 0;
}
