// ===========================================================================
//	Vector4d.h
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