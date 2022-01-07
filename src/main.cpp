#include <memory>
#include <cstdlib>
#include <restbed>
#include <cpr/cpr.h>
#include "base64.h"

using namespace std;
using namespace restbed;

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
    size_t breakPos = authorization.find(" ");
    ret.type = authorization.substr(0, breakPos);
    if (ret.type != "Basic")
        return ret;
    string b64 = authorization.substr(++breakPos);
    string b64Decoded;
    macaron::Base64::Decode(b64, b64Decoded);
    breakPos = b64Decoded.find(":");
    ret.username = b64Decoded.substr(0,breakPos);
    ret.password = b64Decoded.substr(++breakPos);
    fprintf( stdout, "%s (%s:%s => %s)\n", authorization.c_str(), ret.type.c_str(), b64.c_str(), b64Decoded.c_str());
    fprintf( stdout, "Username: \"%s\"\nPassword: \"%s\"\n", ret.username.c_str(), ret.password.c_str());
    return ret;
}

void publish_package( const shared_ptr< Session > session )
{
    const auto request = session->get_request( );

    int content_length = request->get_header( "Content-Length", 0 );

    session->fetch( content_length, [ ]( const shared_ptr< Session > session, const Bytes & body )
    {
        fprintf( stdout, "%.*s\n", ( int ) body.size( ), body.data( ) );
        session->close( OK, "Hello, World!", { { "Content-Length", "13" } } );
    } );
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
    cpr::Response serverResponse;
    if (request->has_header("Authorization"))
    {
        serverResponse = cpr::Get(cpr::Url{private_url}, cpr::Authentication{auth.username, auth.password});
        if (serverResponse.text == "[]")
        {
            serverResponse = cpr::Get(cpr::Url{public_url});
        }
    }
    else
    {
        serverResponse = cpr::Get(cpr::Url{public_url});
    }

    fprintf( stdout, "got %ld: %s\n", serverResponse.status_code, serverResponse.text.c_str());

    session->close( OK, "download package...");
}

int main( const int, const char** )
{
    auto resource = make_shared< Resource >( );

    resource->set_path( "/v1/package/publish" );
    resource->set_method_handler( "POST", publish_package );

    resource->set_path( "v1/package/download" );
    resource->set_method_handler( "GET", download_package );

    auto settings = make_shared< Settings >( );
    settings->set_port( 1984 );
    settings->set_default_header( "Connection", "close" );

    Service service;
    service.publish( resource );
    service.start( settings );

    return EXIT_SUCCESS;
}