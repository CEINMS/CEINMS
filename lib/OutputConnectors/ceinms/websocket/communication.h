#ifndef ceinms_communication_h
#define ceinms_communication_h
#include <boost/beast/core.hpp>
#include <string>

namespace ceinms {

	template<typename T, typename U>
	void sendMessage(const T& protoData, U& ws) {
		std::string msg;
		protoData.SerializeToString(&msg);
		sendMessage(msg, ws);
		//	std::cout << "Sent: " << msg << std::endl;

	}

	template<typename U>
	void sendMessage(const std::string& msg, U& ws) {
		ws.write(boost::asio::buffer(std::string(msg)));
	}

	template <typename T, typename U>
	void receiveMessage(T& protoData, U& ws) {
		boost::beast::multi_buffer buffer;
		ws.read(buffer);
		std::stringstream sstream;
		sstream << boost::beast::buffers(buffer.data());
		protoData.ParseFromString(sstream.str());
		//	std::cout << "Received: " << sstream.str() << std::endl;
	}
}
#endif
