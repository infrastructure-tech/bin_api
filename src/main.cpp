
#include "v1/package/DownloadPackage.h"
#include "v1/package/PublishPackage.h"

int main(const int, const char**)
{
    auto publish = make_shared< Resource >();
    publish->set_path("v1/package/publish");
    publish->set_method_handler("POST", PublishPackage);

    auto download = make_shared< Resource >();
    download->set_path("v1/package/download");
    download->set_method_handler("GET", DownloadPackage);

    auto settings = make_shared< Settings >();
    settings->set_port(80);
    settings->set_default_header("Connection", "close");

    Service service;
    service.publish(publish);
    service.publish(download);
    service.start(settings);

    return EXIT_SUCCESS;
}
