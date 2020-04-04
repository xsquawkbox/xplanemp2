#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>

namespace xpmp {
	/** tokenize takes the string and splits it into no more than n tokens.
     *
     * @param str the string to split
     * @param delim a string containing the delimiting characters.
     * @param n the maximum number of tokens to produce.  If 0, then there's no limit.
     *
     * @return a vector of the tokens after splitting.
    */
    std::vector<std::string>
    tokenize(const std::string &str, const std::string &delim, int n = 0);

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

