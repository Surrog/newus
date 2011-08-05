
#include <boost/asio.hpp>

#include <iostream>

#include "RequestForge.hpp"
#include "FeedManager.hpp"

RequestForge::RequestForge(FeedManager& owner) : _socket(owner.getIo_service()) {
}

bool RequestForge::ConnectTo(const std::string& addr) {
    boost::asio::ip::tcp::resolver resolver(_socket.get_io_service());

    boost::asio::ip::tcp::resolver::query query(addr, "http");
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;

    boost::system::error_code error = boost::asio::error::host_not_found;
    while (endpoint_iterator != end && error) {
        _socket.close();
        _socket.connect(*endpoint_iterator, error);
        ++endpoint_iterator;
    }
    bool result = false;
    if (_socket.is_open() && !error) {
        result = true;
    }
    return result;
}

bool RequestForge::Connected() const {
    return _socket.is_open();
}

void RequestForge::SetGetRequest(std::ostream& request_stream, const std::string& host, const std::string& path) {
    request_stream << "POST " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";
}

bool RequestForge::GetRequest(const std::string& host, const std::string& path) {
    bool result = false;

    boost::system::error_code error;
    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    SetGetRequest(request_stream, host, path);

    boost::asio::write(_socket, request, boost::asio::transfer_all(), error);
    if (!error) {
        result = HandleResponse();
    }
    return result;
}

bool RequestForge::HandleResponse() {
    bool result = false;
    boost::asio::streambuf response;
    boost::asio::read_until(_socket, response, "\r\n");
    std::istream response_stream(&response);

    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    response_stream >> status_message;
    std::getline(response_stream, status_message);

    if (ValidResponse(response_stream, http_version)) {
        if (status_code == 200) {
            result = DisplayBody(response, response_stream);
        } else {
            std::cout << "http version" << http_version << std::endl;
            std::cout << "status code" << status_code << std::endl;
            std::cout << "status message" << status_message << std::endl;
            if (status_code > 300) {
                result = DisplayBody(response, response_stream);
            }
        }
    } else {
        std::cout << "Invalid response" << std::endl;
    }
    return result;
}

bool RequestForge::DisplayBody(boost::asio::streambuf& response, std::istream& response_stream) {
    boost::asio::read_until(_socket, response, "\r\n\r\n");
    DisplayBodyHeader(response, response_stream);
    return DisplayBodyBody(response, response_stream);
}

bool RequestForge::DisplayBodyHeader(boost::asio::streambuf& response, std::istream& response_stream) {
    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
        std::cout << header << std::endl;
    std::cout << std::endl;
    if (response.size() > 0)
        std::cout << &response;
    return true;
}

bool RequestForge::DisplayBodyBody(boost::asio::streambuf& response, std::istream& response_stream) {
    bool result = false;
    boost::system::error_code error;
    while (boost::asio::read(_socket, response, boost::asio::transfer_at_least(1), error)) {
        std::cout << &response;
    }
    if (error == boost::asio::error::eof) {
        result = true;
    } else {
        std::cout << "error " << error.message() << std::endl;
    }
    return result;
}