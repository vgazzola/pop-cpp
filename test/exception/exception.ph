#ifndef _CLASSEXCEP_PH
#define _CLASSEXCEP_PH

//#include "popc.h"

parclass ClassExcep
{

classuid(1001);

public:
  ClassExcep() @{ od.node(1); od.executable("./exception.obj"); };

  ~ClassExcep();

  sync seq   void SeqSync(int i);
  sync conc  void ConcSync(int i);
  sync mutex void MutexSync(int i);
  
private:
  int e;
};
#endif
