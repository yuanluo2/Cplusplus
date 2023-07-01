#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/locale.hpp>
#include <string_view>
#include <vector>
#include <array>

using namespace std::literals::string_view_literals;

struct Request {
    std::string method;
    std::string path;
    std::string http_version;
    std::vector<std::string> headers;
    std::string content;
};

struct Response {
    std::string http_version;
    unsigned int status_code;
    std::string status_msg;
    std::vector<std::string> headers;
    std::string content;
};

// specially for windows platform, because my default chcp is 936.
inline std::string utf8ToGbk(std::string_view str) {
    return boost::locale::conv::between(str.data(), "GBK", "UTF-8");
}

std::string streamBufToString(const boost::asio::streambuf& buf) {
    boost::asio::streambuf::const_buffers_type cbt = buf.data();
    std::string data{ boost::asio::buffers_begin(cbt), boost::asio::buffers_end(cbt) };

    return data;
}

Response http_request(std::string_view host, const Request& req) {
    using namespace boost;
    using namespace boost::asio::ip;

    Response resp;

    try {
        asio::io_context io_context;

        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver{ io_context };
        tcp::resolver::results_type endpoints = resolver.resolve(host, "http"sv);

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket{ io_context };
        asio::connect(socket, endpoints);

        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        asio::streambuf request;
        std::ostream request_stream(&request);

        request_stream << req.method << " " << req.path << " " << req.http_version << "\r\n";

        for (const auto& header : req.headers) {
            request_stream << header << "\r\n";
        }

        request_stream << "\r\n";
        request_stream << req.content;

        // Send the request.
        asio::write(socket, request);

        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        asio::streambuf response;
        asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        response_stream >> resp.http_version;
        response_stream >> resp.status_code;
        std::getline(response_stream, resp.status_msg);

        if (!response_stream || resp.http_version.substr(0, 5) != "HTTP/") {
            std::cout << "Invalid response: " << resp.http_version << "\n";
        }

        if (resp.status_code != 200) {
            std::cout << "Response returned with status code " << resp.status_code << "\n";
        }

        // Read the response headers, which are terminated by a blank line.
        asio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {
            resp.headers.emplace_back(header);
        }

        // Read until EOF, writing data to output as we go.
        boost::system::error_code error;
        while (asio::read(socket, response, asio::transfer_at_least(1), error)) {}

        std::string str = streamBufToString(response);
        resp.content += str;

        if (error != asio::error::eof) {
            throw boost::system::system_error(error);
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return resp;
}

Response https_request(std::string_view host, const Request& req) {
    using namespace boost;
    using namespace boost::asio::ip;

    Response resp;

    try {
        asio::io_context io_context;

        // Create a SSL context that uses the default paths for finding CA certificates:
        asio::ssl::context ssl_context{ asio::ssl::context::method::sslv23 };
        ssl_context.set_default_verify_paths();

        tcp::resolver resolver{ io_context };
        tcp::resolver::results_type endpoints = resolver.resolve(host, "https"sv);

        asio::ssl::stream<boost::asio::ip::tcp::socket> socket{ io_context, ssl_context };
        socket.set_verify_mode(asio::ssl::verify_peer);
        socket.set_verify_callback([](bool preverified, asio::ssl::verify_context& ctx) {
            // The verify callback can be used to check whether the certificate that is
            // being presented is valid for the peer. For example, RFC 2818 describes
            // the steps involved in doing this for HTTPS. Consult the OpenSSL
            // documentation for more details. Note that the callback is called once
            // for each certificate in the certificate chain, starting from the root
            // certificate authority.

            std::array<char, 256> subject_name{};
            X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
            X509_NAME_oneline(X509_get_subject_name(cert), subject_name.data(), subject_name.size());

            // dummy verification
            return true;
            });

        asio::connect(socket.lowest_layer(), endpoints);
        socket.handshake(asio::ssl::stream_base::handshake_type::client);

        // Form the request. We specify the "Connection: close" header so that the
            // server will close the socket after transmitting the response. This will
            // allow us to treat all data up until the EOF as the content.
        asio::streambuf request;
        std::ostream request_stream(&request);

        request_stream << req.method << " " << req.path << " " << req.http_version << "\r\n";

        for (const auto& header : req.headers) {
            request_stream << header << "\r\n";
        }

        request_stream << "\r\n";
        request_stream << req.content;

        // Send the request.
        asio::write(socket, request);

        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        asio::streambuf response;
        asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        response_stream >> resp.http_version;
        response_stream >> resp.status_code;
        std::getline(response_stream, resp.status_msg);

        if (!response_stream || resp.http_version.substr(0, 5) != "HTTP/") {
            std::cout << "Invalid response: " << resp.http_version << "\n";
        }

        if (resp.status_code != 200) {
            std::cout << "Response returned with status code " << resp.status_code << "\n";
        }

        // Read the response headers, which are terminated by a blank line.
        asio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {
            resp.headers.emplace_back(header);
        }

        // Read until EOF, writing data to output as we go.
        boost::system::error_code error;
        while (asio::read(socket, response, asio::transfer_at_least(1), error)) {}

        std::string str = streamBufToString(response);
        resp.content += str;

        if (error != asio::error::eof) {
            throw boost::system::system_error(error);
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return resp;
}
