//
// Created by kuroneko on 2/03/2018.
//

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
