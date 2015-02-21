/**
 *
 * Copyright (c) 2005-2012 POP-C++ project - GRID & Cloud Computing group, University of Applied Sciences of western Switzerland.
 * http://gridgroup.hefr.ch/popc
 *
 * @author Tuan Anh Nguyen
 * @date 2005/01/01
 * @brief Implementation of SUN-XDR message buffer.
 *
 *
 */

/*
  Deeply need refactoring:
    POPC_BufferXDR instead of paroc_buffer_xdr
 */
#include "popc_intface.h"

#include <rpc/types.h>
#include <rpc/xdr.h>
// #include "paroc_interface.h"
#include "paroc_buffer_xdr.h"
#include "paroc_exception.h"
#include "popc_logger.h"

paroc_buffer_xdr::paroc_buffer_xdr() {
    Reset();
}

paroc_buffer_xdr::~paroc_buffer_xdr() {}

void paroc_buffer_xdr::Reset() {
    unpackpos=20;
    packeddata.RemoveAll();
    packeddata.resize(unpackpos);
}

void paroc_buffer_xdr::Pack(const int *data, int n) {
    if(n<=0) {
        return;
    }
    int oldsize=packeddata.size();
    packeddata.resize(n*4+oldsize);
    char *dest=packeddata.data()+oldsize;

    XDR xdr;
    xdrmem_create(&xdr,dest,n*4,XDR_ENCODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(int),(xdrproc_t)xdr_int);
    xdr_destroy(&xdr);

}
void paroc_buffer_xdr::UnPack(int *data, int n) {
    if(n<=0) {
        return;
    }
    char *dest=packeddata.data()+unpackpos;

    int sz=4*n;

    CheckUnPack(sz);

    XDR xdr;
    xdrmem_create(&xdr,dest,sz,XDR_DECODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(int),(xdrproc_t)xdr_int);
    xdr_destroy(&xdr);

    unpackpos+=sz;
}

void paroc_buffer_xdr::Pack(const unsigned *data, int n) {
    if(n<=0) {
        return;
    }
    int oldsize=packeddata.size();
    packeddata.resize(n*4+oldsize);
    char *dest=packeddata.data()+oldsize;

    XDR xdr;
    xdrmem_create(&xdr,dest,n*4,XDR_ENCODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(int),(xdrproc_t)xdr_u_int);
    xdr_destroy(&xdr);

}
void paroc_buffer_xdr::UnPack(unsigned *data, int n) {
    if(n<=0) {
        return;
    }

    int sz=4*n;
    CheckUnPack(sz);

    char *dest=packeddata.data()+unpackpos;
    XDR xdr;
    xdrmem_create(&xdr,dest,sz,XDR_DECODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(unsigned),(xdrproc_t)xdr_u_int);
    xdr_destroy(&xdr);

    unpackpos+=sz;
}

void paroc_buffer_xdr::Pack(const long *data, int n) {
    if(n<=0) {
        return;
    }
    int oldsize=packeddata.size();
    packeddata.resize(n*4+oldsize);
    char *dest=packeddata.data()+oldsize;

    XDR xdr;
    xdrmem_create(&xdr,dest,n*4,XDR_ENCODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(long),(xdrproc_t)xdr_long);
    xdr_destroy(&xdr);
}

void paroc_buffer_xdr::UnPack(long *data, int n) {
    if(n<=0) {
        return;
    }
    char *dest=packeddata.data()+unpackpos;

    int sz=4*n;
    CheckUnPack(sz);

    XDR xdr;
    xdrmem_create(&xdr,dest,sz,XDR_DECODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(long),(xdrproc_t)xdr_long);
    xdr_destroy(&xdr);

    unpackpos+=sz;
}

void paroc_buffer_xdr::Pack(const unsigned long *data, int n) {
    if(n<=0) {
        return;
    }
    int oldsize=packeddata.size();
    packeddata.resize(n*4+oldsize);
    char *dest=packeddata.data()+oldsize;

    XDR xdr;
    xdrmem_create(&xdr,dest,n*4,XDR_ENCODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(long),(xdrproc_t)xdr_u_long);
    xdr_destroy(&xdr);
}

void paroc_buffer_xdr::UnPack(unsigned long *data, int n) {
    if(n<=0) {
        return;
    }
    char *dest=packeddata.data()+unpackpos;

    int sz=n*4;
    CheckUnPack(sz);

    XDR xdr;
    xdrmem_create(&xdr,dest,sz,XDR_DECODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(long),(xdrproc_t)xdr_u_long);
    xdr_destroy(&xdr);

    unpackpos+=sz;
}

