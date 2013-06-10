#ifndef policyTemplates_h
#define policyTemplates_h

#include "EMGDataFromFile.h"
#include "EMGgeneratorFrom10To13.h"
#include "EMGgeneratorFrom16To34.h"
#include "EMGgeneratorFrom6To24.h"


template 
class EMGDataFromFile<EMGgeneratorFrom10To13>;

template 
class EMGDataFromFile<EMGgeneratorFrom16To34>;


template 
class EMGDataFromFile<EMGgeneratorFrom6To24>;

#endif
