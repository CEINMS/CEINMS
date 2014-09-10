#ifndef Dof_h
#define Dof_h

#include <vector>
#include <list>
#include <string>
#include "MTU.h"
#include "Curve.h"

template<typename Activation, typename Tendon, CurveMode::Mode mode>
class DoF;

template<typename Activation, typename Tendon, CurveMode::Mode mode>
std::ostream& operator<< (std::ostream& output, const DoF<Activation, Tendon, mode>& dof);
/** This class associates muscles to the corrisponding degree of freedom
 *  and compute the torque
*/
template<typename Activation, typename Tendon, CurveMode::Mode mode>
class DoF {
public:
    
    typedef MTU<Activation, Tendon, mode> MTUtype;
    typedef typename std::vector< MTU<Activation, Tendon, mode> >::iterator vectorMTUitr;
    DoF(); 
    DoF(const std::string& id);
    DoF(const DoF& orig);
    virtual ~DoF();
    std::string getName() const {return id_;}
    void addNewMuscle(const typename std::vector<MTUtype>::iterator newMuscle);
    friend std::ostream& operator<< <>(std::ostream& output, const DoF& dof);
    void setMomentArms(const std::vector<double>& momentArms);
    void getMomentArms(std::vector<double>& momentArms) const { momentArms = momentArms_;}
    double getTorque() const {return torque_;}
	double getDofStiffness() const { return dofStiffness_; }
    bool compareMusclesNames (const std::vector<std::string>& muscleNames) const;
    void getMusclesNamesOnDof (std::vector<std::string>& musclesNames) const;
    void updateTorque();
	void updateDofStiffness();
private:
    std::string id_;
    std::vector<  typename std::vector< MTUtype >::iterator > muscles_;
    std::vector<double> momentArms_;
    double torque_;
	double dofStiffness_;
};

#include "DoF.cpp"

#endif

