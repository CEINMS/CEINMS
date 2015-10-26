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
* Author(s): E. Ceseracciu, C. Pizzolato, M. Reggiani                        *
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

#ifndef ceinms_LmtMaFromMcbs_h
#define ceinms_LmtMaFromMcbs_h
#include <mcbs/Spline.h>
#include <osimSpliner/SplineSet.h>
#include <memory>
#include <functional>
#include <iostream>
#include <vector>
#include <string>
#include "ceinms/LmtMaFromX.h"
#include "ceinms/concurrency/Queue.h"
#include "ceinms/queues/GeneralisedCoordinatesQueue.h"
#include "ceinms/queues/LmtQueue.h"
#include "ceinms/queues/MultipleMomentArmsQueues.h"
#include "ceinms/concurrency/Latch.h"
#include <map>
#include "ceinms/Mapper.h"

namespace ceinms {

    class SplineSetBase {
    public:
        virtual std::vector<std::string> getDofNames() = 0;
        virtual void getLmt(
            std::vector<double>& lmtValues,
            const std::vector<double>& dofValues) = 0;
        virtual void getMa(
            const std::string& dofName,
            const std::vector< std::string >& musclesConnectedToDofs,
            const std::vector< double >& angleCombinations,
            std::vector <double>& maValues) = 0;
        virtual void evalLmt(
            const std::string& outputDir,
            const std::vector< std::vector< double > >& angleCombinations) = 0;
        virtual void evalMa(
            const std::string& outputDir,
            const std::map< std::string,
            std::vector< std::string > >& musclesConnectedToDofs,
            const std::vector< std::vector< double > >& angleCombinations) = 0;
    };

    template<int N>
    class SplineSetAdapter : public SplineSetBase {
    public:
        SplineSetAdapter() = delete;
        SplineSetAdapter(const std::string& inputDataFilename);
        SplineSetAdapter(
            const std::vector< std::string > dofNames,
            const std::vector< std::string > musclesNames,
            const std::vector<double>& a,
            const std::vector<double>& b,
            const std::vector<int>& n,
            const std::vector<std::vector<double> >& y);
        SplineSetAdapter(const std::string& coeffDir, const std::vector<std::string>& muscleNames);
        SplineSetAdapter(const SplineSetAdapter& orig) = delete;
        virtual std::vector<std::string> getDofNames() override { return splineSet_.getDofNames(); }
        virtual void getLmt(
            std::vector<double>& lmtValues,
            const std::vector<double>& dofValues) override {
            splineSet_.getLmt(lmtValues, dofValues);
        }

        virtual void getMa(
            const std::string& dofName,
            const std::vector< std::string >& musclesConnectedToDofs,
            const std::vector< double >& angleCombinations,
            std::vector <double>& maValues) override {
            splineSet_.getMa(dofName, musclesConnectedToDofs, angleCombinations, maValues);
        }

        virtual void evalLmt(
            const std::string& outputDir,
            const std::vector< std::vector< double > >& angleCombinations) override {
            splineSet_.evalLmt(outputDir, angleCombinations);
        }

        virtual void evalMa(
            const std::string& outputDir,
            const std::map< std::string,
            std::vector< std::string > >& musclesConnectedToDofs,
            const std::vector< std::vector< double > >& angleCombinations) override {
            splineSet_.evalMa(outputDir, musclesConnectedToDofs, angleCombinations);
        }

        static std::shared_ptr<SplineSetAdapter> make(const string& inputDataFilename);
        static std::shared_ptr<SplineSetAdapter> make(
            const std::vector< std::string > dofNames,
            const std::vector< std::string > musclesNames,
            const std::vector<double>& a,
            const std::vector<double>& b,
            const std::vector<int>& n,
            const std::vector<std::vector<double> >& y);
        static std::shared_ptr<SplineSetAdapter> make(const std::string& coeffDir, const std::vector<std::string>& muscleNames);

    private:
        osimSpliner::SplineSet<N> splineSet_;
    };

    std::shared_ptr<SplineSetBase> makeSplineSet(int degree, const std::string& inputDataFilename);
    std::shared_ptr<SplineSetBase> makeSplineSet(int degree,
        const std::vector< std::string > dofNames,
        const std::vector< std::string > musclesNames,
        const std::vector<double>& a,
        const std::vector<double>& b,
        const std::vector<int>& n,
        const std::vector<std::vector<double> >& y);
    std::shared_ptr<SplineSetBase> makeSplineSet(int degree, const std::string& coeffDir, const std::vector<std::string>& muscleNames);

    template<int N>
    std::shared_ptr<SplineSetAdapter<N>> SplineSetAdapter<N>::make(const string& inputDataFilename) {

        return std::shared_ptr<SplineSetAdapter<N>>(new SplineSetAdapter<N>(inputDataFilename));
    }

