#include <cmath>
#include <iostream>


class Barrier {
public:
    Barrier(double xL, double xR, double weight, double exponent) :
        xL_(xL), xR_(xR), weight_(weight), exponent_(exponent) {}
    double getPenalty(double x) const {
        return getRightSidePenalty(x) + getLeftSidePenalty(x);
    }

private:
    double xL_, xR_, weight_, exponent_;

    double getLeftSidePenalty(double x) const {
        double penalty(.0);
        if (x < xL_)
            penalty = weight_*std::pow((xL_ - x), exponent_);
        return penalty;
    }

    double getRightSidePenalty(double x) const {
        double penalty(.0);
        if (x > xR_)
            penalty = weight_*std::pow((x - xR_), exponent_);
        return penalty;
    }

};

int main() {


    Barrier barrier(2, 4, 1, 2);
    for (unsigned i(0); i < 100; ++i) {
        double x = static_cast<double>(i)*0.1;
        std::cout << barrier.getPenalty(x) << std::endl;
    }

    return 0;
}
