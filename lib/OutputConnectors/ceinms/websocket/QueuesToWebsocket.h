#ifndef ceinms_QueuesToWebsocket_h
#define ceinms_QueuesToWebsocket_h
#include "ceinms/concurrency/Queue.h"
#include "ceinms/concurrency/Latch.h"
#include "ceinms/queues/ScalarVectorVectorQueue.h"
#include <string>
#include <vector>
#include <boost/asio/ip/tcp.hpp>
#include <google/protobuf/stubs/common.h>

namespace ceinms {
	// we are going to get this to work with a single queue at the moment
	// but down the line we should be able to subscribe to different queues programmatically
	class QueuesToWebsocket {
	public:
		QueuesToWebsocket(
			ScalarVectorVectorQueue& inputPointCloud,
			const std::vector<std::string>& names, 
			std::string address = "127.0.0.1",
			std::string port = "8080") :
			inputPointCloud_(inputPointCloud),
			address_(address), port_(port), names_(names)
			{
			GOOGLE_PROTOBUF_VERIFY_VERSION;
		}
		void operator()();
	private:
		static void doSession(boost::asio::ip::tcp::socket& socket, 
			ScalarVectorVectorQueue& inputPointCloud, const std::vector<std::string>& names);
		static void getAvailableChannels(const std::vector<std::string>& names, int dimensions);
		ScalarVectorVectorQueue& inputPointCloud_;
		std::vector<std::string> names_;
		std::string address_, port_;
	};
}

#endif
