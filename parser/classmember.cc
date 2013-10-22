/**
 * File : classmember.cc
 * Author : Tuan Anh Nguyen
 * Description : Generate source code for the broker of parallel objects
 * Creation date : -
 * 
 * Modifications :
 * Authors		Date			Comment
 * P.Kuonen   	June 2011 	Modify to print all exceptions raised in methods
 * clementval	March 2012	Add Enum, Struct support
 * clementval	August 2012	Add asynchronous parallel object allocation (APOA) support
 * vanhieu      July 2013       Modify line #925 
 */

#include "popc_intface.h"

#include "parser.h"
#include "paroc_utils.h"
#include "debug.h"
//#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <assert.h>
//#include <ctype.h>
//#include <unistd.h>
//#include <sys/stat.h>
//#include <sys/types.h>

//#include <sys/types.h>
//#include <time.h>

#define MIN_CLASSID 1000

//Begin Param implementation

Param::Param(DataType *ptype)
{
	name[0]=0;
	mytype=ptype;
	
	paramSize=marshalProc=defaultVal=NULL;
	isVoid=isArray=isRef=isConst=isInput=isOutput=false;
}

Param::Param()
{
	name[0]=0;
	mytype=NULL;

	paramSize=marshalProc=defaultVal=NULL;
	isVoid=isArray=isRef=isConst=isInput=isOutput=false;
}

Param::~Param()
{
	if (paramSize!=NULL) free(paramSize);
	if (marshalProc!=NULL) free(marshalProc);
	if (defaultVal!=NULL) free(defaultVal);
}

bool Param::Match(Param *p)
{
	return (paroc_utils::isEqual(name,p->GetName()));
}

bool Param::InParam()
{
	if (isInput || isConst) return true;
	if (isOutput) return false;
	return true;
}

bool Param::OutParam()
{
	if (isConst) return false;
	if (isOutput) return true;
	if (isInput) return false;
	return (isRef || IsPointer() || mytype->IsArray());
}

bool Param::IsPointer()
{
	return (mytype->IsPointer()>0);
}

bool Param::IsRef()
{
	return isRef;
}

bool Param::IsConst()
{
	return isConst;
}

bool Param::IsArray()
{
	return isArray;
}

bool Param::IsVoid()
{
	return isVoid;
}

void Param::SetType(DataType *type)
{
	mytype=type;
}

DataType *Param::GetType()
{
	return mytype;
}

char *Param::GetName()
{
	return name;
}

char *Param::GetDefaultValue()
{
	return defaultVal;
}

int  Param::GetAttr()
{
	return attr;
}

bool Param::CanMarshal()
{
	if (mytype==NULL) 
		return false;
	int stat=mytype->CanMarshal();
	if (marshalProc!=NULL || stat==1 || (stat==2 && paramSize!=NULL)) 
		return true;
	return false;

}

bool Param::DeclareParam(char *output, bool header)
{
	if (mytype == NULL) 
	  return false;
	if (isConst) {
		strcpy(output,"const ");
		output+=strlen(output);
	}
	char tmpvar[1024];
	
	if (isRef) {
		sprintf(tmpvar,"&%s", name);
	} else if (isArray){
		sprintf(tmpvar,"%s", name);
	} else {
		strcpy(tmpvar,name);
	}

	if (!mytype->GetDeclaration(tmpvar, output)) return false;
	if (defaultVal!=NULL && header)
	{
		strcat(output,"=");
		strcat(output,defaultVal);
	}
	return true;
}

bool Param::DeclareVariable(char *output, bool &reformat, bool allocmem)
{
	if (mytype==NULL) return false;
	if (isVoid) return false;
	
	int nptr=mytype->IsPointer();
	if (nptr==1 && paramSize==NULL)
	{
		DataType *base=mytype->GetBaseType();
		assert(base!=NULL);
		if (!base->GetDeclaration(name, output)) return false;
		if (base->IsParClass()) strcat(output,"(paroc_interface::_paroc_nobind)");
		strcat(output,";\n");
		reformat=true;
	}
	else
	{
		if (!mytype->GetDeclaration(name, output)) return false;
		if (mytype->IsParClass()) strcat(output,"(paroc_interface::_paroc_nobind)");
		reformat=false;
		strcat(output,";\n");

		//Check and allocate memory....
		if (allocmem && paramSize!=NULL && nptr)
		{
			char decl[1024];
			DataType *base=mytype->GetBaseType();
			assert(base!=NULL);
			char basetype[1024];
			base->GetDeclaration(NULL,basetype);

			if (nptr==1 && base->IsParClass())
				sprintf(decl, "paroc_interface_container<%s> __paroc_container_%s(%s);\n%s =__paroc_container_%s;\n", basetype,name,paramSize,name,name);
			else
			{
				char tmpstr[1024];
				strcpy(tmpstr,base->GetName());
				for (int i=1;i<nptr;i++) strcat(tmpstr,"*");
				sprintf(decl, "paroc_container<%s> __paroc_container_%s(%s);\n%s=__paroc_container_%s;\n", tmpstr,name ,paramSize,   name, name);

			}
			strcat(output,decl);
		}
	}
	return true;
}

bool Param::DeclareVariable(char *output)
{
	if (mytype==NULL) return false;

	char tmpvar[1024];
	if (isRef&&!GetType()->IsParClass()) sprintf(tmpvar,"&%s", name);
	else strcpy(tmpvar,name);

	if (mytype->GetDeclaration(tmpvar, output))
	{
		strcat(output,";\n");
		return true;
	}
	return false;
}

bool Param::Marshal(char *bufname, bool reformat,bool inf_side, CArrayChar &output)
{
	if (mytype==NULL) return false;

	char tmpvar[1024];
	if (isConst)
	{
		*tmpvar=0;
//             *tmpvar='(';
//       mytype->GetCastType(tmpvar+1);
//       if (isRef) strcat(tmpvar,"&");
//       strcat(tmpvar,")");
	}
	else *tmpvar=0;

	if (reformat) strcat(tmpvar,"&");

	strcat(tmpvar,name);

	int flags=(inf_side)? FLAG_INPUT : 0;
	flags|=FLAG_MARSHAL;

	char sizeinfo[1024];
	*sizeinfo=0;

	if (paramSize!=NULL && !reformat)
	{
		char decl[1024];
		sprintf(decl, "int __paroc_size1_%s=%s;\n%s.Push(\"%sSize\",\"int\",1);\n%s.Pack(& __paroc_size1_%s,1);\n%s.Pop();\n",name,paramSize,bufname, name, bufname,name, bufname);
		sprintf(sizeinfo,"__paroc_size1_%s",name);

		output.InsertAt(-1,decl,strlen(decl));
	}

	if (marshalProc!=NULL)
	{
		char tmpcode[1024];
		sprintf(tmpcode,"%s(%s,%s, %s, %d, 0);\n",marshalProc,bufname,tmpvar, (*sizeinfo==0)? "0" : sizeinfo,flags);
		output.InsertAt(-1,tmpcode,strlen(tmpcode));
	}
	else
	{
		mytype->Marshal(tmpvar,bufname,(*sizeinfo==0)? NULL : sizeinfo,output);
	}
	return true;
}

