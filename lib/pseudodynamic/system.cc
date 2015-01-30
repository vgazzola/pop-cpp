/**
 *
 * Copyright (c) 2005-2012 POP-C++ project - GRID & Cloud Computing group, University of Applied Sciences of western Switzerland.
 * http://gridgroup.hefr.ch/popc
 *
 * @author Tuan Anh Nguyen
 * @date 2005/01/01
 * @brief System stuffs and declarations used by the runtime.
 *
 *
 * Modifications :
 * Authors    Date      Comment
 * L.Winkler  2008-2009   for version 1.3
 * P.Kuonen   02/2010     (GetHost, getIp, add POPC_Host_Name, ...) for version 1.3m (see comments 1.3m)
 * P.Kuonen   02/2011      define default IP for version 1.3.1m (see comments 1.3.1m)
 * P.Kuonen   25/3/2011    Cosmetic changes
 */

/*
  Deeply need refactoring:
    POPC_System instead of paroc_system
 */
#include "popc_intface.h"

//#include <stdio.h>
//#include <netdb.h>
//#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <ctype.h>


#include "paroc_system_mpi.h"
#include "paroc_buffer_factory_finder.h"
#include "paroc_utils.h"
#include "paroc_combox_factory.h"
#include "paroc_exception.h"
#include "paroc_od.h"

paroc_accesspoint paroc_system::appservice;
paroc_accesspoint paroc_system::jobservice;
int paroc_system::pop_current_local_address;

// paroc_accesspoint paroc_system::popcloner;
int paroc_system::popc_local_mpi_communicator_rank;

int paroc_system_mpi::current_free_process;
bool paroc_system_mpi::is_remote_object_process;
bool paroc_system_mpi::mpi_has_to_take_lock;
paroc_condition paroc_system_mpi::mpi_unlock_wait_cond;
paroc_condition paroc_system_mpi::mpi_go_wait_cond;


POPString paroc_system::platform;
std::ostringstream paroc_system::_popc_cout;

//V1.3m
POPString paroc_system::POPC_HostName;
#define LOCALHOST "localhost"
//End modif

const char *paroc_system::paroc_errstr[17]= {
    "Out of resource",                           // 0
    "Fail to bind to the remote object broker",  // 1
    "Mismatch remote method id",                 // 2
    "Can not access code service",               // 3
    "Object allocation failed",                  // 4
    "No parallel object executable",             // 5
    "Bad paroc package format",                  // 6
    "Local application service failed",          // 7
    "Job Manager service failed",                // 8
    "Execution of object code failed",           // 9
    "Bad binding reply",                         // 10
    "No support protocol",                       // 11
    "No support data encoding",                  // 12
    "Standard exception",                        // 13
    "Acknowledgement not received",              // 14
    "Network configuration error",               // 15
    "Unknown exception"                          // 16
};


AppCoreService *paroc_system::mgr=NULL;
POPString paroc_system::challenge;

paroc_system::paroc_system() {
    paroc_combox_factory::GetInstance();
    paroc_buffer_factory_finder::GetInstance();
    char *tmp=getenv("POPC_PLATFORM");
    if(tmp!=NULL) {
        platform=tmp;
    } else {
        char str[128];
#ifndef POPC_ARCH
        char arch[64], sysname[64];
#ifndef __WIN32__
        //sysinfo(SI_SYSNAME,sysname,64);
        //sysinfo(SI_ARCHITECTURE,arch,64);
#endif
        sprintf(str,"%s-%s",sysname,arch);
#else
#ifndef __WIN32__
        strcpy(str,POPC_ARCH);
#else
        strcpy(str, "win32");
#endif
#endif
        platform=str;
    }
    POPC_HostName = POPString("");
}


paroc_system::~paroc_system() {
#ifndef DEFINE_UDS_SUPPORT
    /*if (mgr!=NULL)
    {
      Finalize(false);
      delete mgr;
    }
    mgr=NULL;*/
#endif

    paroc_combox_factory *pf=paroc_combox_factory::GetInstance();
    paroc_buffer_factory_finder *bf=paroc_buffer_factory_finder::GetInstance();
    pf->Destroy();
    delete bf;
}


