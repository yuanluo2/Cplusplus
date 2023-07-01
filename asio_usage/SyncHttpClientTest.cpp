#include "SyncHttpClient.h"
#include <fstream>

void print_resp(const Response& resp) {
	std::cout << resp.http_version << " " << resp.status_code << " " << resp.status_msg << '\n';

	for (const auto& header : resp.headers) {
		std::cout << header << '\n';
	}

	std::cout << utf8ToGbk(resp.content) << '\n';
}

void test_http_get() {
	Request req;
	req.method = "GET";
	req.path = "/get?name=name";
	req.http_version = "HTTP/1.0";

	req.headers.emplace_back("Accept: */*");
	req.headers.emplace_back("Connection: close");

	auto resp = http_request("httpbin.org", req);
	print_resp(resp);
}

void test_http_post_json() {
	Request req;
	req.method = "POST";
	req.path = "/post";
	req.http_version = "HTTP/1.0";
	req.content = R"({ "name": "yang", "age": 39 })";

	req.headers.emplace_back("Accept: */*");
	req.headers.emplace_back("Content-Type: application/json");
	req.headers.emplace_back("Content-Length: " + std::to_string(req.content.size()));
	req.headers.emplace_back("Connection: close");

	auto resp = http_request("httpbin.org", req);
	print_resp(resp);
}

void test_http_post_form() {
	Request req;
	req.method = "POST";
	req.path = "/post";
	req.http_version = "HTTP/1.0";
	req.content = R"(name=Hatsune%20Miku&age=39&birth=2000/08/31)";

	req.headers.emplace_back("Accept: */*");
	req.headers.emplace_back("Content-Type: application/x-www-form-urlencoded");
	req.headers.emplace_back("Content-Length: " + std::to_string(req.content.size()));
	req.headers.emplace_back("Connection: close");

	auto resp = http_request("httpbin.org", req);
	print_resp(resp);
}

void test_http_download() {
	Request req;
	req.method = "GET";
	req.path = "/uploads/allimg/220211/004115-1644511275bc26.jpg";
	req.http_version = "HTTP/1.1";

	req.headers.emplace_back("Accept: */*");
	req.headers.emplace_back("Host: pic.netbian.com");
	req.headers.emplace_back("Connection: closed");

	auto res = http_request("pic.netbian.com", req);
	
	if (res.status_code == 200) {
		std::ofstream out{ R"(C:\users\10940\desktop\pic.png)", std::ios::binary };
		out << res.content;
		out.close();
		std::cout << "success.\n";
	}
}

void test_https_download() {
	Request req;
	req.method = "GET";
	req.path = "/file/20221130/24739487_204956942123_2.jpg";
	req.http_version = "HTTP/1.1";

	req.headers.emplace_back("Accept: */*");
	req.headers.emplace_back("Host: pic.ntimg.cn");
	req.headers.emplace_back("Connection: closed");

	auto res = https_request("pic.ntimg.cn", req);

	if (res.status_code == 200) {
		std::ofstream out{ R"(C:\users\10940\desktop\pic.png)", std::ios::binary };
		out << res.content;
		out.close();
		std::cout << "success.\n";
	}
}

int main() {
	//test_http_download();
	test_https_download();
}