bool Param::UnMarshal(char *bufname, bool reformat, bool alloc_mem, bool inf_side, CArrayChar &output)
{
	if (mytype==NULL) return false;

	int flags=(inf_side)? 0: FLAG_INPUT;

	char tmpvar[1024];

	*tmpvar=0;
	if (reformat) strcat(tmpvar,"&");
	strcat(tmpvar,name);

	char sizeinfo[1024];
	*sizeinfo=0;

	if (paramSize!=NULL && !reformat)
	{
		char decl[1024];
		sprintf(decl, "int __paroc_size2_%s;\n%s.Push(\"%sSize\",\"int\",1);\n \n%s.UnPack(& __paroc_size2_%s,1);\n%s.Pop();\n",name,bufname, name, bufname,name, bufname);

		sprintf(sizeinfo,"__paroc_size2_%s",name);

		output.InsertAt(-1,decl,strlen(decl));

		//check to alloc the memory
		int nptr=mytype->IsPointer();
		if (alloc_mem && nptr)
		{
			char alloccode[1024];
			DataType *base=mytype->GetBaseType();
			assert(base!=NULL);
			char basetype[1024];
			base->GetDeclaration(NULL,basetype);

			if (nptr==1 && base->IsParClass())
				sprintf(alloccode, "paroc_interface_container<%s> __paroc_container_%s(__paroc_size2_%s);\n %s=__paroc_container_%s;\n", basetype,name,name,name, name);
			else
			{
				//	      sprintf(alloccode, "paroc_container<typeof(*%s)> __paroc_container_%s(%s,__paroc_size2_%s);\n", name,name,name,name);
				char tmpstr[1024];
				strcpy(tmpstr,base->GetName());
				for (int i=1;i<nptr;i++) strcat(tmpstr,"*");
				sprintf(alloccode, "paroc_container<%s> __paroc_container_%s(__paroc_size2_%s);\n%s=__paroc_container_%s;\n", tmpstr,name,name,name,name);
			}
			output.InsertAt(-1,alloccode,strlen(alloccode));
		}
	}
	if (marshalProc!=NULL)
	{
		char tmpcode[1024];

		sprintf(tmpcode,"%s(%s,%s,%s,%d,%s);",marshalProc,bufname,tmpvar, (*sizeinfo==0)?  "0" : sizeinfo,flags,(inf_side ? "0" : "&_internal_mem"));
		output.InsertAt(-1,tmpcode,strlen(tmpcode));
	}
	else
		mytype->DeMarshal(tmpvar,bufname,(*sizeinfo==0)? NULL : sizeinfo,output);

	return true;
}

bool Param::UserProc(char *output)
{
	if (mytype==NULL) 
		return false;

	*output=0;
	
	if (marshalProc==0) 
		return false;
		
	char decl[1024];

	mytype->GetDeclaration("param",decl);
	sprintf(output,"void %s(paroc_buffer &buf, %s, int hint,  bool marshal);" , marshalProc, decl);
	return true;
}
//END Param implementation



//BEGIN OD - object description class
ObjDesc::ObjDesc()
{
	odstr=NULL;
}
ObjDesc::~ObjDesc()
{
	if (odstr!=NULL) free(odstr);
}

void ObjDesc::Generate(char *code)
{
	code[0]=0;
	if (odstr == NULL) 
	  return;
	strcpy(code, "\n  ");   
	strcat(code, odstr);
}

void ObjDesc::SetCode(char *code)
{
	if (odstr!=NULL) free(odstr);
	odstr=(code==NULL)? NULL : popc_strdup(code);
}

//END ObjectDesc - object description class

// BEGIN ClassMember class implementation....
ClassMember::ClassMember(Class *cl, AccessType myaccess)
{
	accessType=myaccess;
	myclass=cl;
	line=-1;
}

ClassMember::~ClassMember()
{
}


AccessType ClassMember::GetMyAccess()
{
	return accessType;
}

Class * ClassMember::GetClass()
{
	return myclass;
}

void ClassMember::SetLineInfo(int linenum)
{
	line=linenum;
}

void ClassMember::GenerateClient(CArrayChar &output)
{
}

void ClassMember::generate_header_pog(CArrayChar &output, bool interface)
{
  GenerateHeader(output, interface); 
}

void ClassMember::GenerateHeader(CArrayChar &output, bool interface)
{
	char *fname;
	if (line>0 && (fname=myclass->GetFileInfo())!=NULL)
	{
		char str[1024];
		sprintf(str,"\n# %d \"%s\"\n", line, fname);
		output.InsertAt(-1,str,strlen(str));
	}

}

// END of ClassMember implementation

// Implement Attribute class
Attribute::Attribute(Class *cl, AccessType myaccess): ClassMember(cl, myaccess), attributes(0,1)
{
}

void Attribute::GenerateHeader(CArrayChar &output, bool interface)
{
	if (interface) return;
	ClassMember::GenerateHeader(output, interface);

	char tmp[1024];
	int n=attributes.GetSize();
	for (int i=0;i<n;i++)
	{
		Param &p=*(attributes[i]);
		p.DeclareVariable(tmp);
		output.InsertAt(-1,tmp,strlen(tmp));
	}
}

Param *Attribute::NewAttribute()
{
	Param *t=new Param;
	attributes.InsertAt(-1,t);
	return t;
}

/**
 * Implementation of Enumeration class
 */
 
// enum type constrcutor
Enumeration::Enumeration(Class *cl, AccessType myaccess): ClassMember(cl, myaccess)
{
	
}

// enum type destructor
Enumeration::~Enumeration()
{

}

// Save the name of the enum type
void Enumeration::setName(std::string value)
{
	name = value;
}

// Save the arguments of the enum type
void Enumeration::setArgs(std::string value)
{
	args = value;
}

// Generation of the appropriate code for the enum type
void Enumeration::GenerateHeader(CArrayChar &output, bool interface){
	ClassMember::GenerateHeader(output, interface);
	output.InsertAt(-1, "enum ",strlen("enum "));
	output.InsertAt(-1,name.c_str(),strlen(name.c_str()));
	output.InsertAt(-1, "{", 1);
	output.InsertAt(-1, args.c_str(), strlen(args.c_str()));	
	output.InsertAt(-1, "};", 2);
}

/**
 * Implementation of Structure class
 */
 
// struct type constrcutor
Structure::Structure(Class *cl, AccessType myaccess): ClassMember(cl, myaccess)
{
	
}

// struct type destructor
Structure::~Structure()
{

}

// Save the name of the struct type
void Structure::setName(std::string value)
{
	name = value;
}

// Save the objects of the struct type
void Structure::setObjects(std::string value)
{
	objects = value;
}

void Structure::setInnerDecl(std::string value)
{
	if(innerdecl.length()==0){
		innerdecl = value;	
	} else {
		innerdecl.append("\n");
		innerdecl.append(value);
	}
}

// Generation of the appropriate code for the enum type
void Structure::GenerateHeader(CArrayChar &output, bool interface){
	ClassMember::GenerateHeader(output, interface);
	output.InsertAt(-1, "struct ",strlen("struct "));
	output.InsertAt(-1, name.c_str(),strlen(name.c_str()));
	output.InsertAt(-1, "{\n", 2);
	output.InsertAt(-1, innerdecl.c_str(), strlen(innerdecl.c_str()));	
	output.InsertAt(-1, "\n}", 2);
	output.InsertAt(-1, objects.c_str(), strlen(objects.c_str()));
	output.InsertAt(-1, ";", 1);
}

/**
 * Implementation of TypeDefinition class
 */
 
// TypeDefinition constructor
TypeDefinition::TypeDefinition(Class *cl, AccessType myaccess): ClassMember(cl, myaccess)
{
	ptr = false;
}

// TypeDefinition type destructor
TypeDefinition::~TypeDefinition()
{

}

