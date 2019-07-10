#include "ceinms/concurrency/Queue.h"
#include "ceinms/websocket/QueuesToWebsocket.h"
#include "ceinms/queues/ScalarVectorVectorQueue.h"
#include "csv.h"
#include <fstream>

int main() {

	ceinms::ScalarVectorVectorQueue strainQueue;
	ceinms::QueuesToWebsocket queueToWebsocket(strainQueue, { "x","y","z","w" });
//	std::thread websocketThread(std::ref(queueToWebsocket));
//	websocketThread.join();
	queueToWebsocket();
	return 0;

}