//
// Created by kuroneko on 2/03/2018.
//

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

