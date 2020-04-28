#include "Framework.h"
#include "GUID_Generator.h"
#include <sstream>
#include <iomanip>
#include <guiddef.h>

const UINT GUID_Generator::Generate()
{
	std::hash<std::string> hasher;

	return static_cast<UINT>(hasher(GenerateToString()));
}

const Color GUID_Generator::GenerateColor()
{
	std::hash<std::string> hasher;

	UINT number = static_cast<UINT>(hasher(GenerateToString()));
	Color returnColor = Color(0, 0, 0, 0);
	returnColor.a = number % 255;
	number /= 255;
	returnColor.b = number % 255;
	number /= 255;
	returnColor.g = number % 255;
	number /= 255;
	returnColor.r = number % 255;
	return returnColor;
}

const string GUID_Generator::GenerateToString()
{
	GUID guid;
	auto hr = CoCreateGuid(&guid);

	if (SUCCEEDED(hr))
	{
		std::stringstream sstream;

		sstream
			<< std::hex << std::uppercase
			<< std::setw(8) << std::setfill('0') << guid.Data1 << "-"
			<< std::setw(4) << std::setfill('0') << guid.Data2 << "-"
			<< std::setw(4) << std::setfill('0') << guid.Data3 << "-"
			<< std::setw(2) << std::setfill('0') << static_cast<UINT>(guid.Data4[0])
			<< std::setw(2) << std::setfill('0') << static_cast<UINT>(guid.Data4[1])
			<< std::setw(2) << std::setfill('0') << static_cast<UINT>(guid.Data4[2])
			<< std::setw(2) << std::setfill('0') << static_cast<UINT>(guid.Data4[3])
			<< std::setw(2) << std::setfill('0') << static_cast<UINT>(guid.Data4[4])
			<< std::setw(2) << std::setfill('0') << static_cast<UINT>(guid.Data4[5])
			<< std::setw(2) << std::setfill('0') << static_cast<UINT>(guid.Data4[6])
			<< std::setw(2) << std::setfill('0') << static_cast<UINT>(guid.Data4[7]);

		return sstream.str();
	}

	return "N/A";
}