// Save the name of the typedef
void TypeDefinition::setName(std::string value)
{
	name = value;
}

// Save the basetype of the typedef
void TypeDefinition::setBaseType(std::string value)
{
	basetype = value;
}

void TypeDefinition::setAsPtr()
{
	ptr = true;
}

bool TypeDefinition::isPtr()
{
	return ptr;
}

void TypeDefinition::setAsArray()
{
	array = true;
}

bool TypeDefinition::isArray()
{
	return array;
}


// Generation of the appropriate code for the enum type
void TypeDefinition::GenerateHeader(CArrayChar &output, bool interface){
	ClassMember::GenerateHeader(output, interface);
	output.InsertAt(-1, "typedef ",strlen("typedef "));
	output.InsertAt(-1, name.c_str(),strlen(name.c_str()));
	if(isPtr()){
		output.InsertAt(-1, " * ", 3);
	} else {
		output.InsertAt(-1, " ", 1);
	}
	output.InsertAt(-1, basetype.c_str(), strlen(basetype.c_str()));
	if(isArray()){
		output.InsertAt(-1, "[]", 2);
	}
	output.InsertAt(-1, ";", 1);
}




//Directives inside a parallel class
Directive::Directive(Class *cl, char *directive): ClassMember(cl, PUBLIC)
{
	code=popc_strdup(directive);
}

Directive::~Directive()
{
	if (code!=NULL) free(code);
}

void Directive::GenerateHeader(CArrayChar &output, bool interface)
{
	if (code!=NULL)
	{
		output.InsertAt(-1,"\n",1);
		output.InsertAt(-1,code,strlen(code));
		output.InsertAt(-1,"\n",1);
	}
}



// BEGIN Method class implementation....

const int Method::POPC_METHOD_NON_COLLECTIVE_SIGNAL_ID = 9;
const int Method::POPC_METHOD_NON_COLLECTIVE_SIGNAL_INVOKE_MODE = 0; 
const char* Method::POPC_METHOD_NON_COLLECTIVE_SIGNAL_NAME = "set_non_collective_rank";

Method::Method(Class *cl, AccessType myaccess): ClassMember(cl, myaccess), returnparam(NULL)
{
	name[0]=0;
	id=0;
	invoketype = invokesync;
	isConcurrent = false;
	isMutex = false;
	isHidden = false;
	isVirtual = false;
	isPureVirtual = false;
	isGlobalConst = false;
	
	_is_collective = false;
	_is_broadcast = false;
	_is_scatter = false;
	_is_gather = false;
  _is_reduce = false;
}

Method::~Method()
{
	int n,i;
	n=params.GetSize();
	for (i=0;i<n;i++) if (params[i]!=NULL) delete params[i];
}

/**
 * Set a method as collective method
 * @param type  Type of the collective method (Broadcast, Gather, Scatter, Reduce). 
 */
void Method::set_collective(CollectiveType type)
{
  _is_collective = true; 
  switch (type) {
    case POPC_COLLECTIVE_BROADCAST:
      _is_broadcast = true; 
      break; 
    case POPC_COLLECTIVE_SCATTER: 
      _is_scatter = true; 
      break;    
    case POPC_COLLECTIVE_GATHER: 
      _is_gather = true; 
      break;    
    case POPC_COLLECTIVE_REDUCE: 
      _is_reduce = true; 
      break;    
    default: 
      _is_collective = false;   
  }
}

/**
 * Check if the method is a collective method
 * @return TRUE if the method is collective. FALSE if it is non-collective.
 */
bool Method::is_collective() {
  if(_is_collective)
    return true; 
  return false; 
}

/**
 * Check if the parameter of a specific method can be marshalled
 * @return 0 in case of succes, -1 is the return argument cannot be marshalled, the number of the parameter that cannot be marshalled
 */
int Method::CheckMarshal()
{
	// Avoid if access modifier is not public or if the method is hidden
	if (GetMyAccess()!=PUBLIC || isHidden) 
		return 0;

	if (MethodType()==METHOD_NORMAL && !returnparam.CanMarshal()) 
		return -1;
		
	// Get the number of parameter for this method	
	int n=params.GetSize();
	Class *cl=GetClass();

	// Check all the parameters of the method
	for (int i=0;i<n;i++) {
		if (params[i]->GetType()->IsParClass() && !params[i]->IsRef()) {
			fprintf(stderr,"%s:%d: POP-C++ Error in %s::%s : parallel object '%s' must be passed as reference to the remote object.\n",  cl->GetFileInfo(), line, cl->GetName(), name,params[i]->GetName());
			return(i+1);
		}
		// Check if the parameter can be marshalled 
		if (!params[i]->CanMarshal()) 
			return (i+1);
	}
	return 0;

}

void Method::GenerateReturn(CArrayChar &output, bool header)
{
	GenerateReturn(output,header, false);
}

void Method::GenerateReturn(CArrayChar &output, bool header, bool interface)
{
	DataType *type=returnparam.GetType();
	if (type==NULL) return;
	if (header && isVirtual) output.InsertAt(-1,"virtual ",8);
	else if (!header) output.InsertAt(-1,"\n",1);

	if (returnparam.GetType()->IsParClass() && !returnparam.IsRef())
	{
		/* changed by David : no more warning, error ! */
		//fprintf(stderr,"%s:%d: WARNING in %s::%s : the return argument '%s' is treated as a reference to a parallel object.\n", GetClass()->GetFileInfo(), line, GetClass()->GetName(), (const char*)name, returnparam.GetType()->GetName());
		
		fprintf(stderr,"%s:%d: POP-C++ Error in %s::%s : the return argument '%s' should by a reference to a parallel object !\n", GetClass()->GetFileInfo(), line, GetClass()->GetName(), (const char*)name, returnparam.GetType()->GetName());
		exit(1);
	}
	if (returnparam.IsRef() && !returnparam.GetType()->IsParClass())
	{
		fprintf(stderr,"%s:%d: POP-C++ Error in %s::%s : methods of parallel objects can not technically return a reference.\n", GetClass()->GetFileInfo(), line, GetClass()->GetName(), (const char*)name);
		exit(1);
	}

	// add by david
	if (returnparam.IsConst()){
		const char* tmp_const= "const ";
		output.InsertAt(-1,tmp_const, strlen(tmp_const));
	}

	char tmp[1024];
	
	type->GetDeclaration(NULL,tmp);
	//if (returnparam.IsConst())sprintf(tmp, "const %s", tmp);
	output.InsertAt(-1,tmp, strlen(tmp));
	
	// add by david
	if (returnparam.IsRef() && !interface){
//		if(!returnparam.GetType()->IsParClass())
//		{
			const char* tmp_const= "& ";
			output.InsertAt(-1,tmp_const, strlen(tmp_const));
//		}
	}
	
	output.InsertAt(-1," ");
}

void Method::GeneratePostfix(CArrayChar &output, bool header)
{
	// add const add the end of methode - david
	if(isGlobalConst)
	{
		const char* tmp = " const ";
		output.InsertAt(-1,tmp, strlen(tmp));
	}
	
	if (header) 
	  output.InsertAt(-1,";",1);
}

void Method::GenerateName(CArrayChar &output, bool header)
{
	if (header) {
	  output.InsertAt(-1,name,strlen(name));
	} else {
		char str[256];
		sprintf(str,"%s::%s",GetClass()->GetName(),name);
		output.InsertAt(-1,str,strlen(str));
	}
}

