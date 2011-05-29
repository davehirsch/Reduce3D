// ===========================================================================
//	Crystal.cp
// ===========================================================================
#import "Crystal.h"

// ---------------------------------------------------------------------------
//		¥ Crystal
// ---------------------------------------------------------------------------
Crystal::Crystal()
{
	r = actV = extV = ctcDist = polyV = diffV = 0.0;
	neighbor = ctrSlice = 0;
	ctrID = 0;
}

// ---------------------------------------------------------------------------
//		¥ Crystal
// ---------------------------------------------------------------------------
Crystal::Crystal(const Crystal &inXl)
{
	ctr = inXl.ctr;
	r = inXl.r;
	actV = inXl.actV;
	extV = inXl.extV;
	ctcDist = inXl.ctcDist;
	polyV = inXl.polyV;
	diffV = inXl.diffV;
	neighbor = inXl.neighbor;
	ctrSlice = inXl.ctrSlice;
	ctrID = inXl.ctrID;
}

// ---------------------------------------------------------------------------
//		¥ operator= (Crystal &)
// ---------------------------------------------------------------------------
Crystal
Crystal::operator= (const Crystal &inXl)
{
	static Crystal outXl;
	ctr = inXl.ctr;
	r = inXl.r;
	actV = inXl.actV;
	extV = inXl.extV;
	ctcDist = inXl.ctcDist;
	polyV = inXl.polyV;
	diffV = inXl.diffV;
	neighbor = inXl.neighbor;
	ctrSlice = inXl.ctrSlice;
	ctrID = inXl.ctrID;

	outXl.ctr = inXl.ctr;
	outXl.r = inXl.r;
	outXl.actV = inXl.actV;
	outXl.extV = inXl.extV;
	outXl.ctcDist = inXl.ctcDist;
	outXl.polyV = inXl.polyV;
	outXl.diffV = inXl.diffV;
	outXl.neighbor = inXl.neighbor;
	outXl.ctrSlice = inXl.ctrSlice;
	outXl.ctrID = inXl.ctrID;
	return outXl;
}

// ---------------------------------------------------------------------------
//		¥ operator== (Crystal &)
// ---------------------------------------------------------------------------
bool
Crystal::operator== (Crystal &inXl)
{
	return (	(ctr == inXl.ctr)								\
			&&	(r	== inXl.r)									\
			&&	(actV	== inXl.actV)							\
			&&	(extV	== inXl.extV)							\
			&&	(ctcDist	== inXl.ctcDist)					\
			&&	(polyV	== inXl.polyV)						\
			&&	(diffV	== inXl.diffV)						\
			&&	(neighbor	== inXl.neighbor)				\
			&&	(ctrSlice	== inXl.ctrSlice)				\
			&&	(ctrID	== inXl.ctrID));
}

/*
// ---------------------------------------------------------------------------
//		¥ IntersectionVolume (Crystal &)
// ---------------------------------------------------------------------------
// Formula for the intersection volume is the two spherical caps done separately.
//	Formula is given in CRC Math Tables & Formula, 30th Ed, pg. 314.
float
Crystal::IntersectionVolume(Crystal &inXl)
{
	double d = ctr.Distance(inXl.ctr);
	double h1 = r - r * ((r*r + d*d - inXl.r * inXl.r) / (2 * r * d));	// comes from law of cosines
	double h2 = r + inXl.r - h1 - d;
	double V1 = M_PI * h1 * h1 * (3 * r - h1) / 3;
	double V2 = M_PI * h2 * h2 * (3 * inXl.r - h2) / 3;
	double vol = V1 + V2;
	if (vol < 0)
		return 0;

	return vol;
}
*/
/*
// ===========================================================================
//	¥ RadXlComp
// ===========================================================================
//	Compares only radius values of Crystal structs - gives them back from largest
//	to smallest!  This is so thatwe can place the largest crystals first in
//	simulations.


RadXlComp*	RadXlComp::sRadXlComparator;

RadXlComp::RadXlComp()
{
}

RadXlComp::~RadXlComp()
{
}

SInt32
RadXlComp::Compare(
				   const void*		inItemOne,
				   const void*		inItemTwo,
				   UInt32			// inSizeOne ,
				   UInt32			// inSizeTwo ) const
{
	float numOne = ((Crystal*)inItemOne)->r;
	float numTwo = ((Crystal*)inItemTwo)->r;
	if (numOne > numTwo)
		return -1;
	else if (numTwo > numOne)
		return 1;
	else return 0;
}
*/
