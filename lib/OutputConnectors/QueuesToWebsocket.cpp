#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "ceinms/QueuesToWebsocket.h"
#include "yabbi.pb.h"
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace ceinms {
	void QueuesToWebsocket::operator()() {
	
		try
		{		
//			std::string nodesFilename = "C:\\Users\\s2849511\\coding\\unversioned\\boostwebsockettest\\data\\nodes.csv";
//			std::string strainsFilename = "C:\\Users\\s2849511\\coding\\unversioned\\boostwebsockettest\\data\\strains.csv";

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
					std::move(socket), inputPointCloud_) }.detach();
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
		}
	
	}
	 
	void QueuesToWebsocket::doSession(tcp::socket& socket, ScalarVectorVectorQueue& inputPointCloud)
	{
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
			for (;;)
			{
				// Read a message
				yabbi::ClientRequest clientRequest;
				receiveMessage(clientRequest, ws);

				switch (clientRequest.request()) {
				case yabbi::ClientRequest_Request::ClientRequest_Request_CHANNELS:
				{
					std::cout << "Requesting channels" << std::endl;
					std::vector<std::string> names;
					std::vector<int> dims;
					getAvailableChannels(names, dims);
					yabbi::AvailableChannels availableChannels;
					for (int i(0); i < names.size(); ++i) {
						availableChannels.add_dim(dims[i]);
						availableChannels.add_names(names[i]);
					}
					availableChannels.set_nchannels(names.size());
					sendMessage(availableChannels, ws);
					break;
				}
				case yabbi::ClientRequest_Request::ClientRequest_Request_DATA:
				{
					std::cout << "Requesting data" << std::endl;
					int n = clientRequest.nframes();
					for (int i(0); i < n; ++i) {
						auto dataFrameSet = getDataFrameSet();
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
		strainsQueue.unsubscribe();

	}
}