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
