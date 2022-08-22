#pragma once

#include "Includes.h"

class Environment
{
public:
	static Environment& Instance()
	{
		static Environment environment;
		return environment;
	}
	string GetUpstreamURL() const {
		return m_upstreamReplacement;
	}
	string MangleURL(string url)
	{
		//assume input is good.
		return GetUpstreamURL() + url.substr(m_upstreamUrl.size());
	}
private:
	Environment() :
		m_upstreamUrl("https://infrastructure.tech"),
		m_upstreamReplacement(getenv("INFRASTRUCTURE_URL"))
	{
	}
	const string m_upstreamUrl;
	const string m_upstreamReplacement;
};