void Method::GenerateArguments(CArrayChar &output, bool header)
{
	char tmpcode[10240];

	output.InsertAt(-1,"(",1);
	int nb=params.GetSize();
	for (int j=0;j<nb;j++)
	{
		Param &p=*(params[j]);
		p.DeclareParam(tmpcode, header);
		if (j<nb-1) strcat(tmpcode,", ");
		output.InsertAt(-1,tmpcode,strlen(tmpcode));
	}
	output.InsertAt(-1,")",1);
}


void Method::GenerateClientPrefixBody(CArrayChar &output)
{

}

void Method::GenerateClient(CArrayChar &output)
{
	if ((isVirtual && GetClass()->methodInBaseClass(*this)) || isHidden) 
	  return;
	  
	// Check if we can generate marshalling stubs
	int t = CheckMarshal();
	if (t != 0) {
		Class *cl = GetClass();
		if (t == -1) {
			if(returnparam.IsPointer()) {
				fprintf(stderr,"%s:%d: POP-C++ Error in %s::%s: The return argument is a Illegal Pointer.\n", cl->GetFileInfo(),line, cl->GetName(), name);
			} else {
				fprintf(stderr,"%s:%d: POP-C++ Error in %s::%s: unable to marshal the return argument.\n", cl->GetFileInfo(),line, cl->GetName(), name);
			}
		} else {
			if(params[t-1]->IsPointer()) {
				fprintf(stderr,"%s:%d: POP-C++ Error in %s::%s: Illegal Pointer Parameter at argument %d.\n",  cl->GetFileInfo(), line, cl->GetName(), name,t);
			} else {
				fprintf(stderr,"%s:%d: POP-C++ Error in %s::%s: unable to marshal argument %d.\n",  cl->GetFileInfo(), line, cl->GetName(), name,t);
			}
		}
		exit(1);
	}
	
	char tmpcode[10240];

	char *clname = GetClass()->GetName();
	int j, nb = params.GetSize();

	GenerateReturn(output, false, true);
	GenerateName(output, false);
	GenerateArguments(output, false);
	GeneratePostfix(output, false);

	output.InsertAt(-1, "\n{", 2);

	GenerateClientPrefixBody(output);
		

	// Generate method body
	int invoke_code = 0;
	bool waitreturn = false;
	if (MethodType() == METHOD_CONSTRUCTOR) {
		invoke_code |= INVOKE_CONSTRUCTOR | INVOKE_SYNC | INVOKE_MUTEX;
		waitreturn = true;
	} else {
		if (invoketype == invokesync || (invoketype == autoselect && hasOutput()) ) {
			invoke_code |= INVOKE_SYNC;
			waitreturn = true;
		}

		if (isMutex) {
		  invoke_code |= INVOKE_MUTEX;
		} else if (isConcurrent) {
		  invoke_code |= INVOKE_CONC;
		}
	}
	
	
	/**
	 * Asynchronous Parallel Object Allocation (APOA)
	 * The code below is generated to support the APOA in POP-C++ application.
	 * Generated at the beginning of each remote method invocation (not for constructor method).
	 */
	if(!GetClass()->IsCoreCompilation() && MethodType() != METHOD_CONSTRUCTOR && GetClass()->IsAsyncAllocationDisable()){
            sprintf(tmpcode, "\n  // Waiting for APOA to be done before executing any method\n");
            output.InsertAt(-1,tmpcode,strlen(tmpcode));	
            sprintf(tmpcode, "  void* status;\n  pthread_join(_popc_async_construction_thread, &status);\n");
            output.InsertAt(-1,tmpcode,strlen(tmpcode));
            char* nameOfRetType = returnparam.GetType()->GetName();
            if (MethodType()==METHOD_NORMAL &&!returnparam.GetType()->Same((char*)"void")) 
                sprintf(tmpcode, "  if(!isBinded()) {\n    printf(\"POP-C++ Error: [APOA] Object not allocated but allocation process done !\");\n    %s *tempObject = 0;\n    return (*tempObject);\n  }\n", nameOfRetType);
            else  if (MethodType()==METHOD_NORMAL && returnparam.GetType()->Same((char*)"void")) 
                sprintf(tmpcode, "  if(!isBinded()) {\n    printf(\"POP-C++ Error: [APOA] Object not allocated but allocation process done !\");\n    return;\n  }\n");           
            output.InsertAt(-1,tmpcode,strlen(tmpcode));	  	
	} // End of APOA Support
	
	
	
	
	
	if(!GetClass()->is_collective()) {
  	sprintf(tmpcode, "\n  paroc_mutex_locker __paroc_lock(_paroc_imutex);");
  	output.InsertAt(-1,tmpcode,strlen(tmpcode));  
 	  sprintf(tmpcode, "\n  paroc_connection* _popc_connection = __paroc_combox->get_connection();\n  __paroc_buf->Reset();\n  paroc_message_header __paroc_buf_header(CLASSUID_%s,%d,%d, \"%s\");\n  __paroc_buf->SetHeader(__paroc_buf_header);\n", clname, id, invoke_code, name);
  	output.InsertAt(-1,tmpcode,strlen(tmpcode));	 
	} else {
	  // Additional code if the method is not collective
	  if(!is_collective() && MethodType() != METHOD_CONSTRUCTOR) {
     	sprintf(tmpcode, "\n  // Generate additional information for a non collective call");
    	output.InsertAt(-1,tmpcode,strlen(tmpcode));  
      
      sprintf(tmpcode, "\n  paroc_connection* _popc_connection = _popc_combox->get_connection();\n  _popc_buffer->Reset();\n  paroc_message_header _popc_message_header(CLASSUID_%s, %d, %d, \"%s\");\n  _popc_buffer->SetHeader(_popc_message_header);\n", 
        clname, POPC_METHOD_NON_COLLECTIVE_SIGNAL_ID, POPC_METHOD_NON_COLLECTIVE_SIGNAL_INVOKE_MODE, POPC_METHOD_NON_COLLECTIVE_SIGNAL_NAME);
    	output.InsertAt(-1, tmpcode, strlen(tmpcode));
      
      sprintf(tmpcode, "\n   _popc_buffer->Push(\"rank\", \"int\", 1);\n  _popc_buffer->Pack(&_popc_default_rank_for_single_call, 1);\n  _popc_buffer->Pop();\n"); 
      output.InsertAt(-1, tmpcode, strlen(tmpcode));
      
      sprintf(tmpcode, "\n  popc_send_request(_popc_buffer, _popc_connection);\n  _popc_buffer->Reset();\n  paroc_message_header _popc_message_header_call(CLASSUID_%s, %d, %d, \"%s\");", clname, id, invoke_code, name);    
      output.InsertAt(-1, tmpcode, strlen(tmpcode));  
  
      sprintf(tmpcode, "\n  _popc_buffer->SetHeader(_popc_message_header_call);"); 
      output.InsertAt(-1, tmpcode, strlen(tmpcode));  
	  }	else {
      sprintf(tmpcode, "\n  paroc_connection* _popc_connection = _popc_combox->get_connection();\n  _popc_buffer->Reset();\n  paroc_message_header _popc_message_header(CLASSUID_%s, %d, %d, \"%s\");\n  _popc_buffer->SetHeader(_popc_message_header);\n", clname, id, invoke_code, name);
    	output.InsertAt(-1,tmpcode,strlen(tmpcode));	 	
    }
	}
	


	// Generate marshalling stub
	for (j=0;j<nb;j++)
	{
		Param &p=*(params[j]);
		if (p.InParam())  {
		  if(!GetClass()->is_collective()) {
  			p.Marshal((char*)"(*__paroc_buf)", false, true, output);
  		} else {
      	p.Marshal((char*)"  (*_popc_buffer)", false, true, output);  		  
  		}
		}
	}

	// Marshalling code generation is done. Transmit buffer

	if(!GetClass()->is_collective()) {
	  strcpy(tmpcode,"\n  popc_send_request(__paroc_buf, _popc_connection);");
	} else {
    strcpy(tmpcode,"\n  popc_send_request(_popc_buffer, _popc_connection);");	
	}
	output.InsertAt(-1,tmpcode,strlen(tmpcode));

	if (waitreturn) {
#ifdef OD_DISCONNECT
		strcpy(tmpcode,"\n\tif(od.getCheckConnection()){\n\t\tif(!RecvCtrl())paroc_exception::paroc_throw_errno();\n\t}");
		output.InsertAt(-1,tmpcode,strlen(tmpcode));
		strcpy(tmpcode,"\n\telse\n");
		output.InsertAt(-1,tmpcode,strlen(tmpcode));
#endif
    
  	if(!GetClass()->is_collective()) {    
		  strcpy(tmpcode,"\t{\n\t\tif (!__paroc_buf->Recv((*__paroc_combox), _popc_connection)) paroc_exception::paroc_throw_errno();\n\t}\n\t\n\tparoc_buffer::CheckAndThrow(*__paroc_buf);\n");
		} else {
      strcpy(tmpcode,"\n  popc_recv_response(_popc_buffer, _popc_connection);"); 
		}
		output.InsertAt(-1,tmpcode,strlen(tmpcode));
		for (j=0;j<nb;j++) {
			Param &p=*(params[j]);
			if (p.OutParam()) {
  		  if(!GetClass()->is_collective()) {
    			p.UnMarshal((char*)"(*__paroc_buf)",false,false, true, output);
  	  	} else {
    			p.UnMarshal((char*)"  (*_popc_buffer)",false,false, true, output);
    		}				
			}
		}

		if (MethodType() == METHOD_NORMAL && !returnparam.GetType()->Same((char*)"void")) {
			bool reformat;
			returnparam.DeclareVariable(tmpcode,reformat,false);
			output.InsertAt(-1,tmpcode,strlen(tmpcode));


			
    	if(!GetClass()->is_collective()) {   			
  			returnparam.UnMarshal((char*)"(*__paroc_buf)", reformat, true, true, output);    	
  			strcpy(tmpcode,"\n  __paroc_buf->Reset();");
      } else {
  			returnparam.UnMarshal((char*)"(*_popc_buffer)", reformat, true, true, output);      
  			strcpy(tmpcode,"\n  _popc_buffer->Reset();");      
      }
		  output.InsertAt(-1,tmpcode,strlen(tmpcode));				


      // Added for new communication support		
      strcpy(tmpcode,"\n_popc_connection->reset();\nreturn ");
			
			if (reformat) strcat(tmpcode,"&");
			strcat(tmpcode,returnparam.name);
			strcat(tmpcode,";\n}\n");
			output.InsertAt(-1,tmpcode,strlen(tmpcode));

		} else {
    	if(!GetClass()->is_collective()) {   		
  			strcpy(tmpcode,"\n  __paroc_buf->Reset();\n");
      } else {
  			strcpy(tmpcode,"\n  _popc_buffer->Reset();\n");      
      }  			
			output.InsertAt(-1,tmpcode,strlen(tmpcode));
      // Added for new communication support		
                strcpy(tmpcode,"\n  _popc_connection->reset();}\n");
		  output.InsertAt(-1,tmpcode,strlen(tmpcode));	
		}
	}
	else
	{
		// Check that we are not waiting for a returning param
		for (j=0;j<nb;j++) {
			if (params[j]->OutParam() && !params[j]->GetType()->IsParClass() ) {
				Class *cl=GetClass();
				fprintf(stderr,"%s:%d: POP-C++ Error in %s::%s: Asynchronous method cannot have an output (parameter %s must be declared with [in])\n",cl->GetFileInfo(),line, cl->GetName(), name, params[j]->name);
				exit(1);
			}
		}
		if (MethodType()==METHOD_NORMAL &&!returnparam.GetType()->Same((char*)"void")) {
			Class *cl=GetClass();
			fprintf(stderr,"%s:%d: POP-C++ Error in %s::%s: Asynchronous method must return void\n",cl->GetFileInfo(),line, cl->GetName(), name);
			exit(1);
		}
#ifdef OD_DISCONNECT
		strcpy(tmpcode,"\n\tint time_alive, time_control, oldTime;\nod.getCheckConnection(time_alive, time_control);\n\t");
		output.InsertAt(-1,tmpcode,strlen(tmpcode));
		strcpy(tmpcode,"\n\tif(time_alive > 0 && time_control > 0 ){\n\t\toldTime=__paroc_combox->GetTimeout();\n\t\t__paroc_combox->SetTimeout(time_alive);\n\t\t__paroc_combox->RecvAck();\n\t\t__paroc_combox->SetTimeout(oldTime);\n\t}");
		output.InsertAt(-1,tmpcode,strlen(tmpcode));
#else
  	if(!GetClass()->is_collective()) {   		
 			strcpy(tmpcode,"\n  __paroc_buf->Reset();");
    } else {
			strcpy(tmpcode,"\n  _popc_buffer->Reset();");      
    }  		
		output.InsertAt(-1,tmpcode,strlen(tmpcode));

    // Added for new communication support		
    strcpy(tmpcode,"_popc_connection->reset();\n}\n");
		output.InsertAt(-1,tmpcode,strlen(tmpcode));
#endif
	}
}

