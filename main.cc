#include <iostream>
#include <chrono>
#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include <numeric>
#include <memory>
#include <fstream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>

// #include "NonCopyable.h"
#include "Thread.h"

#include "EventLoop.h"
#include "HttpServer.h"


// using namespace std;
using namespace mew;


// not a serious page fetcher
// test only
std::string GetTestPage(std::string_view file_path) {
    std::ifstream fin { std::string(file_path) };
    std::stringstream buf;
    buf << fin.rdbuf();
    fin.close();
    return buf.str();
}


void HttpResponseCallback(const HttpRequest& request, HttpResponse& response) {
    if (request.method() != kGet) {   // can only handle GET method so far...
        response.SetStatusCode(k400BadRequest);
        response.SetStatusMessage("Bad Request");
        response.SetCloseConnection(true);
        return;
    }    
    const std::string& path = request.path(); 
    if (path == "/") {
        response.SetStatusCode(k200OK);
        response.SetBodyType("text/html");
        response.SetBody(GetTestPage("./web/home.html"));
    } 
    else if (path == "/hello") {
        response.SetStatusCode(k200OK);
        response.SetBodyType("text/html");
        response.SetBody("hey, how are you doing!\n");
    }
    else if (path == "/home") {
        response.SetStatusCode(k200OK);
        response.SetBodyType("text/html");
        response.SetBody(GetTestPage("./web/home.html"));
    }
    else {
        response.SetStatusCode(k404NotFound);
        response.SetStatusMessage("Not Found");
        response.SetCloseConnection(true);
        return;
    }
}



int main(int argc, char *argv[]) {
    current_thread::SetupInfo( [](){}, "main" );  // setup thread information for logging system
    LOG_INIT();
    INFO("log system initialized!");

    EventLoop loop;
    INFO("base loop created!");
    

    HttpServer server { &loop, Address{ 8080 }, true };
    server.SetThreadnums( 4 );
    server.SetResponseTask( HttpResponseCallback );
    INFO("http server created!");
    
    server.Start();
    INFO("http server started!");

    loop.Loop();

    return 0;
}
