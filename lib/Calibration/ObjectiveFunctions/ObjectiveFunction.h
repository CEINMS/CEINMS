//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ObjectiveFunction_h
#define ObjectiveFunction_h

class ObjectiveFunction {

public:
    virtual ~ObjectiveFunction() {}
    virtual void   evalFp() = 0;
    virtual bool   isFacceptable() = 0;
    virtual void   updateF() = 0;
    virtual bool   isFoptAcceptable() = 0;
    virtual void   updateFopt() = 0;
    virtual bool   terminate() = 0;
    virtual void   updateFandFlatest() = 0;
    virtual void   printFs() = 0;
    
};

#endif