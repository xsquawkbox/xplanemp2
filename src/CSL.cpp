//
// Created by kuroneko on 2/03/2018.
//

#include <string>

#include "CSL.h"

using namespace std;

CSL::CSL()
{
	mOffsetSource = VerticalOffsetSource::None;
	mMovingGear = true;
}

CSL::CSL(std::vector<std::string> dirNames) :
	mDirNames(dirNames)
{
	mMovingGear = true;
	mOffsetSource = VerticalOffsetSource::None;
}

double
CSL::getVertOffset() const
{
	switch (mOffsetSource) {
	case VerticalOffsetSource::None:
		return 0.0;
	case VerticalOffsetSource::Model:
		return mModelVertOffset;
	case VerticalOffsetSource::Mtl:
		return mMtlVertOffset;
	case VerticalOffsetSource::Preference:
		return mPreferencesVertOffset;
	}
}

VerticalOffsetSource
CSL::getVertOffsetSource() const
{
	return mOffsetSource;
}

void
CSL::setVertOffsetSource(VerticalOffsetSource offsetSource)
{
	mOffsetSource = offsetSource;
}

void
CSL::setVerticalOffset(VerticalOffsetSource src, double offset)
{
	switch(src) {
	case VerticalOffsetSource::None:
		break;
	case VerticalOffsetSource::Model:
		mModelVertOffset = offset;
		break;
	case VerticalOffsetSource::Mtl:
		mMtlVertOffset = offset;
		break;
	case VerticalOffsetSource::Preference:
		mPreferencesVertOffset = offset;
		break;
	}
	if (src > mOffsetSource) {
		mOffsetSource = src;
	}
}

void
CSL::setMovingGear(bool movingGear)
{
	mMovingGear = movingGear;
}

bool
CSL::getMovingGear() const
{
	return mMovingGear;
}

void
CSL::setICAO(std::string icaoCode)
{
	mICAO = icaoCode;
}

void
CSL::setAirline(std::string icaoCode, std::string airline)
{
	setICAO(icaoCode);
	mAirline = airline;
}

void
CSL::setLivery(std::string icaoCode, std::string airline, std::string livery)
{
	setAirline(icaoCode, airline);
	mLivery = livery;
}

std::string
CSL::getICAO() const {
	return mICAO;
}

std::string
CSL::getAirline() const {
	return mAirline;
}

std::string
CSL::getLivery() const {
	return mLivery;
}

bool
CSL::isUsable() const {
	return true;
}

void *
CSL::newInstanceData()
{
	return nullptr;
}

void
CSL::deleteInstanceData(void *instanceData)
{
};
