// ===========================================================================
//	Side.cp
// ===========================================================================
//	
#import "Side.h"
#import "ReduceConstants.h"
#import "MathStuff.h"

// ---------------------------------------------------------------------------
//		¥ Side
// ---------------------------------------------------------------------------
//	Default Constructor

Side::Side()
{
	edge1 = edge2 = edge3 = kUnfinished;
}

// ---------------------------------------------------------------------------
//		¥ Side
// ---------------------------------------------------------------------------
//	Copy Constructor

Side::Side(const Side &inSide)
{
	*this = inSide;
}

// ---------------------------------------------------------------------------
//		¥ Side
// ---------------------------------------------------------------------------
//	Default Destructor

Side::~Side()
{
}


// ---------------------------------------------------------------------------
//		¥ operator=
// ---------------------------------------------------------------------------
//	Copies a Side to this
Side&
Side::operator= (const Side &inSide)
{
	pt1 = (NumberedPt&)inSide.pt1;
	pt2 = (NumberedPt&)inSide.pt2;
	pt3 = (NumberedPt&)inSide.pt3;
	edge1 = inSide.edge1;
	edge2 = inSide.edge2;
	edge3 = inSide.edge3;
	inVect = inSide.inVect;
	static Side outSide;
	outSide.pt1 = pt1;
	outSide.pt2 = pt2;
	outSide.pt3 = pt3;
	return outSide;
}	

// ---------------------------------------------------------------------------
//		¥ operator==
// ---------------------------------------------------------------------------
//	Compares a Side to this
bool
Side::operator== (const Side &inSide)
{
	return ((pt1 == inSide.pt1 || pt1 == inSide.pt2 || pt1 == inSide.pt3) &&
			(pt2 == inSide.pt1 || pt2 == inSide.pt2 || pt2 == inSide.pt3) &&
			(pt3 == inSide.pt1 || pt3 == inSide.pt2 || pt3 == inSide.pt3));
}



// ---------------------------------------------------------------------------
//		¥ Area
// ---------------------------------------------------------------------------
/* Returns the area of the triangle formed by points pt1,pt2 and pt3.  Formula
	obtained from CRC math tables, 25th edition, p.294 */
float
Side::Area()
{
	float	det1, det2, det3;

	det1 = pt1.y * pt2.z + pt1.z * pt3.y + pt2.y * pt3.z - pt1.y * pt3.z - pt1.z * pt2.y - pt2.z * pt3.y;
	det2 = pt1.z * pt2.x + pt1.x * pt3.z + pt2.z * pt3.x - pt1.z * pt3.x - pt1.x * pt2.z - pt2.x * pt3.z;
	det3 = pt1.x * pt2.y + pt1.y * pt3.x + pt2.x * pt3.y - pt1.x * pt3.y - pt1.y * pt2.x - pt2.y * pt3.x;
	return  sqrt(det1 * det1 + det2 * det2 + det3 * det3) / 2;
}

// ---------------------------------------------------------------------------
//		¥ PointOnSide
// ---------------------------------------------------------------------------
bool
Side::PointOnSide(Point3DFloat &inPt)
{
	if (inPt == pt1 ||
		inPt == pt2 ||
		inPt == pt3)
		return true;
	return false;
}

// ---------------------------------------------------------------------------
//		¥ MakeInVect
// ---------------------------------------------------------------------------
/* Sets inVect to be a unit vector orthogonal to the side, pointing towards inCtr */
void
Side::MakeInVect(Point3DFloat &inCtr)
{
	inVect = AwayVect(inCtr);
	inVect = -inVect;
}

// ---------------------------------------------------------------------------
//		¥ AwayVect
// ---------------------------------------------------------------------------
/* Returns a unit vector orthogonal to the side, pointing in the direction away
	from inPt */
Point3DFloat &
Side::AwayVect(Point3DFloat &inPt)
{
	static Point3DFloat outPt;
	Point3DFloat v13, v12, v1inPt, vCross;
	
	v13 = pt3 - pt1;
	v12 = pt2 - pt1;
	vCross = v13 % v12;
	v1inPt = inPt - pt1;	// the vector from point1 to inPt
	if (v1inPt.Angle(vCross) < M_PI / 2.0)	// if angle between cross vector and vector from pt1 to inPt is < 90¡
		outPt = -vCross;
	else
		outPt = vCross;
	return outPt.Unit();
}

// ---------------------------------------------------------------------------
//		¥ Vect12to3
// ---------------------------------------------------------------------------
/* Returns a unit vector in the plane of the side, orthogonal to the 1-2 join, 
	pointing on the side of pt3 */
Point3DFloat &
Side::Vect12to3()
{
	static Point3DFloat outPt;
	Point3DFloat fakeCtr, orth, v12, v13, vCross;
	
	orth = AwayVect(fakeCtr);
	v12 = pt2 - pt1;
	v13 = pt3 - pt1;
	vCross = orth % v12;
	if (::fabs(v13.Angle(vCross)) > M_PI / 2.0)	// if angle between cross vector and vector from pt1 to inPt is > 90¡
		outPt = -vCross;
	else
		outPt = vCross;
	return outPt.Unit();
}


