#ifndef ceinms_GeneralisedCoordinatesData_h
#define ceinms_GeneralisedCoordinatesData_h

#include <vector>

//TODO: add checks for different vecotr sizes and stuff
class GeneralisedCoordinatesData {
private:
    size_t nCoordinates_;
    std::vector<double> q_, qd_, qdd_;

public:
    GeneralisedCoordinatesData(size_t nCoordinates) :
        nCoordinates_(nCoordinates),
        q_(nCoordinates, .0),
        qd_(nCoordinates, .0),
        qdd_(nCoordinates, .0)
    {}

    size_t getNCoordinates() const { return nCoordinates_; }

    void setQ(const std::vector<double>& q) {
        q_ = q;
    }
    void setQd(const std::vector<double>& qd) {
        qd_ = qd;
    }
    void setQdd(const std::vector<double>& qdd) {
        qdd_ = qdd;
    }

    const std::vector<double>& getQ() const { return q_; }
    const std::vector<double>& getQd() const { return qd_; }
    const std::vector<double>& getQdd() const { return qdd_; }
    void getQ(std::vector<double>& q) const { q = q_; }
    void getQd(std::vector<double>& qd) const { qd = qd_; }
    void getQdd(std::vector<double>& qdd) const { qdd = qdd_; }


};

#endif
