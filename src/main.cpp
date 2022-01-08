#include <memory>
#include <cstdlib>
#include <restbed>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "base64.h"

using namespace std;
using namespace restbed;
using json = nlohmann::json;

struct Auth
{
    string type;
    string username;
    string password;
};

struct RequiredParameter
{
    string upstreamKey;
    string name;
    vector<string> values;
    string defaultVal;
};

Auth GetAuth(const shared_ptr< Session > session)
{
    Auth ret;
    string authorization = session->get_request()->get_header("Authorization" );
    size_t breakPos = authorization.find(' ');
    ret.type = authorization.substr(0, breakPos);
    if (ret.type != "Basic")
        return ret;
    string b64 = authorization.substr(++breakPos);
    string b64Decoded;
    macaron::Base64::Decode(b64, b64Decoded);
    breakPos = b64Decoded.find(':');
    ret.username = b64Decoded.substr(0,breakPos);
    ret.password = b64Decoded.substr(++breakPos);
//    fprintf(stdout, "%s (%s:%s => %s)\n", authorization.c_str(), ret.type.c_str(), b64.c_str(), b64Decoded.c_str());
    fprintf(stdout, "Username: \"%s\"\nPassword: \"%s\"\n", ret.username.c_str(), ret.password.c_str());
    return ret;
}

void publish_package(const shared_ptr< Session > session )
{
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [ request, contentLength ](const shared_ptr< Session > session, const Bytes & body )
    {
#if 0
        string parameters;
        for (const auto& param : request->get_query_parameters())
        {
            parameters += "{"+param.first+" : "+param.second+"} ";
        }
        fprintf(stdout, "Publishing package with parameters: {\n    %s\n}\nand body (%d): {\n    %.*s\n}\n", parameters.c_str(), contentLength, (int) body.size(), body.data());
#endif

        if (!request->has_header("Authorization"))
        {
            session->close(401, "Publishing requires http basic auth with username and password.");
            return;
        }
        Auth auth = GetAuth(session);
        if (auth.username.empty() || auth.password.empty())
        {
            session->close(401, "Publishing requires http basic auth with username and password.");
            return;
        }

        json requestData = json::parse(body.data());

        const vector<RequiredParameter> requiredParameters = {
                {"input_1", "package_name", {}, ""},
                {"input_2", "version", {}, ""},
                {"input_3", "package", {}, ""},
                {"input_4", "visibility", {"public", "private"}, "private"},
                {"input_5", "package_type", {}, "EMPTY"},
                {"input_6", "description", {}, "EMPTY"}
        };

        json upstreamRequestBody;
        for (const auto& req : requiredParameters)
        {
            if (!requestData.contains(req.name))
            {
                if(req.defaultVal.empty())
                {
                    session->close(400, "Please specify \""+req.name+"\"");
                    return;
                }
                if (req.defaultVal == "EMPTY")
                {
                    upstreamRequestBody[req.upstreamKey] = "";
                }
                else
                {
                    upstreamRequestBody[req.upstreamKey] = req.defaultVal;
                }
            }
            else
            {
                upstreamRequestBody[req.upstreamKey] = requestData[req.name];
            }
            fprintf(stdout, "%s (%s): %s\n", req.upstreamKey.c_str(), req.name.c_str(), upstreamRequestBody[req.upstreamKey].get<string>().c_str());
        }
        if (upstreamRequestBody["input_4"] == "public")
            upstreamRequestBody["input_4"] = "publish"; //what upstream expects.

//        string decodedPackage;
//        macaron::Base64::Decode(upstreamRequestBody["input_3"].get<string>(), decodedPackage);
        upstreamRequestBody["input_3"] = "";
        fprintf(stdout, "Request body:\n%s\n", upstreamRequestBody.dump().c_str());

        cpr::Response upstreamResponse = cpr::Post(
                cpr::Url("https://infrastructure.tech/wp-json/gf/v2/forms/1/submissions"),
                cpr::Body(upstreamRequestBody.dump()),
                cpr::Authentication{auth.username, auth.password},
                cpr::Header{{"Content-Type", "application/json"}});

//        multimap< string, string > replyHeaders;
//        for (const auto& head : upstreamResponse.header)
//        {
//            replyHeaders.insert(pair<string,string>(head.first, head.second));
//            fprintf(stdout, "Response header: {%s : %s}\n", head.first.c_str(), head.second.c_str());
//        }

        const multimap< string, string > replyHeaders
        {
            { "Content-Type", "application/json" }
        };

        fprintf(stdout, "Got %ld:\n%s\n", upstreamResponse.status_code, upstreamResponse.text.c_str());
        session->close(upstreamResponse.status_code, upstreamResponse.text, replyHeaders);
    });
}

void download_package(const shared_ptr< Session > session )
{
    Auth auth = GetAuth(session);
    auto request = session->get_request();
    if (!request->has_query_parameter("package_name"))
    {
        session->close(400, "You must specify the \"package_name\" that you would like to download." );
    }
    string packageName = request->get_query_parameter("package_name");
    string public_url = "https://infrastructure.tech/wp-json/wp/v2/package?slug=" + packageName;
    string private_url = public_url + "&status=private";
    cpr::Response upstreamResponse;
    if (request->has_header("Authorization"))
    {
        upstreamResponse = cpr::Get(cpr::Url{private_url}, cpr::Authentication{auth.username, auth.password});
        if (upstreamResponse.text == "[]")
        {
            upstreamResponse = cpr::Get(cpr::Url{public_url});
        }
    }
    else
    {
        upstreamResponse = cpr::Get(cpr::Url{public_url});
    }

    fprintf(stdout, "got %ld: %s\n", upstreamResponse.status_code, upstreamResponse.text.c_str());

    json responseData = json::parse(upstreamResponse.text);
    if (responseData.size() != 1)
    {
        session->close(404, "Package " + packageName + " could not be found.");
        return;
    }
    string fileUrl = responseData[0]["file"].get<string>();
    fprintf(stdout, "json: %s\n", responseData.dump().c_str());
    fprintf(stdout, "file: %s\n", fileUrl.c_str());
    cpr::Response fileData = cpr::Get(cpr::Url{fileUrl});

    const multimap< string, string > replyHeaders
    {
        { "Content-Disposition", "attachment; filename=\""+packageName+".zip\"" },
        { "Content-Type", "application/force-download" }
    };

    session->close(OK, fileData.text, replyHeaders);
}

int main(const int, const char** )
{
    auto publish = make_shared< Resource >();
    publish->set_path("v1/package/publish" );
    publish->set_method_handler("POST", publish_package );

    auto download = make_shared< Resource >();
    download->set_path("v1/package/download" );
    download->set_method_handler("GET", download_package );

    auto settings = make_shared< Settings >();
    settings->set_port(1984 );
    settings->set_default_header("Connection", "close" );

    Service service;
    service.publish(publish );
    service.publish(download );
    service.start(settings );

    return EXIT_SUCCESS;
}