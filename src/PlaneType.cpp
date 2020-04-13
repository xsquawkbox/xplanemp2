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

#include <string>
#include <utility>

#include "PlaneType.h"

using namespace std;

PlaneType::PlaneType(
	const std::string &icao,
	const std::string &airline,
	const std::string &livery) :
	mICAO(icao),
	mAirline(airline),
	mLivery(livery)
{

}

PlaneType::PlaneType(const PlaneType &copySrc) :
	mICAO(copySrc.mICAO),
	mAirline(copySrc.mAirline),
	mLivery(copySrc.mLivery)
{

}

PlaneType::PlaneType(PlaneType &&moveSrc) :
	mICAO(std::move(moveSrc.mICAO)),
	mAirline(std::move(moveSrc.mAirline)),
	mLivery(std::move(moveSrc.mLivery))
{

}

bool
PlaneType::compare(const PlaneType &other, PlaneTypeMask mask) const
{
	bool 	matches = true;

	if (mask & Mask_ICAO) {
		if (other.mICAO != mICAO) {
			return false;
		}
	}
	if (mask & Mask_Airline) {
		if (other.mAirline != mAirline) {
			return false;
		}
	}
	if (mask & Mask_Livery) {
		if (other.mLivery != mLivery) {
			return false;
		}
	}
	return true;
}

bool
PlaneType::operator==(const PlaneType &other) const
{
	return compare(other, Mask_All);
};

bool 
PlaneType::operator!=(const PlaneType &other) const
{
	return !compare(other, Mask_All);
}

void
PlaneType::operator=(const PlaneType &other)
{
	mICAO = other.mICAO;
	mAirline = other.mAirline;
	mLivery = other.mLivery;
}


string
PlaneType::toLongString() const
{
	string rv = "";
	if (!mICAO.empty()) {
		rv += "ICAO=" + mICAO;
	}
	if (!mAirline.empty()) {
		if (!rv.empty())
			rv += " ";
		rv += "AIRLINE=" + mAirline;
	}
	if (!mLivery.empty()) {
		if (!rv.empty())
			rv += " ";
		rv += "LIVERY=" + mLivery;
	}
	if (rv.empty()) {
		rv = "-NILTYPE-";
	}
	return rv;
}

string
PlaneType::toString() const
{
	return mICAO + "/" + mAirline + "/" + mLivery;
}

