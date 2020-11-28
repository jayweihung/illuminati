#include <example/common/root_certificates.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

int main(int argc, char** argv)
{

	auto const host = argv[1];
	auto const port = argv[2];
	auto const target = argv[3];
	//auto const host = "www.sec.gov";
	//auto const target = "/Archives/edgar/data/51143/000110465911057630/ibm-20110930_cal.xml";
	//auto host = "mops.twse.com.tw";
	//auto target = "/server-java/t164sb01?step=3&SYEAR=2020&file_name=tifrs-fr1-m1-ci-cr-2454-2020Q3.html";
	net::io_context ioc;
	beast::error_code ec;
	ssl::context ctx(ssl::context::tlsv12_client);
	ctx.set_default_verify_paths();

	load_root_certificates(ctx);

	ctx.set_verify_mode(ssl::verify_peer);

	tcp::resolver resolver(ioc);
	beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

	if(!SSL_set_tlsext_host_name(stream.native_handle(), host)) {
		beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
		throw beast::system_error{ec};
	}


	auto const results = resolver.resolve(host, port);

	beast::get_lowest_layer(stream).connect(results);

	stream.handshake(ssl::stream_base::client);

	http::request<http::empty_body> req{http::verb::get, target, 11};
	req.set(http::field::host, host);
	req.set(http::field::user_agent, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.183 Safari/537.36");
	req.set(http::field::accept, "text/html");

	http::write(stream, req);

	beast::flat_buffer buffer {2000 * 1024};

	http::response<http::dynamic_body> res;
	http::read(stream, buffer, res);

	std::cout << res << std::endl;
	
	stream.shutdown(ec);
	return EXIT_SUCCESS;
}
