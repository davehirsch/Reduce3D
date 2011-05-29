// ===========================================================================
//	NumberedPt.cp
// ===========================================================================
#import "NumberedPt.h"

// ---------------------------------------------------------------------------
//		¥ NumberedPt
// ---------------------------------------------------------------------------
NumberedPt::NumberedPt()
{
	x = y = z = 0.0;
	seq = 0;
	flag = false;
}

// ---------------------------------------------------------------------------
//		¥ NumberedPt
// ---------------------------------------------------------------------------
NumberedPt::NumberedPt(short inSeq, float inX, float inY, float inZ, bool inFlag)
{
	seq = inSeq;
	x = inX;
	y = inY;
	z = inZ;
	flag = false;
}

// ---------------------------------------------------------------------------
//		¥ NumberedPt
// ---------------------------------------------------------------------------
NumberedPt::NumberedPt(const NumberedPt &inPt)
{
	seq = inPt.seq;
	x = inPt.x;
	y = inPt.y;
	z = inPt.z;
	flag = inPt.flag;
}

// ---------------------------------------------------------------------------
//		¥ NumberedPt
// ---------------------------------------------------------------------------
NumberedPt::NumberedPt(const Point3DFloat &inPt)
{
	seq = 0;
	x = inPt.x;
	y = inPt.y;
	z = inPt.z;
	flag = false;
}

// ---------------------------------------------------------------------------
//		¥ operator= (NumberedPt)
// ---------------------------------------------------------------------------
NumberedPt &
NumberedPt::operator= (NumberedPt &inPt)
{
	static NumberedPt outPt;
	x = inPt.x;
	y = inPt.y;
	z = inPt.z;
	seq = inPt.seq;
	flag = inPt.flag;
	outPt.x = inPt.x;
	outPt.y = inPt.y;
	outPt.z = inPt.z;
	outPt.seq = inPt.seq;
	outPt.flag = inPt.flag;
	return outPt;
}


// ---------------------------------------------------------------------------
//		¥ operator= (Point3DFloat)
// ---------------------------------------------------------------------------
NumberedPt &
NumberedPt::operator= (Point3DFloat &inPt)
{
	static NumberedPt outPt;
	x = inPt.x;
	y = inPt.y;
	z = inPt.z;
	seq = -1;
	outPt.x = inPt.x;
	outPt.y = inPt.y;
	outPt.z = inPt.z;
	outPt.seq = -1;
	return outPt;
}

// ---------------------------------------------------------------------------
//		¥ Copy (NumberedPt)
// ---------------------------------------------------------------------------
void
NumberedPt::Copy (NumberedPt &inPt)
{
	*this = inPt;
}

// ---------------------------------------------------------------------------
//		¥ operator==
// ---------------------------------------------------------------------------
bool
NumberedPt::operator== (const NumberedPt &inPt)
{
	return seq == inPt.seq;
}

// ---------------------------------------------------------------------------
//		¥ operator!=
// ---------------------------------------------------------------------------
bool
NumberedPt::operator!= (NumberedPt &inPt)
{
	return seq != inPt.seq;
}