#ifndef YTST_ROUTE_HANDLER
#define YTST_ROUTE_HANDLER

#include <map>

#include "http_handler.h"

namespace ytst {
	class HttpRouteHandler : public HttpHandler {
		std::map<std::string, std::unique_ptr<HttpHandler> > handles;

		std::unique_ptr<HttpHandler> get_route(std::string path);
	public:
		void add_route(std::string path, HttpHandler* handle);
		virtual void serve(HttpRequest& request, HttpResponseWriter& writer);
	};
}

#endif