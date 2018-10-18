#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "ceinms/websocket/QueuesToWebsocket.h"
#include "ceinms/websocket/communication.h"
#include "ceinms/EndOfData.h"
#include "yabbi.pb.h"
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace ceinms {
	void QueuesToWebsocket::operator()() {
	
		try
		{		
			auto const address = boost::asio::ip::make_address(address_);
			auto const port = static_cast<unsigned short>(std::stoi(port_));

			// The io_context is required for all I/O
			boost::asio::io_context ioc{ 1 };

			// The acceptor receives incoming connections
			tcp::acceptor acceptor{ ioc, {address, port} };
			for (;;)
			{
				// This will receive the new connection
				tcp::socket socket{ ioc };

				// Block until we get a connection
				acceptor.accept(socket);
				std::cout << "Connection accepted" << std::endl;
				// Launch the session, transferring ownership of the socket
				std::thread{ std::bind(
					&doSession,
					std::move(socket), std::ref(inputPointCloud_), names_) }.detach();
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
		}
	
	}
	 
	void QueuesToWebsocket::doSession(tcp::socket& socket,
		ScalarVectorVectorQueue& inputPointCloud, const std::vector<std::string>& names)
	{
		bool runCondition = true;
		auto getDataFrameSet([&inputPointCloud, &names, &runCondition]() {
			auto fillDataFrame([](const std::vector<double>& in, yabbi::DataFrame* out) {
				for (auto& e : in)
					out->add_data(e);
			});
			auto pc(inputPointCloud.pop());
			runCondition = !EndOfData::isEod(pc);
			yabbi::DataFrameSet dataFrameSet;
			yabbi::DataFrame* currentDataFrame;
			for (int i(0); i < pc.data.size(); ++i) {
				currentDataFrame = dataFrameSet.add_dataframe();
				fillDataFrame(pc.data[i], currentDataFrame);
				currentDataFrame->set_channelname(names[i]);
				currentDataFrame->set_time(pc.time);
			}
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
			return dataFrameSet;
		});

		auto getAvailableChannels([&inputPointCloud, &names]() {
			yabbi::AvailableChannels availableChannels;
			for (int i(0); i < names.size(); ++i) {
				availableChannels.add_dim(2048);
				availableChannels.add_names(names[i]);
			}
			availableChannels.set_nchannels(names.size());
			return availableChannels;
		});


		auto printDataFrameSet([](const yabbi::DataFrameSet& dataFrameSet) {
			std::cout << "size: " << dataFrameSet.dataframe_size() << std::endl;
			for (auto &d : dataFrameSet.dataframe()) {
				std::cout << "channelName: " << d.channelname() << "\n";
				std::cout << "       time: " << d.time() << "\n";
				std::cout << "data.size(): " << d.data().size() << "\n";
				//		std::cout << "       data: [";
				//		for (auto &data : d.data())
				//			std::cout << data << " ";
				//		std::cout << "]" << std::endl;
			}
		});

		try
		{
			// Construct the stream by moving in the socket
			websocket::stream<tcp::socket> ws{ std::move(socket) };

			// Accept the websocket handshake
			ws.accept();
			ws.text(false);
			std::cout << "Handshake accepted" << std::endl;
			inputPointCloud.subscribe();
			// change this to a while there is data coming
			while(runCondition)
			{
				// Read a message
				yabbi::ClientRequest clientRequest;
				receiveMessage(clientRequest, ws);

				switch (clientRequest.request()) {
				case yabbi::ClientRequest_Request::ClientRequest_Request_CHANNELS:
				{
					std::cout << "Requesting channels" << std::endl;
					sendMessage(getAvailableChannels(), ws);
					break;
				}
				case yabbi::ClientRequest_Request::ClientRequest_Request_DATA:
				{
					std::cout << "Requesting data" << std::endl;
					int n = clientRequest.nframes();
					for (int i(0); i < n; ++i) {
						auto dataFrameSet(getDataFrameSet());
						if (!runCondition) {
							ws.close("End of data");
							break;
						}
						printDataFrameSet(dataFrameSet);
						sendMessage(dataFrameSet, ws);
					}
					break;
				}
				}
				std::cout << "Done" << std::endl;
			}
		}
		catch (boost::system::system_error const& se)
		{
			// This indicates that the session was closed
			if (se.code() != websocket::error::closed)
				std::cerr << "Error: " << se.code().message() << std::endl;
		}
		catch (std::exception const& e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
		}
		inputPointCloud.unsubscribe();

	}
}