#include <stdio.h>
#include "ParObject.ph"

ParObject::ParObject()
{
	printf("Heritparam2: Creating object ParObject on localhost %s\n", GetAccessPoint().GetAccessString());
}

ParObject::ParObject(POPString machine)
{
	printf("Heritparam2: Creating object ParObject on %s\n", GetAccessPoint().GetAccessString());
}

ParObject::ParObject(float f)
{
	printf("Heritparam2: Creating object ParObject with power %f\n",f);
}

ParObject::~ParObject()
{
	printf("Heritparam2: Destroying the object ParObject... %d,%d\n", theData.GetInternalData(), theData.GetMyData());
}

void ParObject::SetData(HeritData data)
{
	theData=data;
}

HeritData ParObject::GetData()
{
	return theData;
}

@pack(ParObject);
