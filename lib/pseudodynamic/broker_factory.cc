/**
 *
 * Copyright (c) 2005-2012 POP-C++ project - GRID & Cloud Computing group, University of Applied Sciences of western Switzerland.
 * http://gridgroup.hefr.ch/popc
 *
 * @author Tuan Anh Nguyen
 * @date 2005/01/01
 * @brief Implementation of broker factory.
 *
 *
 * Modifications :
 * Authors      Date            Comment
 * clementval  10/27/2011  Set services accesspoint cretaed from String as service accesspoint
 * P.Kuonen     18.9.2012       Add "POP-C++ error" in error messages (PEKA)
 */

/*
  Deeply need refactoring:
    POPC_BrokerFactory instead of paroc_broker_factory
 */

// #include <mpi.h>
#include "popc_intface.h"

//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>

#include "paroc_broker_factory.h"
#include "paroc_utils.h"
#include "paroc_system.h"

paroc_list_broker *paroc_broker_factory::brokerlist=NULL;
ispackedfunc paroc_broker_factory::CheckIfPacked=NULL;

paroc_broker_factory::paroc_broker_factory(initbrokerfunc func, const char *name) {
    if(brokerlist==NULL) {
        brokerlist= new paroc_list_broker;
    }
    if(name==NULL || func==NULL) {
        return;
    }
    if(test(name)) {
        return;
    }
    paroc_broker_init &t=brokerlist->AddTailNew();
    t.func=func;
    t.objname=name;
}

paroc_broker *paroc_broker_factory::Create(const char *objname) {
    // DEBUG("Create broker for %s\n", objname);
    if(brokerlist==NULL || objname==NULL) {
        return NULL;
    }

    POSITION pos=brokerlist->GetHeadPosition();
    while(pos!=NULL) {
        paroc_broker_init &t=brokerlist->GetNext(pos);
        if(paroc_utils::isEqual(objname, t.objname)) {
            return t.func();
        }
    }
    return NULL;
}

void paroc_broker_factory::List(paroc_list_string &objlist) {
    if(brokerlist==NULL) {
        return;
    }
    POSITION pos=brokerlist->GetHeadPosition();
    while(pos!=NULL) {
        paroc_broker_init &t=brokerlist->GetNext(pos);
        POPString &str=objlist.AddTailNew();
        str=t.objname;
    }
}

bool paroc_broker_factory::test(const char *objname) {
    if(brokerlist==NULL) {
        return false;
    }
    POSITION pos=brokerlist->GetHeadPosition();
    while(pos!=NULL) {
        paroc_broker_init &test=brokerlist->GetNext(pos);
        if(paroc_utils::isEqual(objname, test.objname)) {
            return true;
        }
    }
    return false;
}


