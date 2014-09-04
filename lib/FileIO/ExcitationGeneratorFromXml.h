//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ExcitationGeneratorFromXml_h
#define ExcitationGeneratorFromXml_h

#include <string>
#include <vector>

class ExcitationGeneratorFromXml{

struct Component
{
  unsigned index; //index of this component in the inputSignals list
  double weight;
};

public:
  ExcitationGeneratorFromXml(const std::string& xmlFilename  = "excitationGenerator.xml");
  inline bool checkInputSignalsNames(const std::vector<std::string>& inputSignalsNames) const {
    return ( (inputSignalsNames == inputSignalsNames_) );
  }
  void convert(const std::vector<double>& inputSignals, std::vector<double>& excitations) const;
  int getNoInputSignals() const { return noInputSignals_;}
  int getNoExcitations() const {return noExcitations_;}
  void getInputSignalsNames(std::vector<std::string>& inputSignalsNames) const { inputSignalsNames = inputSignalsNames_;}
  void getExcitationsNames(std::vector<std::string>& excitationsNames) const { excitationsNames = excitationsNames_;}
  const std::vector<std::string>& getInputSignalsNames() const { return inputSignalsNames_;}
  const std::vector<std::string>& getExcitationsNames() const { return excitationsNames_;}

private:
  unsigned noInputSignals_;
  unsigned noExcitations_;
  std::vector< std::vector<Component> > weights_;
  std::vector<std::string> inputSignalsNames_;
  std::vector<std::string> excitationsNames_;

};

#endif
