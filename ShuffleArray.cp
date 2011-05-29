// ===========================================================================//	ShuffleArray.cp// ===========================================================================//	#include	"ShuffleArray.h"#include 	"MathStuff.h"// ---------------------------------------------------------------------------//		� ShuffleArray// ---------------------------------------------------------------------------//	Constructor for an empty ShuffleArrayShuffleArray::ShuffleArray(short numElements)		: LArray(sizeof(short)){	short	*extraList = new short[numElements+1];	short	i, randNum;		for (i=1; i<=numElements; i++)		extraList[i] = i;	extraList[0] = numElements;		for (i=1; i<=numElements; i++) {		randNum = (short) RandomDbl(1, extraList[0]);		Push(extraList[randNum]);		for (short j=randNum+1; j<=extraList[0]; j++) {			extraList[j-1] = extraList[j];		}		(extraList[0])--;	}	delete[] extraList;}// ---------------------------------------------------------------------------//		� ~ShuffleArray// ---------------------------------------------------------------------------//	Destructor for ShuffleArrayShuffleArray::~ShuffleArray(){}// ---------------------------------------------------------------------------------//		� Pop// ---------------------------------------------------------------------------------shortShuffleArray::Pop(){	static Boolean NumFound;	static short outNum;		NumFound = FetchItemAt(GetCount(), &outNum);	if (NumFound)		RemoveItemsAt(1, GetCount());	else		throw ArrayIOErr();	return outNum;}// ---------------------------------------------------------------------------------//		� Push// ---------------------------------------------------------------------------------voidShuffleArray::Push(short &inNum){	InsertItemsAt(1, index_Last, &inNum);}// ---------------------------------------------------------------------------------//		� Clear// ---------------------------------------------------------------------------------voidShuffleArray::Clear(){	RemoveItemsAt(index_Last, 1);}// ---------------------------------------------------------------------------//		� operator[]// ---------------------------------------------------------------------------shortShuffleArray::operator[](short inSub){	short outNum;	FetchItemAt(inSub, &outNum);	return outNum;}// ---------------------------------------------------------------------------//		� Update// ---------------------------------------------------------------------------voidShuffleArray::Update(short index, short inNum){	AssignItemsAt(1, index, (void *) &inNum);}