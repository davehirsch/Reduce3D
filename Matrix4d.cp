// ===========================================================================
//	Matrix4d.cp
// ===========================================================================
//	
#import	"Matrix4d.h"

// ---------------------------------------------------------------------------
//		¥ Matrix4d
// ---------------------------------------------------------------------------
//	Default Constructor

Matrix4d::Matrix4d()
{
	// should be called after all 5 Vector4d's have been constructed (with zeros)
	// we'll now make it the identity matrix
	theArray[1][1] = 1;
	theArray[2][2] = 1;
	theArray[3][3] = 1;
	theArray[4][4] = 1;
}

// ---------------------------------------------------------------------------
//		¥ Matrix4d
// ---------------------------------------------------------------------------
//	Copy Constructor

Matrix4d::Matrix4d(const Matrix4d &inMatrix4d)
{
	theArray[1] = *(GetVector(1));
	theArray[2] = *(GetVector(2));
	theArray[3] = *(GetVector(3));
	theArray[4] = *(GetVector(4));
}

// ---------------------------------------------------------------------------
//		¥ Matrix4d
// ---------------------------------------------------------------------------
//	Default Destructor

Matrix4d::~Matrix4d()
{
}

// ---------------------------------------------------------------------------
//		¥ operator[]
// ---------------------------------------------------------------------------
Vector4d &
Matrix4d::operator[](short inSub)
{
	if (inSub < 1 || inSub > 4)
		throw OutOfBoundsErr();
	return theArray[inSub];
}

Vector4d *
Matrix4d::GetVector(short inSub)
{
	if (inSub < 1 || inSub > 4)
		throw OutOfBoundsErr();
	return &(theArray[inSub]);
}

// ---------------------------------------------------------------------------
//		¥ operator* (Point3DFloat)
// ---------------------------------------------------------------------------
Point3DFloat
Matrix4d::operator*(const Point3DFloat &inPt)
{
	static Point3DFloat outPt;
	outPt.x = inPt.x * theArray[1][1] + inPt.y * theArray[1][2] + inPt.z * theArray[1][3] + theArray[1][4];
	outPt.y = inPt.x * theArray[2][1] + inPt.y * theArray[2][2] + inPt.z * theArray[2][3] + theArray[2][4];
	outPt.z = inPt.x * theArray[3][1] + inPt.y * theArray[3][2] + inPt.z * theArray[3][3] + theArray[3][4];
	return outPt;
}

// ---------------------------------------------------------------------------
//		¥ operator* (NumberedPt)
// ---------------------------------------------------------------------------
NumberedPt
Matrix4d::operator*(const NumberedPt &inPt)
{
	static NumberedPt outPt;
	outPt.seq = inPt.seq;
	Point3DFloat tempPt(inPt);
	tempPt = ((*this) * tempPt);
//	outPt = ((*this) * (Point3DFloat) inPt);
	return outPt;
}

// ---------------------------------------------------------------------------
//		¥ operator* (Matrix4d)
// ---------------------------------------------------------------------------
Matrix4d
Matrix4d::operator*(const Matrix4d &inMat)
{
	static Matrix4d outMat;
	short i, j, k;

	for (i = 1; i <= 4; i++)
		for (j = 1; j <= 4; j++) {
			outMat[i][j] = 0.0;
			for (k = 1; k <= 4; k++) {
				outMat[i][j] = outMat[i][j] + theArray[k][j] * (const_cast<Matrix4d &>(inMat))[i][k];
			}
		}
	return outMat;
}

// ---------------------------------------------------------------------------
//		¥ BeIdentity
// ---------------------------------------------------------------------------
void
Matrix4d::BeIdentity()
{
	for (short i = 1; i <= 4; i++)
		for (short j = 1; j <= 4; j++)
			theArray[i][j] = 0.0;
	theArray[1][1] = 1;
	theArray[2][2] = 1;
	theArray[3][3] = 1;
	theArray[4][4] = 1;
}

// ---------------------------------------------------------------------------
//		¥ RotationMatrix
// ---------------------------------------------------------------------------
/* This routine creates the transformation matrix which will enable us to perform
 a simplified test for "wrapping" a bounding plane around an edge of a triangle
 which is known to be on the boundary.  The idea is to align the edge we're
 wrapping around so that it is on the y axis and the third point of the triangle
 is on the xy plane with negative x. This is done through an orthogonal matrix.
 Note: this is for left-handed coordinates.  If we switch to right-handed, we
 probably have to switch x and y, both here and in the routine "FindMinTheta" */
Matrix4d
Matrix4d::RotationMatrix (Point3DFloat &pt1, Point3DFloat &pt2, Point3DFloat &pt3)
{
	static Matrix4d outMat;
	Point3DFloat	lookat, toh, xP;
	Point3DFloat	v1, v2, v3;

	lookat = pt2 - pt1;
	v2 = lookat / -lookat.Magnitude();
	toh = pt3 - pt1;
	xP = lookat % toh;
	v3 = xP / xP.Magnitude();
	v1 = v3 % v2;

	outMat[1][1] = v1.x;
	outMat[1][2] = v1.y;
	outMat[1][3] = v1.z;
	outMat[2][1] = v2.x;
	outMat[2][2] = v2.y;
	outMat[2][3] = v2.z;
	outMat[3][1] = v3.x;
	outMat[3][2] = v3.y;
	outMat[3][3] = v3.z;
	return outMat;
}

// ---------------------------------------------------------------------------
//		¥ TransformationMatrix
// ---------------------------------------------------------------------------
Matrix4d
Matrix4d::TransformationMatrix (Point3DFloat &pt1, Point3DFloat &pt2, Point3DFloat &pt3)
{
	Matrix4d tempMat((TranslationMatrix(-pt1.x, -pt1.y, -pt1.z) * RotationMatrix(pt1, pt2, pt3)));
	(*this) = tempMat;
	return (*this);
}

// ---------------------------------------------------------------------------
//		¥ TranslationMatrix
// ---------------------------------------------------------------------------
/* This routine creates a transformation matrix which translates a coordinate
	system by dx, dy and dz */
Matrix4d
Matrix4d::TranslationMatrix (double dx, double dy, double dz)
{
	static Matrix4d outMat;
	outMat[1][4] = dx;
	outMat[2][4] = dy;
	outMat[3][4] = dz;
	return outMat;
}