void paroc_buffer_xdr::Pack(const short *data, int n) {
    if(n<=0) {
        return;
    }
    int oldsize=packeddata.size();
    packeddata.resize(((n-1)/2+1)*4+oldsize);
    char *dest=packeddata.data()+oldsize;

    XDR xdr;
    xdrmem_create(&xdr,dest,n*2,XDR_ENCODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(short),(xdrproc_t)xdr_short);
    xdr_destroy(&xdr);

}

void paroc_buffer_xdr::UnPack(short *data, int n) {
    if(n<=0) {
        return;
    }
    char *dest=packeddata.data()+unpackpos;

    int sz=2*n;
    CheckUnPack(sz);
    XDR xdr;
    xdrmem_create(&xdr,dest,sz,XDR_DECODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(short),(xdrproc_t)xdr_short);
    xdr_destroy(&xdr);

    unpackpos+=((n-1)/2+1)*4;

}

void paroc_buffer_xdr::Pack(const unsigned short *data, int n) {
    if(n<=0) {
        return;
    }
    int oldsize=packeddata.size();
    packeddata.resize(((n-1)/2+1)*4+oldsize);
    char *dest=packeddata.data()+oldsize;

    XDR xdr;
    xdrmem_create(&xdr,dest,n*2,XDR_ENCODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(short),(xdrproc_t)xdr_u_short);
    xdr_destroy(&xdr);
}

void paroc_buffer_xdr::UnPack(unsigned short *data, int n) {
    if(n<=0) {
        return;
    }
    char *dest=packeddata.data()+unpackpos;

    int sz=2*n;
    CheckUnPack(sz);

    XDR xdr;
    xdrmem_create(&xdr,dest,sz,XDR_DECODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(short),(xdrproc_t)xdr_u_short);
    xdr_destroy(&xdr);

    unpackpos+=((n-1)/2+1)*4;
}

void paroc_buffer_xdr::Pack(const bool *data, int n) {
    if(n<=0) {
        return;
    }
    int t=packeddata.size();
    packeddata.resize(t+((n-1)/4+1)*4);
    char *dat=packeddata.data()+t;
    while(n-->0) {
        *dat=(*data==true);
        dat++;
        data++;
    }
}
void paroc_buffer_xdr::UnPack(bool *data, int n) {
    if(n<=0) {
        return;
    }
    CheckUnPack(n);
    packeddata.size();

    char *dat = packeddata.data() + unpackpos;
    while(n-->0) {
        *data=(*dat!=0);
        dat++;
        data++;
    }
    unpackpos+=((n-1)/4+1)*4;
}

void paroc_buffer_xdr::Pack(const char *data, int n) {
    if(n<=0) {
        return;
    }
    int t=packeddata.size();
    packeddata.resize(t+((n-1)/4+1)*4);
    memcpy(packeddata.data()+t,data,n);
}

void paroc_buffer_xdr::UnPack(char *data, int n) {
    if(n<=0) {
        return;
    }
    CheckUnPack(n);
    packeddata.size();
    memcpy(data, (packeddata.data())+unpackpos,n);
    unpackpos+=((n-1)/4+1)*4;
}

void paroc_buffer_xdr::Pack(const unsigned char *data, int n) {
    Pack((char *)data,n);
}

void paroc_buffer_xdr::UnPack(unsigned char *data, int n) {
    UnPack((char *)data,n);
}

void paroc_buffer_xdr::Pack(const float *data, int n) {
    if(n<=0) {
        return;
    }
    int oldsize=packeddata.size();
    packeddata.resize(n*4+oldsize);
    char *dest=packeddata.data()+oldsize;

    XDR xdr;
    xdrmem_create(&xdr,dest,n*4,XDR_ENCODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(float),(xdrproc_t)xdr_float);
    xdr_destroy(&xdr);
}

void paroc_buffer_xdr::UnPack(float *data, int n) {
    if(n<=0) {
        return;
    }
    char *dest=packeddata.data()+unpackpos;

    int sz=n*4;
    CheckUnPack(sz);

    XDR xdr;
    xdrmem_create(&xdr,dest,sz,XDR_DECODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(float),(xdrproc_t)xdr_float);
    xdr_destroy(&xdr);

    unpackpos+=sz;

}

void paroc_buffer_xdr::Pack(const double *data, int n) {
    if(n<=0) {
        return;
    }
    int oldsize=packeddata.size();
    packeddata.resize(n*8+oldsize);
    char *dest=packeddata.data()+oldsize;

    XDR xdr;
    xdrmem_create(&xdr,dest,n*8,XDR_ENCODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(double),(xdrproc_t)xdr_double);
    xdr_destroy(&xdr);
}

