/*
 * Copyright (c) 2004, Ben Supnik and Chris Serio.
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

#ifndef PLANETYPE_H
#define PLANETYPE_H

typedef unsigned short PlaneTypeMask;

const PlaneTypeMask		Mask_ICAO = 	1 << 0;
const PlaneTypeMask		Mask_Airline =	1 << 1;
const PlaneTypeMask		Mask_Livery = 	1 << 2;
const PlaneTypeMask		Mask_All =		Mask_ICAO|Mask_Airline|Mask_Livery;

class PlaneType
{
public:
	std::string	mICAO;
	std::string	mAirline;
	std::string	mLivery;

	PlaneType(const std::string &icao="", const std::string &airline="", const std::string &livery="");
	PlaneType(const PlaneType &copySrc);
	PlaneType(PlaneType &&moveSrc);

	/* compare checks if the two types match
	 * @param other The other PlaneType to compare with
	 * @param mask The parts of the type to check.  defaults to all
	 */
	bool compare(const PlaneType &other, PlaneTypeMask mask=Mask_ICAO|Mask_Airline|Mask_Livery) const;
	bool operator==(const PlaneType &other) const;
	bool operator!=(const PlaneType &other) const;
	void operator=(const PlaneType &other);


	std::string toLongString() const;
	std::string toString() const;
};


#endif //XSQUAWKBOX_VATSIM_PLANETYPE_H
