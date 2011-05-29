// ===========================================================================
//	Vector4d.cp
// ===========================================================================
//	
#import	"Vector4d.h"

// ---------------------------------------------------------------------------
//		¥ Vector4d
// ---------------------------------------------------------------------------
//	Default Constructor

Vector4d::Vector4d()
{
	theVect[1] = theVect[2] = theVect[3] = theVect[4] = 0.0;
}

// ---------------------------------------------------------------------------
//		¥ Vector4d
// ---------------------------------------------------------------------------
//	Default Constructor

Vector4d::Vector4d(Vector4d &inVector4d)
{
	theVect[1] = inVector4d[1];
	theVect[2] = inVector4d[2];
	theVect[3] = inVector4d[3];
	theVect[4] = inVector4d[4];
}

// ---------------------------------------------------------------------------
//		¥ Vector4d
// ---------------------------------------------------------------------------
//	Default Destructor

Vector4d::~Vector4d()
{
}

Vector4d &
Vector4d::operator=(Vector4d &inVector4d)
{
	theVect[1] = inVector4d[1];
	theVect[2] = inVector4d[2];
	theVect[3] = inVector4d[3];
	theVect[4] = inVector4d[4];
	return *this;
}

// ---------------------------------------------------------------------------
//		¥ operator[]
// ---------------------------------------------------------------------------
double &
Vector4d::operator[](short inSub)
{
	if (inSub < 1 || inSub > 4)
		throw OutOfBoundsErr();
	return theVect[inSub];
}



