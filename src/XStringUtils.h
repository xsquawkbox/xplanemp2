#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>

namespace xpmp {
	std::vector<std::string> tokenize(const std::string &str, const std::string &delim);
	void	ltrim(std::string &s);
	void	rtrim(std::string &s);
	void	trim(std::string &s);

	void	BreakStringPvt(
		const char *inString,
		std::vector<std::string> &outStrings,
		int maxBreak,
		const std::string &inSeparators);

	char *	fgets_multiplatform(char *s, int n, FILE *file);
}

#endif // STRING_UTILS_H