void paroc_system::perror(const char *msg) {
    LOG_ERROR("paroc_system::perror : %d",errno);
    if(errno>USER_DEFINE_ERROR && errno<=USER_DEFINE_LASTERROR) {
        if(msg==NULL) {
            msg="POP-C++ Error";
        }
        fprintf(stderr,"%s: %s (errno %d)\n",msg,paroc_errstr[errno-USER_DEFINE_ERROR-1],errno);
        LOG_ERROR("%s: %s (errno %d)",msg,paroc_errstr[errno-USER_DEFINE_ERROR-1],errno);
    } else if(errno>USER_DEFINE_LASTERROR) {
        fprintf(stderr,"%s: Unknown error (errno %d)\n",msg, errno);
    } else {
        ::perror(msg);
    }
}

void paroc_system::perror(const paroc_exception *e) {
    errno=e->Code();
    paroc_system::perror((const char*)e->Extra());
}

// V1.3m
// Try to determine the Host Name of the machine and put it in POPC_Host_Name
// IF env. variable POPC_HOST is defined THEN use POPC_HOST
// ELSE IF try to use gethostname() and possibly env. variable POPC_DOMAIN
// ELSE call GetIP()
//----------------------------------------------------------------------------
POPString paroc_system::GetHost() {
    if(POPC_HostName.Length()<1) {
        char str[128];
        char *t=getenv("POPC_HOST");
        if(t==NULL || *t==0) {
            popc_gethostname(str,127);
            if(strchr(str,'.')==NULL || strstr(str,".local\0")!=NULL) {
                int len=strlen(str);
                char *domain=getenv("POPC_DOMAIN");
                if(domain!=NULL && domain!=0) {
                    str[len]='.';
                    strcpy(str+len+1,domain);
                    POPC_HostName = str;
                } else { //(domain!=NULL && domain!=0)
                    POPC_HostName = GetIP();
                }
            } else { //(strchr(str,'.')==NULL || strstr(str,".local\0")!=NULL)
                POPC_HostName = str;
            }
        } else { //(t==NULL || *t==0)
            POPC_HostName=t;
        }
    }
    //printf("GetHost returns %s\n", (const char*)POPC_HostName);
    return POPC_HostName;
}


// V1.3m
// Try to determine the IP address of the machine.
// If fail return the localhost IP = LOCALHOST
//------------------------------------------------------
POPString paroc_system::GetIP() {
#ifndef __WIN32__
    POPString iface,ip;
    char* tmp;
    ip = POPString(LOCALHOST);

    tmp=getenv("POPC_IP");
    if(tmp!=NULL) {     // Env. variable POPC_IP is defined
        ip=tmp;
    } else {           //Env. variable POPC_IP is not defined
        tmp=getenv("POPC_IFACE");
        if(tmp!=NULL) { // Env. Variable POP_IFACE is defined
            iface=tmp;    // Try to determine IP from network interface name
            if(!(GetIPFromInterface(iface,ip))) {
                // Not found
                setenv("POPC_IP",LOCALHOST, 0); // V1.3.1m define LOCALHOST as IP
                printf("POP-C++ Warning: Cannot find an IP for interface %s, using '%s' as IP address.\n",(const char*)iface, LOCALHOST);
            }
        } else { // Env. Variable POP_IFACE is not defined
            iface=GetDefaultInterface();
            if(iface.Length()>0) {
                if(!(GetIPFromInterface(iface,ip))) {
                    setenv("POPC_IP",LOCALHOST, 0); // V1.3.1m define LOCALHOST as IP
                    printf("POP-C++ Warning: host IP not found, using '%s' as IP address.\n",LOCALHOST);
                }
            } else {
                setenv("POPC_IP",LOCALHOST, 0); // V1.3.1m define LOCALHOST as IP
                printf("POP-C++ Warning: no default network interface found, using '%s' as IP address.\n", LOCALHOST);
            }
        }
    }
    return ip;
#else
    char ipaddr[64];
    char hostname[256];
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(popc_gethostname(hostname,256)!=0) {
        return paroc_string("127.0.0.1");
    }
    struct hostent *hp=gethostbyname(hostname);
    unsigned ip=(unsigned(127)<<24)+1;

    if(hp==NULL || *(hp->h_addr_list)==NULL) {
        return paroc_string("127.0.0.1");
    }

    char **p=hp->h_addr_list;
    while(*p!=NULL) {
        memcpy(&ip,*p, sizeof(unsigned));
        ip=ntohl(ip);
        if(ip!=(unsigned(127)<<24)+1) {
            break;
        }
        p++;
    }
    int n1,n2,n3,n4;
    n1=ip & 255;
    ip=ip>>8;

    n2=ip & 255;
    ip=ip>>8;

    n3=ip & 255;
    ip=ip>>8;

    n4=ip & 255;
    sprintf(ipaddr,"%d.%d.%d.%d",n4,n3,n2,n1);
    WSACleanup();
    return ipaddr;
#endif
}