void Method::generate_header_pog(CArrayChar &output, bool interface) {
	int type = MethodType();
	if (interface) {
		if (type == METHOD_DESTRUCTOR || GetMyAccess() != PUBLIC || isHidden) 
		  return;
		if (isVirtual && GetClass()->methodInBaseClass(*this)) {
			return;
		}
	}

	ClassMember::GenerateHeader(output, interface);
	
	if(type == METHOD_NORMAL) {
	  GenerateReturn(output, true, interface);
	} else {
	  GenerateReturn(output, true);
	}

	if (!interface && type != METHOD_NORMAL) {
		char str[256];
		sprintf(str, "%s%s", name, Class::POG_OBJECT_POSTFIX);
		output.InsertAt(-1, str, strlen(str));
	} else {
	  GenerateName(output, true);
  }
  
 	GenerateArguments(output, true);
 	
	if (isPureVirtual && !interface) {
		output.InsertAt(-1,"=0",2);
	}

	GeneratePostfix(output, true);  
}

void Method::GenerateHeader(CArrayChar &output, bool interface)
{
	int type=MethodType();
	if (interface)
	{
		if (type==METHOD_DESTRUCTOR || GetMyAccess()!=PUBLIC || isHidden) return;
		if (isVirtual && GetClass()->methodInBaseClass(*this))
		{
			return;
		}
	}
	ClassMember::GenerateHeader(output, interface);
	
	if(type==METHOD_NORMAL) GenerateReturn(output, true, interface);
	else GenerateReturn(output, true);

	if (!interface && type != METHOD_NORMAL)
	{
		char str[256];
		sprintf(str, "%s%s", name, OBJ_POSTFIX);
		output.InsertAt(-1, str, strlen(str));
	} else { 
	  GenerateName(output, true);
	}
	GenerateArguments(output, true);

	if (isPureVirtual && !interface)
	{
		output.InsertAt(-1,"=0",2);
	}

	GeneratePostfix(output,true);
}

