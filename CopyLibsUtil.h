#pragma once

struct MacroIdentifier
{
	const char* beginning;
	const char* ending;

	MacroIdentifier(const char* _beginning, const char* _ending) :
		beginning(_beginning), ending(_ending) {}
};

inline const MacroIdentifier MI_DirIdentifier ("?{",	"}");
inline const MacroIdentifier MI_ArgIdentifier ("?*",	"*");