int paroc_system::GetIP(const char *hostname, int *iplist, int listsize) {
    /* This method should normally not be used to return more than one ip*/
    assert(listsize==1);
#ifdef __WIN32__
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    struct hostent *hp;
    int n;
    char **p;
    int addr;
    if((int)(addr = popc_inet_addr(hostname)) == -1) {
        hp=gethostbyname(hostname);
    } else {
        if(listsize>0) {
            *iplist=popc_ntohl(addr);
            return 1;
        }
        return 0;
    }
    if(hp==NULL) {
        return 0;
    }
    n=0;
    for(p = hp->h_addr_list; *p != 0 && n<listsize; p++) {
        memcpy(iplist, *p, sizeof(int));
        *iplist=popc_ntohl(*iplist);
        if(*iplist==(int(127)<<24)+1) {
            continue;
        }
        n++;
        iplist++;
    }
    popc_endhostent();
    if(n==0) {
        n=1;
    }
#ifdef __WIN32__
    WSACleanup();
#endif
    return n;
}


int paroc_system::GetIP(int *iplist, int listsize) {
    assert(listsize==1); /* This method cannot return more than one ip*/
    char* parocip=popc_strdup((const char*)GetIP());
    int n;
    int addr;
    char *tmp;
    char *tok=popc_strtok_r(parocip," \n\r\t",&tmp);
    n=0;
    while(tok!=NULL && n<listsize) {
        if((int)(addr = popc_inet_addr(tok)) != -1) {
            *iplist=popc_ntohl(addr);
            iplist++;
            n++;
        }
        tok=popc_strtok_r(NULL," \n\r\t",&tmp);
    }
    return n;
}

POPString paroc_system::GetDefaultInterface() {
    char buff[1024], iface[16], net_addr[128];
    //char flags[64], mask_addr[128], gate_addr[128];
    //int iflags, metric, refcnt, use, mss, window, irtt;
    bool found=false;
    FILE *fp = fopen("/proc/net/route", "r");

    if(fp != NULL) { //else
        while(fgets(buff, 1023, fp) && !found) {
            //num = sscanf(buff, "%16s %128s %128s %X %d %d %d %128s %d %d %d",
            //       iface, net_addr, gate_addr, &iflags, &refcnt, &use, &metric, mask_addr, &mss, &window, &irtt);
            int num = sscanf(buff, "%16s %128s",iface, net_addr);
            if(num < 2) {
                paroc_exception::paroc_throw_errno();
            }
            // LOG_DEBUG("iface %s, net_addr %s, gate_addr %s, iflags %X, &refcnt %d, &use %d, &metric %d, mask_addr %s, &mss %d, &window %d, &irtt %d\n\n",iface, net_addr, gate_addr,iflags, refcnt, use, metric, mask_addr, mss, window, irtt);

            if(!strcmp(net_addr,"00000000")) {
                LOG_DEBUG("Default gateway : %s", iface);
                found=true;
            }
        }
        fclose(fp);
    }
    if(!found) {
        iface[0] = '\0';    // if not found iface = ""
    }
    return POPString(iface);
}

