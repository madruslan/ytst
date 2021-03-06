#include <utility>

#include "log.h"
#include "http_response_writer.h"

namespace ytst {
	void HttpResponseWriter::add_callback(std::function<void()> notify) {
		writer.add_callback(notify);
	}

	int HttpResponseWriter::write_response(int code, bool chunked, std::string& body) {
		auto buf = std::make_shared<Buffer>(body.c_str(), body.length());
		write_header(code, chunked, buf->nbytes()+2);
		write_buffer(code, chunked, buf);

		return write_last_chunk();
	}

	int HttpResponseWriter::write_buffer(int code, bool chunked, std::shared_ptr<Buffer> buf) {
		if (chunked) {
			write_chunked_buffer(buf);
		} else {
			writer.write_buffer(buf);
		}
		
		return 0;
	}

	int HttpResponseWriter::write_buffer(std::shared_ptr<Buffer> buf) {
		return write_buffer(HttpResponse::STATUS_OK, chunked, buf);
	}

	int HttpResponseWriter::write_last_chunk() {
		if (chunked) {
			return writer.write_buffer(std::make_shared<Buffer>("0\r\n\r\n", 5));
		}

		return writer.write_buffer(std::make_shared<Buffer>("\r\n", 2));
	}

	int HttpResponseWriter::write_header(int code, bool chunked, ssize_t len) {
		this->chunked = chunked;

		if (chunked) {
			header["Transfer-Encoding"] = "chunked";
		} else if (len > 0) {
			header["Content-Length"] = std::to_string(len);
		}

		std::string code_name;
		int res = HttpResponse::get_name(code, code_name);
		if (res < 0) {
			LOG(logWARNING) << "Invalid HTTP code";

			return res;
		}
		
		std::string headers;
		for (auto& h : header) {
			headers = headers + h.first + ": " + h.second + "\r\n";
		}

		std::string header_response =
			"HTTP/1.1 " + std::to_string(code) + " " + code_name + "\r\n"
			+ headers + "\r\n";

		auto buf = std::make_shared<Buffer>(header_response.c_str(), header_response.length());
		writer.write_buffer(buf);

		LOG(logDEBUG) << "HTTP headers sent";

		return 0;
	}

	std::shared_ptr<Buffer> HttpResponseWriter::get_buffer() {
		return writer.get_buffer();
	}

	bool HttpResponseWriter::has_buffer() {
		return writer.has_buffer();
	}

	int HttpResponseWriter::write_chunked_buffer(std::shared_ptr<Buffer> buf) {
		char lbuf[16];
		if (buf->nbytes() > UINT_MAX) {
			return -1;
		}
		int lsize = snprintf(lbuf, 16, "%x\r\n", static_cast<unsigned int>(buf->nbytes()));
		writer.write_buffer(std::make_shared<Buffer>(lbuf, lsize));
		writer.write_buffer(buf);
		writer.write_buffer(std::make_shared<Buffer>("\r\n", 2));

		return 0;
	}

	HttpResponseWriter::HttpResponseWriter() { }
}
