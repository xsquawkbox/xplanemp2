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

#include <algorithm>
#include <cctype>
#include <cstring>
#include "XStringUtils.h"

using namespace std;

namespace xpmp {

	vector<string>
    tokenize(const string &str, const string &delim, int n)
	{
		string			dup = str;
		vector<string>	result;
		if (delim.empty() || n == 1)
		{
			result.emplace_back(std::move(dup));
			return std::move(result);
		}

		if (dup.empty()) return std::move(result);

		while (true)
		{
			auto 	position = dup.find_first_of(delim);
			string	token = dup.substr(0, position);

			if (!token.empty())
			{
				result.emplace_back(std::move(token));
			}

			// Nothing remaining
			if (position == string::npos) return std::move(result);

			dup = dup.substr(position + 1);
			if (n > 0 && result.size() >= (n-1)) {
			    result.emplace_back(std::move(dup));
			    return std::move(result);
			}
		}
	}

	// trim from start (in place)
	void
		ltrim(std::string &s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char c) { return !std::isspace(c); }));
	}

	// trim from end (in place)
	void
		rtrim(std::string &s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](char c) { return !std::isspace(c); }).base(), s.end());
	}

	// trim from both ends (in place)
	void
		trim(std::string &s)
	{
		ltrim(s);
		rtrim(s);
	}

	//FIXME: this whole function needs to be reconsidered - stringformat variants are far fewer these days.
	// This routine gets one line, but handles any platforms crlf setup.
	char *
		fgets_multiplatform(char *s, int n, FILE *file)
	{
		char *p = s;

		// Save one slot for the null.  If we do not have enough memory
		// to do this, bail.
		if (--n < 0) {
			return nullptr;
		}

		// Only bother to read if we have enough space in the char buf.
		if (n) {
			int c;
			do {
				c = getc(file);

				// EOF: this could mean I/O error or end of file.
				if (c == EOF) {
					if (feof(file) && p != s) {    // We read something and now the file's done, ok.
						break;
					}
					else {
						// Haven't read yet?  I/O error?  Return NULL!
						return nullptr;
					}
				}

				*p++ = c;
			}
			// Stop when we see either newline char or the buffer is full.
			// Note that the \r\n IS written to the line.
			while (c != '\n' && c != '\r' && --n);

			// Ben's special code: eat a \n if it follows a \r, etc.  Mac stdio
			// swizzles these guys a bit, so we will consolidate BOTH \r\n and \n\r into
			// just the first.
			if (c == '\r') {
				int c1 = getc(file);
				if (c1 != '\n') {
					ungetc(c1, file);
				}
			}
			if (c == '\n') {
				int c1 = getc(file);
				if (c1 != '\r') {
					ungetc(c1, file);
				}
			}
		}

		// Unless we're bailing with NULL, we MUST null terminate.
		*p = 0;

		return (s);
	}

	// This routine breaks a line into one or more tokens based on delimitors.
	void
		BreakStringPvt(
			const char *inString, std::vector<std::string> &outStrings, int maxBreak, const std::string &inSeparators)
	{
		outStrings.clear();

		const char *endPos = inString + strlen(inString);
		const char *iter = inString;
		while (iter < endPos) {
			while ((iter < endPos) && (inSeparators.find(*iter) != std::string::npos)) {
				++iter;
			}
			if (iter < endPos) {
				if (maxBreak && (maxBreak == static_cast<int>(outStrings.size() + 1))) {
					outStrings.emplace_back(iter, endPos);
					return;
				}
				const char *wordEnd = iter;
				while ((wordEnd < endPos) && (inSeparators.find(*wordEnd) == std::string::npos)) {
					++wordEnd;
				}

				outStrings.emplace_back(iter, wordEnd);

				iter = wordEnd;
			}
		}
	}

}