void Method::GenerateBrokerHeader(CArrayChar &output)
{
	int type=MethodType();
	if (type==METHOD_DESTRUCTOR || GetMyAccess()!=PUBLIC || isHidden) return;
	if (isVirtual && GetClass()->methodInBaseClass(*this)) return;

	char str[1024];
	sprintf(str,"\nvoid Invoke_%s_%d(paroc_buffer &__paroc_buf, paroc_connection *__interface_output);",name , id);
	output.InsertAt(-1,str,strlen(str));
}

void Method::generate_broker_header_pog(CArrayChar &output)
{
	int type=MethodType();
	if (type==METHOD_DESTRUCTOR || GetMyAccess()!=PUBLIC || isHidden) return;
	if (isVirtual && GetClass()->methodInBaseClass(*this)) return;

	char str[1024];
	sprintf(str,"\n  void Invoke_%s_%d(paroc_buffer &_popc_buffer, paroc_connection *_popc_connection);",name , id);
	output.InsertAt(-1,str,strlen(str));
}

void Method::GenerateBroker(CArrayChar &output)
{
	int type = MethodType();
	
	if (type == METHOD_DESTRUCTOR || GetMyAccess() != PUBLIC || isHidden) {
	  return;
	}
	
	if (isVirtual && GetClass()->methodInBaseClass(*this)) {
	  return;
	}

	Class *cl = GetClass();
	char *clname = cl->GetName();

	int nb=params.GetSize();

	char brokername[256];
	sprintf(brokername, "%s%s", cl->GetName(), Class::POG_BROKER_POSTFIX);
	
	paroc_array<bool> reformat;
	reformat.SetSize(nb);


	// Now generate method wrappers
	char str[1024];

  if(GetClass()->is_collective()) {
    sprintf(str,"\nvoid %s::Invoke_%s_%d(paroc_buffer &_popc_buffer, paroc_connection *_popc_connection)\n{",brokername,name, id);
  } else {
  	sprintf(str,"\nvoid %s::Invoke_%s_%d(paroc_buffer &__paroc_buf, paroc_connection *__interface_output)\n{",brokername,name, id);
  }
	output.InsertAt(-1,str,strlen(str));

	char methodcall[1024];
	bool haveReturn=false;
	
	if (cl->IsCoreCompilation() || !cl->IsBasePureVirtual()) { // ADDED FOR 2.0.3 Create constructor and stuff only if the parclass is not abstract
		if (type==METHOD_CONSTRUCTOR) {
			//Constructor...create object now...
			if(GetClass()->is_collective()) {
  			sprintf(methodcall,"\n  try {\n    _popc_internal_object = new %s%s(", clname, Class::POG_OBJECT_POSTFIX);
			} else {
  			sprintf(methodcall,"\n  try {\n    obj = new %s%s(", clname, OBJ_POSTFIX);
  		}
		} else if (type != METHOD_NORMAL || returnparam.GetType()->Same((char*)"void")) {
		  if(GetClass()->is_collective()) {
  			sprintf(methodcall,"\n  %s%s* _popc_object = dynamic_cast<%s%s*>(_popc_internal_object);\n  try {\n    _popc_object->%s(",clname,Class::POG_OBJECT_POSTFIX,clname,Class::POG_OBJECT_POSTFIX, name); 
      } else {
  			sprintf(methodcall,"\n%s%s * _paroc_obj = dynamic_cast<%s%s *>(obj);\ntry {\n_paroc_obj->%s(", clname, OBJ_POSTFIX, clname, OBJ_POSTFIX, name);       
      }  			
		} else {
			char retdecl[1024];
			char tmpvar[1024];
			strcpy(tmpvar,returnparam.GetName());
			returnparam.GetType()->GetDeclaration(tmpvar,retdecl);

			if (returnparam.GetType()->IsParClass()) {
        if(GetClass()->is_collective()) {			
				  sprintf(methodcall,"\n%s(paroc_interface::_paroc_nobind);\n    %s%s * _popc_object = dynamic_cast<%s%s*>(_popc_internal_object);\n  try {\n    %s=_popc_object->%s(",retdecl, clname, Class::POG_OBJECT_POSTFIX, clname, Class::POG_OBJECT_POSTFIX, tmpvar, name);
				} else {
          sprintf(methodcall,"\n%s(paroc_interface::_paroc_nobind);\n%s%s * _paroc_obj=dynamic_cast<%s%s *>(obj);\ntry {\n%s=_paroc_obj->%s(", retdecl, clname, OBJ_POSTFIX, clname, OBJ_POSTFIX, tmpvar, name);				
				}
			} else {
			  if(GetClass()->is_collective()) {
	  			sprintf(methodcall,"\n%s;\n%s%s * _popc_object = dynamic_cast<%s%s*>(_popc_internal_object);\n  try {\n    %s=_popc_object->%s(",retdecl, clname, Class::POG_OBJECT_POSTFIX, clname, Class::POG_OBJECT_POSTFIX, tmpvar, name);
				} else {
  				sprintf(methodcall,"\n%s;\n%s%s * _paroc_obj=dynamic_cast<%s%s *>(obj);\ntry {\n%s=_paroc_obj->%s(",retdecl, clname, OBJ_POSTFIX, clname, OBJ_POSTFIX, tmpvar, name);
				}
			}
			haveReturn = true;
		}

		bool have_memspool = false;

		// unmarhall and allocate memory for input arguments first
  	strcpy(str,"\n  ");
		output.InsertAt(-1,str,strlen(str));
		
		for (int j = 0; j < nb; j++) {
			Param &p = *(params[j]);
			if (!p.InParam()) 
				continue;
			char decl[1024];
			if(p.DeclareVariable(decl,reformat[j],false))
				output.InsertAt(-1, decl, strlen(decl));

		  if(GetClass()->is_collective()) {	    
  			strcpy(str,"_popc_buffer");
  		} else {
  			strcpy(str,"__paroc_buf");
  		}
			if (p.marshalProc!=NULL && !have_memspool) {
				strcpy(str,"\nparoc_memspool _internal_mem;");
				output.InsertAt(-1,str,strlen(str));
				have_memspool=true;
			}
		  if(GetClass()->is_collective()) {	 			
  			p.UnMarshal((char*)"  _popc_buffer", reformat[j], true, false, output);
  		} else {
      	p.UnMarshal((char*)"__paroc_buf",reformat[j],true,false,output);  		
  		}
		}

		// Then, declare and alloc mem  for output-only arguments

		for (int j = 0; j < nb;j++) {
		
			Param &p=*(params[j]);
			if (!p.InParam()  /*&& strcmp(p.GetType()->GetName(), "void") != 0 */) {
				char decl[1024];
				p.DeclareVariable(decl,reformat[j],true);
				output.InsertAt(-1, decl, strlen(decl));
			}

			if (reformat[j] && !p.IsVoid())  
				strcat(methodcall," &");
			strcat(methodcall,p.GetName());
			if (j<nb-1) 
				strcat(methodcall,", ");
		} 

		strcat(methodcall,");");
		// Add 'catch' to catch and print all std exceptions raised in the method
		char tempcatch[256];
		for (int i=0; i<256; i++)
            tempcatch[i]='\0';

		sprintf(tempcatch,"\n  } catch(std::exception& e) {\n    printf(\"POP-C++ Warning: Exception '%%s' raised in method '%s' of class '%s'\\n\",e.what());\n    throw;\n  }\n", name, clname);
		strcat(methodcall,tempcatch); 
		


		//now....generate the call...
		output.InsertAt(-1,methodcall,strlen(methodcall));
		
/*

  } */
		
		if(GetClass()->is_collective()) {
  		sprintf(str,"\n  if (_popc_connection != 0) {\n    _popc_buffer.Reset();\n    paroc_message_header _popc_message_header(\"%s\");\n    _popc_buffer.SetHeader(_popc_message_header);\n", name);
		} else {
    	sprintf(str,"\nif (__interface_output!=0) \n{\n__paroc_buf.Reset();\nparoc_message_header __paroc_buf_header(\"%s\");\n__paroc_buf.SetHeader(__paroc_buf_header);\n", name);	
		}
		
		output.InsertAt(-1,str,strlen(str));

		for (int j=0;j<nb;j++) {
			Param &p=*(params[j]);
			if (p.OutParam()) {
    		if(GetClass()->is_collective()) {			
		  		p.Marshal((char*)"    _popc_buffer", reformat[j], false, output);
		  	} else {
  		  	p.Marshal((char*)"__paroc_buf", reformat[j], false, output);
		  	}
			}
		}
		
		if (haveReturn) {
			if(GetClass()->is_collective()) {			
  			returnparam.Marshal((char*)"    _popc_buffer",false,false, output);
		  } else {
  			returnparam.Marshal((char*)"__paroc_buf",false,false, output);
		  }

		}




		if(GetClass()->is_collective()) {			
		  // TODO change true by false if non-collective
		  if(type == METHOD_CONSTRUCTOR) {
      	sprintf(str, "\n    popc_send_response(_popc_buffer, _popc_connection, true);");	  
		  } else {
  		  sprintf(str, "\n    popc_send_response(_popc_buffer, _popc_connection, %s);", (is_collective()) ? "true" : "false");  
		  }
    	output.InsertAt(-1,str,strlen(str));					
    	strcpy(str, "\n    _popc_connection->reset();\n  }\n}\n");		
    	output.InsertAt(-1,str,strlen(str));				
    } else {
  		strcpy(str,"\nif (!__paroc_buf.Send(__interface_output)) paroc_exception::paroc_throw_errno();\n}\n");
    	output.InsertAt(-1,str,strlen(str));					
    	strcpy(str,"\nif(__interface_output != 0)\n__interface_output->reset();\n}\n");		
    	output.InsertAt(-1,str,strlen(str));				    
    }
  	// End of mod
	} else {
		if(cl->IsWarningEnable())
			printf("POP-C++ Warning: %s is an abstract parclass. Be aware that only the final class (parallel object) will keep this semantic.\n", clname);
		strcpy(str,"}\n");	// Close the method braces
  	output.InsertAt(-1,str,strlen(str));
	}

}

