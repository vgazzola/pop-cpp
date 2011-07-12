#include <stdio.h>
#include "remotelog.ph"

RemoteLog::RemoteLog(const paroc_string &challenge): paroc_service_base(challenge)
{
}
RemoteLog::~RemoteLog()
{
}
	
void RemoteLog::Log(const paroc_string &info)
{
  fprintf(stdout,info);
  fflush(stdout);
}