void paroc_buffer_xdr::UnPack(double *data, int n) {
    if(n<=0) {
        return;
    }
    char *dest=packeddata.data()+unpackpos;

    int sz=8*n;
    CheckUnPack(sz);

    XDR xdr;
    xdrmem_create(&xdr,dest,sz,XDR_DECODE);
    xdr_vector(&xdr,(char *)data,n,sizeof(double),(xdrproc_t)xdr_double);
    xdr_destroy(&xdr);

    unpackpos+=sz;
}

void paroc_buffer_xdr::Pack(const signed char *data, int n) {
    Pack((char *)data,n);
}

void paroc_buffer_xdr::UnPack(signed char *data, int n) {
    UnPack((char *)data,n);
}

/*void paroc_buffer_xdr::Pack(const long long *data, int n)
{
  // This method has to be written
}

void paroc_buffer_xdr::UnPack(long long *data, int n)
{
  // This method has to be written
}

void paroc_buffer_xdr::Pack(const long double *data, int n)
{
  // This method has to be written
}

void paroc_buffer_xdr::UnPack(long double *data, int n)
{
  // This method has to be written
}*/



// void paroc_buffer_xdr::Pack(const paroc_accesspoint *list, int n)
// {
//   if (n<=0 || list==NULL) return;
//   int len;
//   for (int i=0;i<n;i++,list++)
//     {
//       const char *res=list->GetAccessString();
//       int len=(res==NULL)? 0 : (strlen(res)+1);
//       Pack(&len,1);
//       if (len) Pack(res,len);
//     }
// }

// void paroc_buffer_xdr::UnPack(paroc_accesspoint *list, int n)
// {
//   if (n<=0 || list==NULL) return;
//   paroc_array<char> tmpstr;
//   int len;
//   for (int i=0;i<n;i++,list++)
//     {
//       UnPack(&len,1);
//       if (len)
//  {
//    tmpstr.resize(len);
//    UnPack(tmpstr,len);
//    list->SetAccessString(tmpstr);
//  }
//       else list->SetAccessString(NULL);
//     }
// }

// void paroc_buffer_xdr::Pack(const POPString *list, int n)
// {
//   if (n<=0 || list==NULL) return;
//   for (int i=0;i<n;i++,list++)
//     {
//       const char *res=(*list);
//       int len=list->Length()+1;
//       Pack(&len,1);
//       if (len>0) Pack(res,len);
//     }
// }

// void paroc_buffer_xdr::UnPack(POPString *list, int n)
// {
//   if (n<=0 || list==NULL) return;
//   paroc_array<char> tmpstr;
//   int len;
//   for (int i=0;i<n;i++,list++)
//     {
//       UnPack(&len,1);
//       if (len>0)
//  {
//    tmpstr.resize(len);
//    UnPack(tmpstr,len);
//    (*list)=(char *)tmpstr;
//  }
//       else (*list)=NULL;
//     }
// }

// void paroc_buffer_xdr::Pack(const paroc_od *list, int n)
// {
//   float val[7];
//   POPString t;
//   while (n>0)
//     {
//       list->getPower(val[0],val[1]);
//       list->getMemory(val[2],val[3]);
//       list->getBandwidth(val[4],val[5]);
//       val[6]=list->getWallTime();
//       Pack(val,7);

//       list->getURL(t);
//       Pack(&t,1);

//       list->getJobURL(t);
//       Pack(&t,1);

//       list->getExecutable(t);
//       Pack(&t,1);

//       list->getProtocol(t);
//       Pack(&t,1);

//       list->getEncoding(t);
//       Pack(&t,1);

//       n--;
//       list++;
//     }

// }

// void paroc_buffer_xdr::UnPack(paroc_od *list, int n)
// {
//   float val[7];
//   POPString t;
//   while (n>0)
//     {
//       UnPack(val,7);
//       list->power(val[0],val[1]);
//       list->memory(val[2],val[3]);
//       list->bandwidth(val[4],val[5]);
//       list->walltime(val[6]);
//       UnPack(&t,1);
//       list->url(t);
//       UnPack(&t,1);
//       list->joburl(t);

//       UnPack(&t,1);
//       list->executable(t);

//       UnPack(&t,1);
//       list->protocol(t);

//       UnPack(&t,1);
//       list->encoding(t);
//       n--;
//       list++;
//     }
// }

