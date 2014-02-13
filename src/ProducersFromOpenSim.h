#ifndef ProducersFromOpenSim_h
#define ProducersFromOpenSim_h

#include <string>
#include <boost/thread.hpp>

class ProducersFromOpenSim {
public:
	ProducersFromOpenSim(const std::string&  osimModelFilename, 
						 const std::string& ikMotFilename, 
						 const std::vector< std::string>& muscleNames, 
						 const std::vector <std::vector <std::string> >& musclesOnDofs,
						 const std::vector< std::string>& dofNames);
	~ProducersFromOpenSim();
	void join();
private:
	void startLmtMaProducers();
	void startTorqueProducer();
	std::vector<std::string> muscleNames_, dofNames_;
	std::vector <std::vector <std::string> > musclesOnDofs_;
	std::string osimModelFilename_, ikFilename_, grfFilename_;
	boost::thread emgProducerThread_, lmtMaProducerThread_, torqueProducerThread_;

};


#endif