Param *Method::AddNewParam()
{
	Param *t=new Param;
	params.InsertAt(-1,t);
	return t;
}

bool Method::hasInput()
{
	int np=params.GetSize();
	Param **pr=params;
	for (int i=0;i<np;i++,pr++) if ((*pr)->InParam()) return true;
	return false;
}

bool Method::hasOutput()
{
	if (MethodType()==METHOD_NORMAL)
	{
		if (!returnparam.GetType()->Same((char*)"void")) return true;
	}

	int np=params.GetSize();
	Param **pr=params;
	for (int i=0;i<np;i++,pr++) if ((*pr)->OutParam()) return true;
	return false;
}

bool Method::operator ==(Method &other)
{
	if (isVirtual!=other.isVirtual) return false;
	if (GetMyAccess()!=other.GetMyAccess()) return false;
	if (MethodType()!=other.MethodType()) return false;
	if ( !paroc_utils::isEqual(name,other.name) ) return false;


	DataType *type1=returnparam.GetType();
	DataType *type2=other.returnparam.GetType();

	if ((type1==NULL || type2==NULL) && type1!=type2) return false;

	if (!type1->Same(type2)) return false;

	int n=params.GetSize();
	if (n!=other.params.GetSize()) return false;
	for (int i=0;i<n;i++)
	{
		Param &t1=*params[i];
		Param &t2=*other.params[i];
		if (!t1.GetType()->Same(t2.GetType())) return false;
	}
	return true;
}

// END Method class implementation....

//Implement Constructor class

Constructor::Constructor(Class *cl, AccessType myaccess):Method(cl, myaccess)
{
	strcpy(name, cl->GetName());
}

void Constructor::set_id(int value)
{
  identifier = value;
}
int Constructor::get_id()
{
  return identifier; 
}

bool Constructor::isDefault()
{
	return (params.GetSize()==0);
}


ObjDesc & Constructor::GetOD()
{
	return od;
}

void Constructor::generate_header_pog(CArrayChar &output, bool interface)
{
	Method::generate_header_pog(output, interface);

  

	if (interface) {
		char str[1024];
		strcpy(str,"\nvoid _popc_constructor");
		output.InsertAt(-1,str,strlen(str));
		GenerateArguments(output, true);
		GeneratePostfix(output,true);
	} 
}

void Constructor::GenerateHeader(CArrayChar &output, bool interface)
{
	Method::GenerateHeader(output, interface);

	if (interface) {
		char str[1024];
		strcpy(str,"\nvoid _paroc_Construct");
		output.InsertAt(-1,str,strlen(str));
		GenerateArguments(output, true);
		GeneratePostfix(output,true);
	}
}

void Constructor::GenerateReturn(CArrayChar &output, bool header)
{
}

void Constructor::GeneratePostfix(CArrayChar &output, bool header)
{
	if (header)
	{
		output.InsertAt(-1,";",1);
		return;
	}

	CArrayBaseClass &baseClass=GetClass()->baseClass;
	int n=baseClass.GetSize();
	if (n)
	{
		CArrayClass bases;
		bases.SetSize(n);
		for (int i=0;i<n;i++)  bases[i]=baseClass[i]->base;
		Class *cl=GetClass();
		CodeFile *prog=cl->GetCodeFile();
		prog->FindAllBaseClass(*cl, bases,true);

		n=bases.GetSize();
		char tmpcode[10240];
		strcpy(tmpcode," : ");
		for (int j=0;j<n;j++)
		{
			strcat(tmpcode,bases[j]->GetName());
			if (j<n-1) strcat(tmpcode,"(_paroc_nobind) ,");
			else  strcat(tmpcode,"(_paroc_nobind)");
		}
		output.InsertAt(-1,tmpcode,strlen(tmpcode));
	}
}