    template<int N>
    std::shared_ptr<SplineSetAdapter<N>> SplineSetAdapter<N>::make(
        const std::vector< std::string > dofNames,
        const std::vector< std::string > musclesNames,
        const std::vector<double>& a,
        const std::vector<double>& b,
        const std::vector<int>& n,
        const std::vector<std::vector<double> >& y) {

        return std::shared_ptr<SplineSetAdapter<N>>(new SplineSetAdapter<N>(dofNames, musclesNames, a, b, n, y));
    }

    template<int N>
    std::shared_ptr<SplineSetAdapter<N>> SplineSetAdapter<N>::make(const std::string& coeffDir, const std::vector<std::string>& muscleNames) {

        return std::shared_ptr<SplineSetAdapter<N>>(new SplineSetAdapter<N>(coeffDir, muscleNames));
    }

    template<int N>
    SplineSetAdapter<N>::SplineSetAdapter(const std::string& inputDataFilename) :
        splineSet_(inputDataFilename)
    {}

    template<int N>
    SplineSetAdapter<N>::SplineSetAdapter(
        const std::vector< std::string > dofNames,
        const std::vector< std::string > musclesNames,
        const std::vector<double>& a,
        const std::vector<double>& b,
        const std::vector<int>& n,
        const std::vector<std::vector<double> >& y) :
        splineSet_(dofNames, musclesNames, a, b, n, y) {}

    template<int N>
    SplineSetAdapter<N>::SplineSetAdapter(const std::string& coeffDir, const std::vector<std::string>& muscleNames) :
        splineSet_(coeffDir, muscleNames) {}

    template<int N, typename... Args>
    auto wrap(Args... args) -> decltype(SplineSetAdapter<N>::make(args...)){
        return SplineSetAdapter<N>::make(args...);
    }

    //we use this because the second constructor has plenty of arguments
    template<int N>
    std::shared_ptr<SplineSetBase> wrapAlias(
        const std::vector< std::string > dofNames,
        const std::vector< std::string > musclesNames,
        const std::vector<double>& a,
        const std::vector<double>& b,
        const std::vector<int>& n,
        const std::vector<std::vector<double> >& y) {

        return wrap < N,
            const std::vector< std::string >,
            const std::vector< std::string >,
            const std::vector<double>&,
            const std::vector<double>&,
            const std::vector<int>&,
            const std::vector<std::vector<double> >& > (dofNames, musclesNames, a, b, n, y);
    }

    class LmtMaFromMcbs : public LmtMaFromX {
    public:

        //coeffDir contains dofAngles.cfg
        template <typename NMSModelT>
        LmtMaFromMcbs(
            GeneralisedCoordinatesQueue& inputCoordinateQueue,
            LmtQueue& outputLmtQueue,
            MultipleMomentArmsQueues& outputMomentArmsQueue,
            Concurrency::Latch& doneWithSubscriptions,
            Concurrency::Latch& doneWithExecution,
            const NMSModelT& subject,
            const string& coeffDir);
        void operator()() override;

    private:
        std::shared_ptr<SplineSetBase> splineSet_;
        GeneralisedCoordinatesQueue& inputCoordinateQueue_;
        std::map<std::string, std::vector<std::string>> muscleNamesOnDofsMap_;
        Mapper dofMapper_;
    };

    template <typename NMSModelT>
    LmtMaFromMcbs::LmtMaFromMcbs(
        GeneralisedCoordinatesQueue& inputCoordinateQueue,
        ScalarVectorQueue& outputLmtQueue,
        MultipleMomentArmsQueues& outputMomentArmsQueue,
        Concurrency::Latch& doneWithSubscriptions,
        Concurrency::Latch& doneWithExecution,
        const NMSModelT& subject,
        const string& coeffDir) :
        LmtMaFromX(outputLmtQueue, outputMomentArmsQueue, doneWithSubscriptions, doneWithExecution, subject),
        splineSet_(makeSplineSet(dofNames_.size(), coeffDir, musclesNames_)),
        inputCoordinateQueue_(inputCoordinateQueue) {

        subject.getMuscleNamesOnDofs(muscleNamesOnDofsMap_);

        std::vector<string> dofNamesFromSpline(splineSet_->getDofNames());
        std::cout << "dof names from spline ";
        for (auto dof : dofNamesFromSpline)
            std::cout << dof << " ";
        std::cout << std::endl;

        dofMapper_.setNames(dofNames_, dofNamesFromSpline);

        // inputCoordinateQueue_should receive data already formatted correctly because
        // this object doesn't know opensim
    }
}

#endif