// void paroc_buffer_xdr::Pack(paroc_interface *inf, int n)
// {
//    if (n<=0) return;
//   int len;
//   for (int i=0;i<n;i++,inf++)
//     {
//       int ref=inf->AddRef();
//       Pack(&(inf->GetAccessPoint()),1);
//       Pack(&ref,1);
//       const paroc_od &myod=inf->GetOD();
//       Pack(&myod,1);
//     }
// }

// void paroc_buffer_xdr::UnPack(paroc_interface *inf, int n)
// {
//   if (n<=0 || inf==NULL) return;
//   paroc_od myod;
//   paroc_accesspoint entry;
//   int len;
//   for (int i=0;i<n;i++,inf++)
//     {
//       int ref;
//       UnPack(&entry,1);
//       UnPack(&ref,1);
//       UnPack(&myod,1);

//       inf->SetOD(myod);

//       if (ref>0)
//  {
//    inf->Bind(entry);
//    inf->DecRef();
//  }
//     }
// }

// void paroc_buffer_xdr::Pack(paroc_exception *e, int n)
// {
//    if (n<=0) return;
//   for (int i=0;i<n;i++,e++)
//     {
//       int t=e->Code();
//       Pack(&t,1);
//       char *extra=e->Extra();
//       t=strlen(extra)+1;
//       Pack(&t,1);
//       Pack(extra,t);
//     }
// }

// void paroc_buffer_xdr::UnPack(paroc_exception *e, int n)
// {
//    if (n<=0) return;
//   for (int i=0;i<n;i++,e++)
//     {
//       int t;
//       UnPack(&t,1);
//       e->Code()=t;
//       char *extra=e->Extra();
//       UnPack(&t,1);
//       UnPack(extra,t);
//     }
// }


void paroc_buffer_xdr::CheckUnPack(int sz) {
    if(sz+unpackpos > packeddata.size()) {
        paroc_exception::paroc_throw(POPC_BUFFER_FORMAT, "Wrong buffer format in paroc_buffer_xdr::CheckUnPack");
    }
}

/**
 * Send the packed data to the matching combox
 * @param s
 * @param conn
 * @return
 */
bool paroc_buffer_xdr::Send(paroc_combox &s, paroc_connection *conn) {
    // Pack the header (20 bytes)
    char *dat=packeddata.data();

    if(dat == NULL) {
        LOG_ERROR("fail 1");
        return false;
    }

    int n = packeddata.size();
    int h[5];
    memset(h, 0, 5 * sizeof(int));

    int type = header.GetType();

    h[0] = popc_htonl(n);
    h[1] = popc_htonl(type);

    switch(type) {
    case TYPE_REQUEST:
        h[2] = popc_htonl(header.GetClassID());
        h[3] = popc_htonl(header.GetMethodID());
        h[4] = popc_htonl(header.GetSemantics());
        break;
    case TYPE_EXCEPTION:
        h[2] = popc_htonl(header.GetExceptionCode());
        break;
    case TYPE_RESPONSE:
        h[2] = popc_htonl(header.GetClassID());
        h[3] = popc_htonl(header.GetMethodID());
        break;
    default:
        return false;
    }

    memcpy(dat, h, 20);


    // Send the message header first as it has fixed size
    char* data_header = new char[20];
    memcpy(data_header, h, 20);
    // LOG_DEBUG("XDR: %s Send header", (isServer) ? "server":"client", n);
    if(s.Send(data_header, 20, conn)) {
        LOG_ERROR("Error while sending header");
        return false;
    }

    // If there are data to send, send them
    dat += 20;
    n -= 20;
    if(n > 0) {
        // LOG_DEBUG("XDR: %s Send message size is %d: %s", (isServer) ? "server":"client", n, (char*)packeddata);
        if(s.Send(dat, n, conn) < 0) {
            LOG_ERROR("XDR: Fail to send a message!");
            return false;
        }
    }



    return true;
}

/**
 *
 */