void Constructor::GenerateClientPrefixBody(CArrayChar &output)
{
	char tmpcode[10240];


	/**
	 * Asynchronous Parallel Object Creation (APOA)
	 * The code below is generated to support the APOA in POP-C++ application. 
	 */

	if(!GetClass()->IsCoreCompilation() && GetClass()->IsAsyncAllocationDisable()){
  	strcpy(tmpcode, "\n");
		output.InsertAt(-1, tmpcode, strlen(tmpcode));
		od.Generate(tmpcode);	// Generates the object description
		output.InsertAt(-1, tmpcode, strlen(tmpcode));
#ifdef __APPLE__		
		strcpy(tmpcode,"\n  pthread_attr_t attr;\n  pthread_attr_init(&attr);\n  pthread_attr_setdetachstate(&attr, 1);");
#else
		strcpy(tmpcode,"\n  pthread_attr_t attr;\n  pthread_attr_init(&attr);\n  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);");
#endif		
		output.InsertAt(-1, tmpcode, strlen(tmpcode));
		
		sprintf(tmpcode,"\n  pthread_args_t_%d *arguments = (pthread_args_t_%d *) malloc(sizeof(pthread_args_t_%d));\n  %s* ptr = static_cast<%s*>(this);\n  arguments->ptr_interface = ptr;\n", get_id(), get_id(), get_id(), GetClass()->GetName(), GetClass()->GetName());
		output.InsertAt(-1, tmpcode, strlen(tmpcode));
		
		int nb = params.GetSize();
		for (int j = 0; j < nb; j++)
		{
			Param &p = *(params[j]);
			sprintf(tmpcode, "  arguments->");
			strcat(tmpcode, p.GetName());
			strcat(tmpcode, " = ");
			strcat(tmpcode, p.GetName());
      strcat(tmpcode, ";\n");  
      output.InsertAt(-1, tmpcode, strlen(tmpcode));       
		}
		
		sprintf(tmpcode, "  int ret;\n  ret = pthread_create(&_popc_async_construction_thread, &attr, %s_AllocatingThread%d, arguments);\n", GetClass()->GetName(), get_id());	
		output.InsertAt(-1, tmpcode, strlen(tmpcode));
		strcpy(tmpcode, "  if(ret != 0) {\n    printf(\"Thread creation failed\\n\");\n    perror(\"pthread_create\");\n    pthread_attr_destroy(&attr);\n    return;\n  }\n  pthread_attr_destroy(&attr);\n");
		output.InsertAt(-1, tmpcode, strlen(tmpcode));
	} else if(!GetClass()->is_collective()) { // End of APOA Support
		/**
		 * Standard parallel object allocation
		 */
		strcpy(tmpcode, "");
		od.Generate(tmpcode);	// Generates the object description
		strcat(tmpcode,"\nAllocate();");	// Generates call to the Allocate method of paroc_interface
		output.InsertAt(-1, tmpcode, strlen(tmpcode));

		// Generates invocation to the constructor of the remote object
		strcpy(tmpcode,"\n_paroc_Construct(");	
		int nb = params.GetSize();
		for (int j=0;j<nb;j++)
		{
			Param &p=*(params[j]);
			strcat(tmpcode,p.GetName());
			if (j<nb-1) strcat(tmpcode,", ");
		}
		strcat(tmpcode,");\n");
		output.InsertAt(-1, tmpcode, strlen(tmpcode)); 
		// End of constructor invocation
	} else {
	  // Generate the object description in the interface constructor
	  sprintf(tmpcode, "\n  _popc_selected_constructor_id = %d;", id); 
		output.InsertAt(-1, tmpcode, strlen(tmpcode)); 
			  
		strcpy(tmpcode, "\n  ");
		od.Generate(tmpcode);
		strcat(tmpcode, "\n"); 
		output.InsertAt(-1, tmpcode, strlen(tmpcode)); 
		
		// Save constructor parameters for group initialization
		int nb = params.GetSize();		
		for (int j=0;j<nb;j++) {
			Param &p = *(params[j]);
			sprintf(tmpcode, "  _popc_constructor_%d_%s = %s;\n", id, p.GetName(), p.GetName()); 
			output.InsertAt(-1, tmpcode, strlen(tmpcode));       
		}
	}

	
	strcpy(tmpcode,"}\n");
	output.InsertAt(-1, tmpcode, strlen(tmpcode));
	
	
	// APOA Code generation
	if(!GetClass()->IsCoreCompilation() && GetClass()->IsAsyncAllocationDisable()){
	  sprintf(tmpcode,"\n// This code is generated for Asynchronous Parallel Object Allocation support for the object %s\n", GetClass()->GetName());
		output.InsertAt(-1,tmpcode,strlen(tmpcode));		
		sprintf(tmpcode,"extern \"C\"\n{\n  void* %s_AllocatingThread%d(void* arg)\n  {\n", GetClass()->GetName(), get_id());
		output.InsertAt(-1,tmpcode,strlen(tmpcode));		
		
		sprintf(tmpcode,"    pthread_args_t_%d *arguments = (pthread_args_t_%d*)arg;\n", get_id(), get_id());
		output.InsertAt(-1,tmpcode,strlen(tmpcode));
		
		sprintf(tmpcode,"    %s* _this_interface = static_cast<%s*>(arguments->ptr_interface);\n",GetClass()->GetName(), GetClass()->GetName());
		output.InsertAt(-1,tmpcode,strlen(tmpcode));
		
		int nb = params.GetSize();
		for (int j = 0; j < nb; j++)
		{
			Param &p=*(params[j]);
		  sprintf(tmpcode, "%s %s = arguments->%s;\n", p.GetType()->GetName(), p.GetName(), p.GetName()); 
			output.InsertAt(-1, tmpcode, strlen(tmpcode));
		}
		  
		sprintf(tmpcode, "    try{\n      _this_interface->Allocate();\n      _this_interface->_paroc_Construct("); 

		for (int j=0;j<nb;j++) {
			Param &p = *(params[j]);
			strcat(tmpcode,p.GetName());
			if (j<nb-1) strcat(tmpcode,", ");
		}
		
		strcat(tmpcode, ");\n");
		output.InsertAt(-1, tmpcode, strlen(tmpcode)); 
		
		sprintf(tmpcode, "    } catch(paroc_exception* ex) {\n      printf(\"Async allocation: %%s\", ex->what()); \n    }\n   free(arg);\n  return 0;\n  }\n}\n");
		output.InsertAt(-1,tmpcode,strlen(tmpcode));			
	}	
	
	

  if(!GetClass()->is_collective()) {	
  	sprintf(tmpcode, "\nvoid %s::_paroc_Construct", GetClass()->GetName());
	  output.InsertAt(-1, tmpcode, strlen(tmpcode));
  } else { 
   	//sprintf(tmpcode, "\nvoid %s::construct_remote_object() {\n  _popc_constructor(", GetClass()->GetName());
	  //output.InsertAt(-1, tmpcode, strlen(tmpcode));    
	  
	  
	  
	  // Place saved constructor arguments
/*		int nb = params.GetSize();		
		for (int j = 0; j < nb; j++) {
			Param &p = *(params[j]);
			sprintf(tmpcode, "_popc_constructor_%d_%s", get_id(), p.GetName()); 
			output.InsertAt(-1, tmpcode, strlen(tmpcode));       
			if (j < nb-1) {
  			strcpy(tmpcode, ", ");
  			output.InsertAt(-1, tmpcode, strlen(tmpcode));
  		}
		}
	  
	  
	  strcpy(tmpcode, ");\n}\n");
	  output.InsertAt(-1, tmpcode, strlen(tmpcode)); 	 */ 
  
  	sprintf(tmpcode,"\nvoid %s::_popc_constructor",GetClass()->GetName());
	  output.InsertAt(-1, tmpcode, strlen(tmpcode));    
  }    	  
  
  GenerateArguments(output,false);
  

  
  strcpy(tmpcode,"\n{");
  output.InsertAt(-1, tmpcode, strlen(tmpcode));
}

//Implement Destructor class

Destructor::Destructor(Class *cl, AccessType myaccess): Method(cl, myaccess)
{
}

void Destructor::GenerateClient(CArrayChar &output)
{
	//Ignore the destructor of the interface....
}

void Destructor::GenerateReturn(CArrayChar &output, bool header)
{
	if (header) output.InsertAt(-1,"~",1);
	else output.InsertAt(-1,"\n~",2);
}

// BEGIN  BaseClass implementation....

BaseClass::BaseClass(Class *name, AccessType basemode, bool onlyVirtual)
{
	base=name;
	type=basemode;
	baseVirtual=onlyVirtual;
}
// END  BaseClass implementation....