bool paroc_system::GetIPFromInterface(POPString &iface, POPString &str_ip) {
#ifndef __WIN32__
    (void) iface;
    (void) str_ip;
    /*struct ifaddrs *addrs, *iap;
    struct sockaddr_in *sa;
    char str_ip_local[32];

    getifaddrs(&addrs);

    //printf("\nLooking for interface: %s --->\n",(const char*)iface);
    for (iap = addrs; iap != NULL; iap = iap->ifa_next){
      //printf("name=%s, addr=%p, flag=%d (%d), familly=%d (%d)\n",iap->ifa_name, iap->ifa_addr, iap->ifa_flags, IFF_UP, iap->ifa_addr->sa_family, AF_INET);
      if ( iap->ifa_addr &&
           (iap->ifa_flags & IFF_UP) &&
           (iap->ifa_addr->sa_family == AF_INET) &&
           !strcmp(iap->ifa_name, (const char*)iface)) {
        sa = (struct sockaddr_in *)(iap->ifa_addr);
        inet_ntop(iap->ifa_addr->sa_family,
                  (void *)&(sa->sin_addr),
                  str_ip_local,
                  sizeof(str_ip_local) );
        //LOG_DEBUG("The IP of interface %s is %s",iap->ifa_name,str_ip_local);
        //printf("The IP of interface %s is %s\n",iap->ifa_name,str_ip_local);
        str_ip=str_ip_local;
        freeifaddrs(addrs);
        return true;
      }
    }
    freeifaddrs(addrs);*/
    return false;
#else
    (void) iface;
    (void) str_ip;
#endif
}


/**
 * Initialize the base system to run a POP-C++ application
 * @param   argc    Number of arguments (passed from the main)
 * @param   argv    Arguments (passed from the main)
 * @return  TRUE if the system is initialized. FALSE in any others cases.
 */
bool paroc_system::Initialize(int *argc,char ***argv) {
    // Get access point address of the Job Manager
    char *info = paroc_utils::checkremove(argc, argv, "-jobservice=");
    if(info == NULL) {
        printf("Error: missing -jobservice argument\n");
        return false;
    }
    paroc_system::jobservice.SetAccessString(info);
    paroc_system::jobservice.SetAsService();

    // Get path of the application service executable
    char *codeser=paroc_utils::checkremove(argc,argv,"-appservicecode=");
    //char *proxy=paroc_utils::checkremove(argc,argv,"-proxy=");

    // Check if need to run on local node only
    if(paroc_utils::checkremove(argc,argv,"-runlocal")) {
        paroc_od::defaultLocalJob=true;
    }

    // Get application service contact address
    char *appcontact = paroc_utils::checkremove(argc,argv,"-appservicecontact=");

    if(codeser==NULL && appcontact==NULL) {
        printf("Error: missing -appservicecontact=... or -appservicecode=... argument\n");
        return false;
    }
    try {
        if(appcontact == NULL) {
            char url[1024];
            strcpy(url, codeser);

#ifndef DEFINE_UDS_SUPPORT
            // Creating application services
            //mgr = CreateAppCoreService(url);
            //printf("POP-C++ Application Service created %s\n", mgr->GetAccessPoint().GetAccessString());
#endif
        } else {
            challenge=NULL;
            paroc_accesspoint app;
            app.SetAccessString(appcontact);
            app.SetAsService();
#ifndef DEFINE_UDS_SUPPORT
           //mgr = new AppCoreService(app);
#endif
        }
        //paroc_system::appservice=mgr->GetAccessPoint();
        paroc_system::appservice.SetAsService();
    } catch(POPException *e) {
        printf("POP-C++ Exception occurs in paroc_system::Initialize\n");
        POPSystem::perror(e);
        delete e;
#ifndef DEFINE_UDS_SUPPORT
        /*if (mgr!=NULL) {
            mgr->KillAll();
            mgr->Stop(challenge);
            delete mgr;
            mgr=NULL;
        }*/
#endif
        return false;
    } catch(...) {
 #ifndef DEFINE_UDS_SUPPORT
       printf("Exception occurs in paroc_system::Initialize\n");
       /*if (mgr!=NULL) {
            mgr->KillAll();
            mgr->Stop(challenge);
            delete mgr;
            mgr=NULL;
        }*/
#endif
        return false;
    }

    char *codeconf = paroc_utils::checkremove(argc,argv,"-codeconf=");
    (void) codeconf; // Added this to avoid warning

// DEBUGIF(codeconf==NULL,"No code config file\n");

    /*if (codeconf!=NULL && !paroc_utils::InitCodeService(codeconf,mgr))
    {
    return false;
    }
    else return true; */

    //printf("SYSTEM: Init code service\n");
    //bool ret = !(codeconf != NULL && !paroc_utils::InitCodeService(codeconf,mgr));
    //printf("SYSTEM: Init code service done\n");
    return false;
}

