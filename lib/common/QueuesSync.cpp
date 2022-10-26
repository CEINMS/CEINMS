#include "ceinms/QueuesSync.h"

namespace ceinms {
    namespace QueuesSync {

        std::tuple<MuscleForcesFrame, GeneralisedCoordinatesFrame, EstimatedTorquesFrame> syncJointReactionForces(MuscleForcesQueue& inputMuscleForcesQueue, GeneralisedCoordinatesQueue& inputGeneralisedCoordinatesQueue, EstimatedTorquesQueue& inputEstimatedTorquesQueue) {
            MuscleForcesFrame frame1;
            GeneralisedCoordinatesFrame frame2;
            EstimatedTorquesFrame frame3;
            std::optional<MuscleForcesFrame> frame1FromQueue = inputMuscleForcesQueue.pop();
            if (frame1FromQueue) {
                frame1 = frame1FromQueue.value();
                return std::make_tuple(frame1, syncToTime(frame1.time, inputGeneralisedCoordinatesQueue), syncToTime(frame1.time, inputEstimatedTorquesQueue));
            }
            else {
                return std::make_tuple(frame1, frame2, frame3);
            }

        }
    }
}