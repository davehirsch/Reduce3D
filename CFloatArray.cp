// ===========================================================================
//	CFloatArray.cp
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
//
//		A 1-based array of floats using the C++ STL for storage.  Used for legacy
//		(CW PowerPlant) compatibility
// ===========================================================================
//	
#import	"CFloatArray.h"

// ---------------------------------------------------------------------------
//		� CFloatArray
// ---------------------------------------------------------------------------
//	Constructor for an empty CFloatArray
CFloatArray::CFloatArray()
{
	// reserve space for the array
	array.reserve(1000);
}

// ---------------------------------------------------------------------------
//		� ~CFloatArray
// ---------------------------------------------------------------------------
//	Destructor for CFloatArray
CFloatArray::~CFloatArray()
{
}

// ---------------------------------------------------------------------------------
//		� Pop
// ---------------------------------------------------------------------------------
double
CFloatArray::Pop()
{
	static double outNum;
	
	if (array.size() > 0) {
		outNum = array.back();
		array.pop_back();
	} else {		
		throw ArrayIOErr();
	}
	
	return outNum;
}

// ---------------------------------------------------------------------------------
//		� Push
// ---------------------------------------------------------------------------------
void
CFloatArray::Push(double *inNum)
{
	array.push_back(*inNum);
}

// ---------------------------------------------------------------------------------
//		� Push
// ---------------------------------------------------------------------------------
void
CFloatArray::Push(double inNum)
{
	array.push_back(inNum);
}

// ---------------------------------------------------------------------------------
//		� Clear
// ---------------------------------------------------------------------------------
void
CFloatArray::Clear()
{
	array.clear();
}

// ---------------------------------------------------------------------------
//		� operator[]
// ---------------------------------------------------------------------------
double
CFloatArray::operator[](int inSub)
{
	return array.at(inSub);
}

// ---------------------------------------------------------------------------
//		� Update
// ---------------------------------------------------------------------------
void
CFloatArray::Update(int index, double inNum)
{
	if (index > 0 && index < array.size()){
		array.at(index) = inNum;
	}
}

