#include <vector>
using std::vector;

namespace ceinms {
    namespace Hybrid {
        namespace StaticComputationMode {

            template<typename NMSmodelT>
            Default<NMSmodelT>::Default(NMSmodelT& subject) :
                subject_(subject) {

                subject_.getMuscleForces(forceDataT1_);
                cout << "muscle forces\n";
                for (unsigned i = 0; i < forceDataT1_.size(); ++i)
                    cout << forceDataT1_.at(i) << " ";
                cout << endl;


                subject_.getEmgs(emgDataT1_);
                musclesToUpdate_.assign(subject_.getNoMuscles(), true);

            }


            template<typename NMSmodelT>
            void Default<NMSmodelT>::getMusclesToUpdate() {

                musclesToUpdate_.clear();
                vector<double> currentEMGs;
                subject_.getEmgs(currentEMGs);
                for (unsigned idx = 0; idx < currentEMGs.size(); ++idx)
                    if (currentEMGs.at(idx) != emgDataT1_.at(idx))
                        musclesToUpdate_.push_back(idx);
                emgDataT1_ = currentEMGs;
            }



            template<typename NMSmodelT>
            void Default<NMSmodelT>::getTorques(vector< double >& torques) {

                getMusclesToUpdate();
                subject_.updateState_HYBRID(musclesToUpdate_);
                subject_.getTorques(torques);

                //              std::cout << "currentTorques\n";
                //             for (unsigned i = 0; i < torques.size(); ++i)
                //                 std::cout << torques.at(i) << " ";
                //             std::cout << std::endl;


                /*        vector<double> currentEMGs;

                      std::cout << "currentEMGs\n";
                      subject_.getEmgs(currentEMGs);
                      for (unsigned i = 0; i < currentEMGs.size(); ++i)
                      std::cout << currentEMGs.at(i) << " ";
                      std::cout << std::endl;
                      */
            }

        }
    }
}