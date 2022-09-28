#pragma once

#include "v1/common/Auth.h"
#include "v1/common/Environment.h"
#include "v1/common/Parameter.h"

/**
 * Download a package from the upstream api.
 * @return file data of zip file if successful.
 * @param session
 */
void DownloadPackage(const shared_ptr< Session > session)
{
	auto request = session->get_request();

	#if 1
	string parameters;
	for (const auto& param : request->get_query_parameters())
	{
		parameters += "{"+param.first+" : "+param.second+"} ";
	}
	fprintf(stdout, "Getting package with parameters: {\n    %s\n}\n", parameters.c_str());
	#endif

	Auth auth = GetAuth(session);
	if (!request->has_query_parameter("package_name"))
	{
		session->close(400, "You must specify the \"package_name\" that you would like to download.");
	}

	string packageName = request->get_query_parameter("package_name");
	if (packageName.empty())
	{
		session->close(204, "You must specify the \"package_name\" that you would like to download.");
	}

	string public_url = Environment::Instance().GetUpstreamURL() + "/wp-json/wp/v2/package?slug=" + packageName;
	string private_url = public_url + "&status=private";

	cpr::Response upstreamResponse;

	if (request->has_header("Authorization"))
	{
		upstreamResponse = cpr::Get(cpr::Url{private_url}, cpr::Authentication(auth.username, auth.password, cpr::AuthMode::BASIC));
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
	if (upstreamResponse.status_code == 401)
	{
		session->close(401, "Unauthorized. Wrong password?");
		session->erase();
		return;
	}

	json responseData = json::parse(upstreamResponse.text);
	if (responseData.size() != 1 || !responseData[0].contains("file"))
	{
		session->close(404, "Package " + packageName + " could not be found.");
		session->erase();
		return;
	}
	string fileUrl = responseData[0]["acf"]["file"].get<string>();
	//    fprintf(stdout, "json: %s\n", responseData.dump().c_str());
	//    fprintf(stdout, "file: %s\n", fileUrl.c_str());
	if (fileUrl.empty())
	{
		session->close(404, "Package " + packageName + " could not be found.");
		session->erase();
		return;
	}
	string mangledFileUrl = Environment::Instance().MangleURL(fileUrl);
	fprintf(stdout, "returning: %s\n", mangledFileUrl.c_str());
	cpr::Response fileData = cpr::Get(cpr::Url{mangledFileUrl});

	const multimap< string, string > replyHeaders
	{
		{ "Content-Disposition", "attachment; filename=\""+packageName+".zip\"" },
		{ "Content-Type", "application/force-download" },
		{ "Content-Length", to_string(fileData.text.size()) }
	};

	session->close(OK, fileData.text, replyHeaders);
	session->erase();
}
