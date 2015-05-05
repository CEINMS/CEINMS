/* -------------------------------------------------------------------------- *
 * CEINMS is a standalone toolbox for neuromusculoskeletal modelling and      *
 * simulation. CEINMS can also be used as a plugin for OpenSim either         *
 * through the OpenSim GUI or API. See https://simtk.org/home/ceinms and the  *
 * NOTICE file for more information. CEINMS development was coordinated       *
 * through Griffith University and supported by the Australian National       *
 * Health and Medical Research Council (NHMRC), the US National Institutes of *
 * Health (NIH), and the European Union Framework Programme 7 (EU FP7). Also  *
 * see the PROJECTS file for more information about the funding projects.     *
 *                                                                            *
 * Copyright (c) 2010-2015 Griffith University and the Contributors           *
 *                                                                            *
 * CEINMS Contributors: C. Pizzolato, M. Reggiani, M. Sartori,                *
 *                      E. Ceseracciu, and D.G. Lloyd                         *
 *                                                                            *
 * Author(s):                                                                 *
 *                                                                            *
 * CEINMS is licensed under the Apache License, Version 2.0 (the "License").  *
 * You may not use this file except in compliance with the License. You may   *
 * obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.*
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */
 
#ifndef ceinms_ElasticTendon_h
#define ceinms_ElasticTendon_h
#include "Curve.h"
#include <string>


// class for the initial fiber legth computation. #Revised#
namespace ceinms {
    class LDFM {

    public:
        typedef Curve<CurveMode::Offline> CurveOffline;
        LDFM()  {}
        LDFM(double optimalFiberLength,
            double pennationAngle,
            double tendonSlackLength,
            double percentageChange,
            double damping,
            double maxIsometricForce,
            double strengthCoefficient,
            const CurveOffline& activeForceLengthCurve,
            const CurveOffline& passiveForceLengthCurve,
            const CurveOffline& forceVelocityCurve,
            const CurveOffline& tendonForceStrainCurve);
        virtual ~LDFM() {}


        void   setInitConditions(double muscleTendonLength, double muscleTendonVelocity, double activation);
        double estimateFiberLength();
        double oldEstimation(double muscleTendonLength, double muscleTendonVelocity, double activation);

    private:
        void   updateInitConditions();
        void computeMuscleTendonForce();
        double computePennationAngle() const;


        //** parameters from MTU
        double optimalFiberLength_;
        double pennationAngle_;
        double tendonSlackLength_;
        double percentageChange_;
        double damping_;
        double maxIsometricForce_;
        double strengthCoefficient_;
        CurveOffline activeForceLengthCurve_;
        CurveOffline passiveForceLengthCurve_;
        CurveOffline forceVelocityCurve_;
        CurveOffline tendonForceStrainCurve_;
        //**


        //** initial condition parameters
        double activation_;
        double muscleTendonLength_;
        double muscleTendonVelocity_;
        //**


        //** variables used for the LDFM method
        double muscleTendonForce_;
        double fiberLength_;
        double fiberVelocity_;
        double fiberStiffness_;
        double tendonLength_;
        double tendonVelocity_;
        double tendonStiffness_;
        double optimalFiberLengthAtT_;
        //**
    };


    template <CurveMode::Mode mode>
    class ElasticTendon {

    public:
        typedef Curve<CurveMode::Offline> CurveOffline;
        typedef Curve<mode> CurveT;
        ElasticTendon();
        ElasticTendon(std::string id_);
        ElasticTendon(double optimalFiberLength,
            double pennationAngle,
            double tendonSlackLength,
            double percentageChange,
            double damping,
            double maxIsometricForce,
            double strengthCoefficient,
            double maxContractionVelocity,
            const CurveOffline& activeForceLengthCurve,
            const CurveOffline& passiveForceLengthCurve,
            const CurveOffline& forceVelocityCurve,
            const CurveOffline& tendonForceStrainCurve
            );
        virtual ~ElasticTendon();

        ElasticTendon(const ElasticTendon& orig);
        ElasticTendon& operator=(const ElasticTendon& orig);
        void operator() (double x, const std::vector<double> &y, std::vector<double> &dydx);
        void setParametersToComputeForces(double optimalFiberLength,
            double pennationAngle,
            double tendonSlackLength,
            double percentageChange,
            double damping,
            double maxIsometricForce,
            double strengthCoefficient,
            double maxContractionVelocity);

        void setMuscleTendonLength(double muscleTendonLength, double activation, double time);
        void updateFiberLengthUsingNewActivation(double activation, double time);
        double getFiberLength() const { return fiberLength_; }
        void setStrengthCoefficient(double strengthCoefficient);
        void setTendonSlackLength(double tendonSlackLength);
        void setPennationAngle(double pennationAngle);
        void setMaxIsometricForce(double maxIsometricForce);
        void setMaxContractionVelocity(double maxContractionVelocity);
        void setCurves(const CurveOffline& activeForceLengthCurve,
            const CurveOffline& passiveForceLengthCurve,
            const CurveOffline& forceVelocityCurve,
            const CurveOffline& tendonForceStrainCurve);

        void resetState();
        void setTolerance(double tolerance = 0.0){};

    private:
        double getInitialFiberLength(double time);
        double computeFiberVelocityAtT(double muscleTendonLengthAtT, double activationAtT, double muscleTendonVelocityAtT, double fiberLength, double lmtTimescale);
        double computeNormFiberVelocity(double activation, double activeForce, double velocityDependentForce);

        double getFiberLengthRKF();

        //** Parameters from the MTU
        double optimalFiberLength_;
        double pennationAngle_;
        double tendonSlackLength_;
        double percentageChange_;
        double damping_;
        double maxIsometricForce_;
        double strengthCoefficient_;
        double maxContractionVelocity_; //TODO actually use this parameter...
        CurveOffline activeForceLengthCurve_;
        CurveOffline passiveForceLengthCurve_;
        CurveOffline forceVelocityCurve_;
        CurveOffline tendonForceStrainCurve_;
        CurveT muscleTendonLengthTrace_;
        //**

        double testTime_;
        //    Curve muscleTendonLengthTrace_; //verificare se questa serve in altre parti del tendine elastico
        double muscleTendonLength_;     //questa va passato dalla classe muscle -> vedere se si puo' evitare di farlo membre
        double muscleTendonLengthT1_;
        double muscleTendonVelocity_;   //questo viene derivato dall lunghezza mt -> come sopra
        double muscleTendonVelocityT1_;
        double fiberLength_;            //calcolato nella classe
        double fiberLengthT1_;          //valore della fiberLength al passo precedente
        double fiberVelocity_;
        double muscleTendonForce_;
        double activation_;             // da passare in qualche modo alla classe, potrebbe servire anche l'attivazione passata
        double activationT1_;
        double timescale_;          //da sistemare il valore hardcoded
        double lmtTime_;                   // da passare alla classe, forse si puo' evitare di utilizzare una variabile membro
        double lmtTimeT1_;
        unsigned nLmt_;

        std::string id_;
    };
}

#endif
