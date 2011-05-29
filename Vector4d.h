// ===========================================================================
//	Vector4d.h
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================
#pragma once

// ---------------------------------------------------------------------------
//		¥ Vector4d
// ---------------------------------------------------------------------------

class	Vector4d {
	public:
		class OutOfBoundsErr{};	// exception class
						Vector4d();
						Vector4d(Vector4d &inVector4d);
		virtual			~Vector4d();
		double			&operator[](short inSub);
		Vector4d		&operator=(Vector4d &inVector4d);

	public:
		double			theVect[5];
};