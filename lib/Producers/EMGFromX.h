//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//
 
#ifndef EMGFromX_h
#define EMGFromX_h

#include <vector>

class EMGFromX 
{
  public:
    virtual ~EMGFromX();
    virtual void operator()() {};
    void pushEmgBack(const std::vector<double>& newEmgToPush);
    void updateEmg(const std::vector<double>& currentEmgData, double currentTime);  
};

#endif
