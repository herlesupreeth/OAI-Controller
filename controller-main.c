#include "controller_agent.h"
#include "log.h"

int main( int argc, char **argv )
{
	logInit();
	controller_agent_start(0);
}