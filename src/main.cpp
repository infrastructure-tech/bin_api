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

Auth GetAuth(const shared_ptr< Session > session)
{
    Auth ret;
    string authorization = session->get_request( )->get_header( "Authorization" );
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
    fprintf( stdout, "%s (%s:%s => %s)\n", authorization.c_str(), ret.type.c_str(), b64.c_str(), b64Decoded.c_str());
    fprintf( stdout, "Username: \"%s\"\nPassword: \"%s\"\n", ret.username.c_str(), ret.password.c_str());
    return ret;
}

void publish_package( const shared_ptr< Session > session )
{
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);
    string parameters;
    for (const auto& param : request->get_query_parameters())
    {
        parameters += "{"+param.first+" : "+param.second+"} ";
    }
    fprintf( stdout, "Publishing package with parameters: {\n    %s\n}\n and body (%d): {\n    %s\n}\n", parameters.c_str(), contentLength, request->get_body().data());
    session->fetch( contentLength, [ request ]( const shared_ptr< Session > session, const Bytes & body )
    {
        fprintf( stdout, "%.*s\n", ( int ) body.size( ), body.data( ) );
    });

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

    if (!request->has_query_parameter("package"))
    {
        session->close(400, "You must upload a file in the \"package\" parameter");
        return;
    }
    string package = request->get_query_parameter("package");

    if (!request->has_query_parameter("package_name"))
    {
        session->close(400, "You must specify the \"package_name\" parameter");
        return;
    }
    string packageName = request->get_query_parameter("package_name");

    if (!request->has_query_parameter("version"))
    {
        session->close(400, "You must specify the \"version\" parameter");
        return;
    }
    string version = request->get_query_parameter("version");

    string visibility = "private";
    string validVisibilityValues[] = {"private", "publish"};
    if (request->has_query_parameter("visibility"))
    {
        visibility = request->get_query_parameter("visibility");
        if (visibility == "public")
            visibility = "publish";
        if (find(begin(validVisibilityValues), end(validVisibilityValues), visibility) == end(validVisibilityValues))
        {
            session->close(400, "\"visibility\" must be \"public\" or \"private\" (default)");
            return;
        }
    }

    string packageType;
    if (request->has_query_parameter("package_type"))
    {
        packageType = request->get_query_parameter("package_type");
    }
    else
    {
        packageType = packageName.substr(0, packageName.find('_'));
        if (packageType.size() == packageName.size())
            packageType = ""; //we tried.
    }

    string description;
    if (request->has_query_parameter("description"))
    {
        description = request->get_query_parameter("description");
    }

    json upstreamRequestBody;
    upstreamRequestBody["input_1"] = packageName;
    upstreamRequestBody["input_2"] = version;
    upstreamRequestBody["input_3"] = package;
    upstreamRequestBody["input_4"] = visibility;
    upstreamRequestBody["input_5"] = packageType;
    upstreamRequestBody["input_6"] = description;

    cpr::Response upstreamResponse = cpr::Post(
            cpr::Url("https://infrastructure.tech/wp-json/gf/v2/forms/1/submissions"),
            cpr::Body(upstreamRequestBody.dump()),
            cpr::Authentication{auth.username, auth.password});
    multimap< string, string > replyHeaders;
    for (const auto& head : upstreamResponse.header)
    {
        replyHeaders.insert(pair<string,string>(head.first, head.second));
    }
    session->close(upstreamResponse.status_code, upstreamResponse.text, replyHeaders);
}

void download_package( const shared_ptr< Session > session )
{
    Auth auth = GetAuth(session);
    auto request = session->get_request( );
    if (!request->has_query_parameter("package_name"))
    {
        session->close( 400, "You must specify the \"package_name\" that you would like to download." );
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
    fprintf( stdout, "json: %s\n", responseData.dump().c_str());
    fprintf( stdout, "file: %s\n", fileUrl.c_str());
    cpr::Response fileData = cpr::Get(cpr::Url{fileUrl});

    const multimap< string, string > replyHeaders
    {
        { "Content-Disposition", "attachment; filename=\""+packageName+".zip\"" },
        { "Content-Type", "application/force-download" }
    };

    session->close( OK, fileData.text, replyHeaders);
}

int main( const int, const char** )
{
    auto publish = make_shared< Resource >( );
    publish->set_path( "v1/package/publish" );
    publish->set_method_handler( "POST", publish_package );

    auto download = make_shared< Resource >( );
    download->set_path( "v1/package/download" );
    download->set_method_handler( "GET", download_package );

    auto settings = make_shared< Settings >( );
    settings->set_port( 1984 );
    settings->set_default_header( "Connection", "close" );

    Service service;
    service.publish( publish );
    service.publish( download );
    service.start( settings );

    return EXIT_SUCCESS;
}