void paroc_system::Finalize(bool /*normalExit*/) {
#ifndef DEFINE_UDS_SUPPORT
    //printf("Finalize the application %s\n", normalExit ? "true" : "false");
    /*  if (mgr != NULL) {
        //printf("mgr not null\n");
        try {
            if(normalExit) {
            // Wait all object to be terminated before exiting
            int timeout = 1;
            int oldcount = 0, count;
            int loop = 0;
            while ((count = mgr->CheckObjects()) > 0){
              if (timeout < 1800 && oldcount == count){
                timeout = timeout * 4/3;
                loop++;
                if (loop % 10 == 0) {
                  timeout += 1;
                }
              } else {
                loop = 0;
                timeout = 1;
              }
                    popc_sleep(timeout);
              oldcount = count;
            }
          } else {
            //printf("Finalize killall\n");
            mgr->KillAll();
          }
          //printf("Finalize stop\n");
          mgr->Stop(challenge);
          delete mgr;
        } catch (paroc_exception *e) {
          paroc_system::perror(e->Extra());
          delete e;
        } catch (...) {
          fprintf(stderr,"POP-C++ error on finalizing the application\n");
        }
        mgr = NULL;
      }*/
    //printf("Finalize the application end\n");
#endif
}



/*AppCoreService *paroc_system::CreateAppCoreService(char *codelocation)
{
    // Create challenge string
    srand(time(NULL));
    char tmp[256];

    for(int i=0; i<255; i++) {
        tmp[i]=(char)(1+254.0*rand()/(RAND_MAX+1.0) );
    }
    tmp[255]='\0';
    challenge=tmp;

    return new AppCoreService(challenge, false, codelocation);
}*/


void paroc_system::processor_set(int /*cpu*/) {
#ifndef __APPLE__
    // Use glibc to set cpu affinity
    /*if (cpu < 0) {
        printf("POP-C++ Warning: Cannot set processor to %d<0", cpu);
        exit(EXIT_FAILURE);
    }
    if(cpu >= CPU_SETSIZE) {
        printf("POP-C++ Warning: Cannot set processor to %d while CPU_SETSIZE=%d", cpu, CPU_SETSIZE);
        exit(EXIT_FAILURE);
    }

    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(cpu, &cpu_set);
    if(sched_setaffinity(0, sizeof(cpu_set), &cpu_set) == -1) {
        printf("POP-C++ Warning: Cannot set processor to %d (cpu_set %p)", cpu,(void *)&cpu_set);
        exit(EXIT_FAILURE);
    }

    cpu_set_t cpu_get;
    CPU_ZERO(&cpu_get);
    if(sched_getaffinity(0, sizeof(cpu_get), &cpu_get) == -1) {
        printf("POP-C++ Warning: Unable to sched_getaffinity to (cpu_get) %p", (void *)&cpu_get);
        exit(EXIT_FAILURE);
    }

    if(memcmp(&cpu_get, &cpu_set, sizeof(cpu_set_t))) {
        printf("POP-C++ Warning: Unable to run on cpu %d", cpu);
        exit(EXIT_FAILURE);
    }
    #else
    // Apple thread API
    */

#endif
}
