/*
 * Copyright (c) 2015, Roland Wilklmeier
 * Copyright (c) 2018, 2020, Chris Collins.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
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

