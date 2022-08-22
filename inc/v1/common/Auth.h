#pragma once

#include "Includes.h"

struct Auth
{
	string type;
	string username;
	string password;
};

Auth GetAuth(const shared_ptr< Session > session)
{
	Auth ret;
	string authorization = session->get_request()->get_header("Authorization");
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
