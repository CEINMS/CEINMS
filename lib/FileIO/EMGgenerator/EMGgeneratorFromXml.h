//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef EMGgeneratorFromXml_h
#define EMGgeneratorFromXml_h

#include <string>
#include <vector>
//#include <cstdarg>
#include <boost/any.hpp>

struct Functor {
    unsigned num_arguments;
    Functor() {}

    virtual void operator()(boost::any, boost::any) {}
    virtual void operator()(boost::any) {};
};


template<typename R, typename A>
struct Sum : public Functor {
    Sum() {};
    void operator()(boost::any r, boost::any a) {
        R* out = boost::any_cast<R*>(r);
        A  arguments = boost::any_cast<A>(a);
        R value = 0;
        for(typename A::const_iterator it(arguments.begin()); it != arguments.end(); ++it)
            value += *it;
        *out = value;
    }
};


template<typename R, typename A>
struct Mean : public Functor {
    Mean() {};
    void operator()(boost::any r, boost::any a) {
        R* out = boost::any_cast<R*>(r);
        A  arguments = boost::any_cast<A>(a);
        R value = 0;
        for(typename A::const_iterator it(arguments.begin()); it != arguments.end(); ++it)
            value += *it;
        *out = (value/arguments.size());
    }
};


template<typename R, typename A>
struct Assign : public Functor {
    Assign() {};
    void operator()(boost::any r, boost::any a) {
        R* out = boost::any_cast<R*>(r);
        A  argument = boost::any_cast<A>(a);
        *out = argument;
    }
};



template<typename R>
struct Nil : public Functor {
    Nil() {};
    void operator()(boost::any r) {
        R* out = boost::any_cast<R*>(r);
        *out = .0;
    }
};


class EMGgeneratorFromXml{
    
public:
  EMGgeneratorFromXml(const std::string& xmlFilename  = "cfg/xml/emgGenerator.xml"); 
  inline bool checkFromMusclesNames(const std::vector<std::string>& fromMusclesNames) const {
    return ( (fromMusclesNames == fromMusclesNames_) );
  }
  void convert(const std::vector<double>& fromEMG, std::vector<double>& toEMG) const;
  const std::vector<std::string>&  getMusclesNames() const {return toMusclesNames_;};
  int getNoFromMuscles() const { return noFromMuscles_;}
  int getNoToMuscles() const {return noToMuscles_;}
private:
    void setEmgMappingOperation(unsigned index, const std::string& operation);
    void setEmgMappingArguments(unsigned index, const std::vector<std::string>& muscleNames);
  unsigned noFromMuscles_;
  unsigned noToMuscles_;
  std::vector<Functor*>   functors_;
  std::vector< std::vector<unsigned> > argumentIndex_;
  std::vector<std::string> fromMusclesNames_;
  std::vector<std::string> toMusclesNames_;
  
};

#endif
