#pragma once

#include "v1/common/Auth.h"
#include "v1/common/Environment.h"
#include "v1/common/Parameter.h"

void PublishPackage(const shared_ptr< Session > session)
{
	const auto request = session->get_request();
	int contentLength = request->get_header("Content-Length", 0);

	session->fetch(contentLength, [ request, contentLength ](const shared_ptr< Session > l_session, const Bytes & l_body)
	{
		#if 0
		string parameters;
        for (const auto& param : request->get_query_parameters())
        {
            parameters += "{"+param.first+" : "+param.second+"} ";
        }
        fprintf(stdout, "Publishing package with parameters: {\n    %s\n}\nand l_body (%d): {\n    %.*s\n}\n", parameters.c_str(), contentLength, (int) l_body.size(), l_body.data());
		#endif

		if (!request->has_header("Authorization"))
		{
			l_session->close(401, "Publishing requires http basic auth with username and password.");
			l_session->erase();
			return;
		}
		Auth auth = GetAuth(l_session);
		if (auth.username.empty() || auth.password.empty())
		{
			l_session->close(401, "Publishing requires http basic auth with username and password.");
			l_session->erase();
			return;
		}
		//TODO: why is this requestJson string nonsense required in order to trim invalid bytes from the end of the 2nd and onward calls of json::parse()???
		char* requestJson = new char[l_body.size()];
		sprintf(requestJson, "%.*s", (int) l_body.size(), l_body.data());
		string jStr(requestJson); //= R"({"package_name": "bin_cpp_api_test", "version": "v0.0.3", "visibility": "private", "package": ""})"; // <-  static string works 100%.
		//        json requestData = json::parse(l_body.data(), nullptr, false); //parsing bytes only works the 1st time; even though the prints are valid.
		json requestData = json::parse(jStr, nullptr, false);
		if (requestData.is_discarded())
		{
			fprintf(stdout, "Could not parse:\n%.*s\n", (int) l_body.size(), l_body.data());
			l_session->close(400, "Could not parse request. Please submit request as valid json.");
			l_session->erase();
			return;
		}
		delete[] requestJson;
		//{(ignore this bit), "parameter name", {possible choices}, "default value"}
		const vector<RequiredParameter> requiredParameters = {
			{"input_1", "package_name", {}, ""}, //the name of the package
			{"input_2", "version", {}, ""}, //the version of the package
			{"input_3", "package", {}, ""}, //the package data, base64 encoded.
			{"input_4", "visibility", {"public", "private"}, "private"}, //"private" if the package requires authentication to view; "public" if it does not.
			{"input_5", "package_type", {}, "EMPTY"}, //metadata. use as thou wilt.
			{"input_6", "description", {}, "EMPTY"} //docs maybe?
		};

		json upstreamRequestBody;
		for (const auto& req : requiredParameters)
		{
			if (!requestData.contains(req.name))
			{
				if(req.defaultVal.empty())
				{
					l_session->close(400, "Please specify \"" + req.name + "\"");
					l_session->erase();
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

		//        fprintf(stdout, "Request l_body:\n%s\n", upstreamRequestBody.dump().c_str());

		cpr::Response upstreamResponse = cpr::Post(
			cpr::Url(Environment::Instance().GetUpstreamURL() + "/wp-json/gf/v2/forms/1/submissions"),
			cpr::Body(upstreamRequestBody.dump()),
			cpr::Authentication{auth.username, auth.password},
			cpr::Header{{"Content-Type", "application/json"}});

		//        multimap< string, string > replyHeaders;
		//        for (const auto& head : upstreamResponse.header)
		//        {
		//            replyHeaders.insert(pair<string,string>(head.first, head.second));
		//            fprintf(stdout, "Response header: {%s : %s}\n", head.first.c_str(), head.second.c_str());
		//        }

		//        const multimap< string, string > replyHeaders
		//        {
		//            { "Content-Type", "application/json" }
		//        };

		fprintf(stdout, "Got %ld:\n%s\n", upstreamResponse.status_code, upstreamResponse.text.c_str());
		//        l_session->close(upstreamResponse.status_code, upstreamResponse.text, replyHeaders); //<- we don't want to be sending raw responses back to the requester.
		if (upstreamResponse.status_code == 400 && upstreamResponse.text.find("You do not have access to update this package.") != std::string::npos)
		{
			l_session->close(401, "Unauthorized.");
			l_session->erase();
			return;
		}
		l_session->close(upstreamResponse.status_code, "complete.");
		l_session->erase();
	});
}