bool paroc_buffer_xdr::Recv(paroc_combox &s, paroc_connection *conn) {
    int h[5];
    int n;

    // Recv the header
    char *dat = (char *)h;


    // Receiving the real data
LOG_DEBUG("XDR: recv header");
    s.Recv(dat, 20, conn);
    Reset();
    /* for(int i = 0; i < 5; i++) {
       memcpy(&h[i], dat+(i*4), 4);
     }*/



    n = popc_ntohl(h[0]);
    if(n < 20) {
        LOG_ERROR("[CORE] XDR Buffer - Bad message header (size error:%d)", n);
        return false;
    }

    int type = popc_ntohl(h[1]);
    header.SetType(type);
    switch(type) {
    case TYPE_REQUEST:
        header.SetClassID(popc_ntohl(h[2]));
        header.SetMethodID(popc_ntohl(h[3]));
        header.SetSemantics(popc_ntohl(h[4]));
        break;
    case TYPE_EXCEPTION:
        header.SetExceptionCode(popc_ntohl(h[2]));
        break;
    case TYPE_RESPONSE:
        header.SetClassID(popc_ntohl(h[2]));
        header.SetMethodID(popc_ntohl(h[3]));
        break;
    default:
        LOG_ERROR("Unknown type %d", type);
        return false;
    }

    packeddata.resize(n);
    n-=20;

    if(n > 0) {
        dat = packeddata.data()+20;
        // LOG_DEBUG("XDR: %s ready to receive %d",(isServer) ? "server":"client",  n);
        s.Recv(dat, n, conn);
        // LOG_DEBUG("XDR: %s received %d",(isServer) ? "server":"client",  n);
    }



    return true;
}

int paroc_buffer_xdr::get_size() {
    return packeddata.size();
}

// Note LWK: This method was copied from the dynamic to the pseudodynamic version of the code
// TODO LWK: Check with Valentin if ok
char* paroc_buffer_xdr::get_load() {
    char *dat = packeddata.data();

    if(!dat) {
        return NULL;
    }

    int n = packeddata.size();
    int h[5];
    memset(h,0, 5*sizeof(int));

    int type=header.GetType();

    h[0]=popc_htonl(n);
    h[1]=popc_htonl(type);

    switch(type) {
    case TYPE_REQUEST:
        h[2]=popc_htonl(header.GetClassID());
        h[3]=popc_htonl(header.GetMethodID());
        h[4]=popc_htonl(header.GetSemantics());
        break;
    case TYPE_EXCEPTION:
        h[2]=popc_htonl(header.GetExceptionCode());
        break;
    case TYPE_RESPONSE:
        h[2]=popc_htonl(header.GetClassID());
        h[3]=popc_htonl(header.GetMethodID());
        break;
    default:
        LOG_ERROR("fail 2");
        return NULL;
    }

    memcpy(dat, h, 20);

    return packeddata.data();
}
// Note LWK: This method was copied from the dynamic to the pseudodynamic version of the code
// TODO LWK: Check with Valentin if ok
void paroc_buffer_xdr::load(char* data, int length) {
    int h[5];

    Reset();
    memcpy(packeddata.data(), data, length);
    memcpy(h, packeddata.data(), 20);

    int n = popc_ntohl(h[0]);
    if(n < 20) {
        LOG_ERROR("[CORE] XDR Buffer - Bad message header (size error:%d)", n);
        return;
    }

    int type = popc_ntohl(h[1]);
    header.SetType(type);
    switch(type) {
    case TYPE_REQUEST:
        header.SetClassID(popc_ntohl(h[2]));
        header.SetMethodID(popc_ntohl(h[3]));
        header.SetSemantics(popc_ntohl(h[4]));
        break;
    case TYPE_EXCEPTION:
        header.SetExceptionCode(popc_ntohl(h[2]));
        break;
    case TYPE_RESPONSE:
        header.SetClassID(popc_ntohl(h[2]));
        header.SetMethodID(popc_ntohl(h[3]));
        break;
    default:
        return;
    }

    packeddata.resize(length);
}


#ifdef OD_DISCONNECT
bool paroc_buffer_xdr::RecvCtrl(paroc_combox &s, paroc_connection *conn) {
    while(true) {
        paroc_connection * t = (paroc_connection *) s.Wait();
        if(!t) {
            paroc_exception::paroc_throw("Remote Object not alive (1)");
        }

        if(!Recv(s, t)) {
            paroc_exception::paroc_throw(errno);
        }

        if(header.GetType() == TYPE_RESPONSE) {
            if(header.GetClassID() == 0 && header.GetMethodID() == 6) {
                return true;
            } else {
                paroc_message_header h = header;
                int unpackposold = unpackpos;
                paroc_array<char> packeddataold = packeddata;
                paroc_connection * t = (paroc_connection *) s.Wait();
                if(!t) {
                    paroc_exception::paroc_throw("Remote Object not alive (2)");
                }

                if(!Recv(s, t)) {
                    paroc_exception::paroc_throw(errno);
                }

                Reset();
                header = h;
                unpackpos = unpackposold;
                packeddata = packeddataold;

                return false;
            }
        }
    }
}

#endif