paroc_broker * paroc_broker_factory::Create(int *argc, char ***argv) {
    /**
     * Display the information about the parallel object executable
     * note: this bit of code existed in pseudodynamic version and is
     * kept just in case.
     */
/*
    char *tmp1 = paroc_utils::checkremove(argc,argv,"-printmpi");
    if(tmp1 != NULL) {
        char abspath[1024];
        char *thisfile = getenv("POPC_EXE");
        if(thisfile == NULL) {
            thisfile = (*argv)[0];
        }
        paroc_utils::FindAbsolutePath(thisfile,abspath);
        PrintBrokersMPI(abspath);
        exit(0);
    }
*/
    char *tmp = paroc_utils::checkremove(argc,argv,"-list");
    if(tmp!=NULL) {
        char abspath[1024];
        char *thisfile=getenv("POPC_EXE");
        if(thisfile==NULL) {
            thisfile=(*argv)[0];
        }
        paroc_utils::FindAbsolutePath(thisfile,abspath);
        PrintBrokers(abspath, strcmp(tmp,"long")==0);
        exit(0);
    }

    char *usage=paroc_utils::checkremove(argc,argv,"-help");
    char *object = paroc_utils::checkremove(argc,argv,"-object=");
    if(usage != NULL || object == NULL) {
        printf("\n Usage:\n\t%s [-help] [-list | -listlong] [-port=<local port>] [-callback=<host:port>] [-appservice=<host:port>] [-nostdio] -object=<objectname>\n",(*argv)[0]);
        return NULL;
    }


    tmp = paroc_utils::checkremove(argc,argv,"-appservice=");
    if(tmp != NULL) {
        paroc_system::appservice.SetAccessString(tmp);
        paroc_system::appservice.SetAsService();  //Set the accesspoint as a service accesspoint
    }

    /*#ifndef DEFINE_UDS_SUPPORT
        if ((tmp=getenv("POPC_JOBSERVICE"))!=NULL) {
        paroc_system::jobservice.SetAccessString(tmp);
    paroc_system::jobservice.SetAsService();  //Set the accesspoint as a service accesspoint
    } else if ((tmp = paroc_utils::checkremove(argc, argv, "-jobservice=")) != NULL) {
        paroc_system::jobservice.SetAccessString(tmp);
    paroc_system::jobservice.SetAsService();  //Set the accesspoint as a service accesspoint
    } else {
        char tmpstr[256];
        DEBUG("Jobservice is not specified. Use the default one!");
        sprintf(tmpstr, "%s:%d", (const char *)paroc_system::GetHost(), DEFAULTPORT);
        paroc_system::jobservice.SetAccessString(tmpstr);
    paroc_system::jobservice.SetAsService();  //Set the accesspoint as a service accesspoint
        }
    #endif*/

    bool nostdio = (paroc_utils::checkremove(argc, argv, "-nostdio") != NULL);

    //Now create the broker
    if(object == NULL || *object == 0) {
        return NULL;
    }

    // Create the real Broker object
    paroc_broker *objbroker = Create(object);
    if(objbroker == NULL) {
        printf("POP-C++ Error: %s: unkown object name\n", (*argv)[1]);
        return NULL;
    }

    // Set the classname for this broker
    paroc_broker::classname = object;

    if(nostdio) {
        popc_close(0);
        popc_close(1);
        popc_close(2);

        popc_open("/dev/null",O_RDONLY);
#ifndef NDEBUG
        char fname[256];
        sprintf(fname,"/tmp/object_%s_%d.log", (const char *)paroc_broker::classname,popc_getpid());
        popc_open(fname,O_WRONLY | O_CREAT,S_IRWXU | S_IRGRP);
        popc_dup2(1,2);
#else
        popc_open("/dev/null",O_WRONLY);
        popc_open("/dev/null",O_WRONLY);
#endif
    }

    return objbroker;
}


void paroc_broker_factory::PrintBrokers(const char *abspath, bool longformat) {
    if(!longformat) {
        printf("List of parallel object classes:\n====\n");
    }
    if(brokerlist!=NULL) {
        POSITION pos=brokerlist->GetHeadPosition();
        while(pos!=NULL) {
            paroc_broker_init &t=brokerlist->GetNext(pos);
            if(!(paroc_broker_factory::CheckIfPacked!=NULL && !paroc_broker_factory::CheckIfPacked(t.objname))) {
                if(longformat) {
                    printf("%s %s %s\n",(const char *)t.objname, (const char *)paroc_system::platform, abspath);
                } else {
                    printf("%s\n",(const char *)t.objname);
                }
            }
        }
    }
    if(!longformat) {
        printf("====\nArchitecture=%s\n",(const char *)paroc_system::platform);
    }
}

/* note: this coded existed in the pseudodynamic version of the code but is now disabled.
void paroc_broker_factory::PrintBrokersMPI(const char *abspath) {
    if(brokerlist!=NULL) {
        POSITION pos=brokerlist->GetHeadPosition();
        while(pos!=NULL) {
            paroc_broker_init &t=brokerlist->GetNext(pos);
            if(!(paroc_broker_factory::CheckIfPacked!=NULL && !paroc_broker_factory::CheckIfPacked(t.objname))) {
                printf("-host localhost -np 1 %s -mpi -object=%s\n", abspath, (const char *)t.objname);
            }
        }
    }
}

*/