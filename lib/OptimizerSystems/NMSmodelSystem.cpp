#include "NMSmodelSystem.h"
#include <vector>
#include <string>
#include <limits>

//ideally, 
namespace CEINMS {

    template <typename NMSmodelT, typename ModelEvaluatorT, typename ObjectiveFunctionT>
    NMSmodelSystem<NMSmodelT, ModelEvaluatorT, ObjectiveFunctionT>::
        NMSmodelSystem(NMSmodelT& subject,
        const ModelEvaluatorT& modelEvaluator,
        const ObjectiveFunctionT& objectiveFunction,
        const Parameter::ParameterSet& parameterSet) :
        subject_(subject),
        modelEvaluator_(modelEvaluator),
        objectiveFunction_(objectiveFunction),
        parameterInterpreter_(parameterSet),
        f_(std::numeric_limits<double>::max())
    {

        nParameters_ = parameterInterpreter_.getNoParameters();
        //all dofs in the subject by default
        subject_.getDofNames(dofsToCalibrate_);
        parameterInterpreter_.setDofsToCalibrate(dofsToCalibrate_);
        
    }

    //NOTE: when using global o single parameters, only the calibrating dofs are considered
    template <typename NMSmodelT, typename ModelEvaluatorT, typename ObjectiveFunctionT>
    void NMSmodelSystem<NMSmodelT, ModelEvaluatorT, ObjectiveFunctionT>::setDofsToCalibrate(const std::vector<std::string>& dofsToCalibrate) {

        dofsToCalibrate_ = dofsToCalibrate;
        parameterInterpreter_.setDofsToCalibrate(dofsToCalibrate_); 
    }

    template <typename NMSmodelT, typename ModelEvaluatorT, typename ObjectiveFunctionT>
    void NMSmodelSystem<NMSmodelT, ModelEvaluatorT, ObjectiveFunctionT>::getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) const {

        parameterInterpreter_.getUpperLowerBounds(upperBounds, lowerBounds);
    }

    template <typename NMSmodelT, typename ModelEvaluatorT, typename ObjectiveFunctionT>
    void NMSmodelSystem<NMSmodelT, ModelEvaluatorT, ObjectiveFunctionT>::setParameters(const std::vector<double>& x) {
        
        //How I think it should work..
        parameterInterpreter_.setSubjectParameters(x);
    }

    template <typename NMSmodelT, typename ModelEvaluatorT, typename ObjectiveFunctionT>
    std::vector<double> NMSmodelSystem<NMSmodelT, ModelEvaluatorT, ObjectiveFunctionT>::getParameters() {

        return parameterInterpreter_.getSubjectParameters();
    }

    template <typename NMSmodelT, typename ModelEvaluatorT, typename ObjectiveFunctionT>
    void NMSmodelSystem<NMSmodelT, ModelEvaluatorT, ObjectiveFunctionT>::evaluate() {

        //subjcet has the parameters, model evaluator just runs it
        //first option trial-by-trial.. but it cannot do the speedup trick... 
        std::vector<ModelEvaluatorT::EvaluationResults> evalResults;
        for (auto& trial : trialData_) {
            modelEvaluator_.evaluateSubject(subject_, trial);
            //model evaluator save the results in an object that contains all the variables from the simulation
            evalResults.emplace_back(modelEvaluator_.getResults());
        }

        //alternatively use this.. 
        modelEvaluator_.evaluateSubject(subject_, trialData);
        evalResults = modelEvaluator_.getResults();
        //the objective function has the task to compare somehow external data and results of the simulation
        // external data is saved in the object, the results are passed each time
        objectiveFunction_.calculate(evalResults);
        f_ = objectiveFunction_.getValue();
        //need a way to get the breakdown of the errors from the obj function. There could be a member function that returns a vector<double>
        //and another member function that returns a vector<string> that describes the each entry of the error vector.
        //this is necessary for stats/error control.. 
    }




}