//**************************************************************************************
// Filename:	EnvelopeData.cpp
// Copyright © 1997 <YourNameHere>. All rights reserved.
//
// Description:	
//
//**************************************************************************************
// Revision History:
// Wednesday, October 1, 1997 - Original
//**************************************************************************************

#import "EnvelopeData.h"
#import "MathStuff.h"
#import <float.h>
//**************************************************************************************
// Function:	Default Constructor
//
// Description: Builds the EnvelopeData class.
//
// Inputs:	none
//		
// Outputs:	none	
//
//**************************************************************************************
EnvelopeData::EnvelopeData(short inEnvelopes, short inHPoints, bool inDoSigmas)
{
	mRuns = NULL;
	
	mDoSigmas = inDoSigmas;
	mNumEnv = inEnvelopes;
	mNumH = inHPoints;
	
	mRuns = new OneSet[mNumEnv + 1];
	ThrowIfNil_(mRuns);
	
	for (short thisEnv = 1; thisEnv <= mNumEnv; thisEnv++) {
		mRuns[thisEnv].L = new double[mNumH + 1];
		ThrowIfNil_(mRuns[thisEnv].L);
		mRuns[thisEnv].Lgd = new double[mNumH + 1];
		ThrowIfNil_(mRuns[thisEnv].Lgd);
		mRuns[thisEnv].PCF = new double[mNumH + 1];
		ThrowIfNil_(mRuns[thisEnv].PCF);
		mRuns[thisEnv].PCFgd = new double[mNumH + 1];
		ThrowIfNil_(mRuns[thisEnv].PCFgd);
		mRuns[thisEnv].MCF = new double[mNumH + 1];
		ThrowIfNil_(mRuns[thisEnv].MCF);
		mRuns[thisEnv].MCF3 = new double[mNumH + 1];
		ThrowIfNil_(mRuns[thisEnv].MCF3);
		mRuns[thisEnv].MCFgd = new double[mNumH + 1];
		ThrowIfNil_(mRuns[thisEnv].MCFgd);
		mRuns[thisEnv].MCF3gd = new double[mNumH + 1];
		ThrowIfNil_(mRuns[thisEnv].MCF3gd);
		for (short thisH = 1; thisH <= mNumH; thisH++) {
			mRuns[thisEnv].L[thisH] = 0.0;
			mRuns[thisEnv].Lgd[thisH] = 0.0;
			mRuns[thisEnv].PCF[thisH] = 0.0;
			mRuns[thisEnv].PCFgd[thisH] = 0.0;
			mRuns[thisEnv].MCF[thisH] = 0.0;
			mRuns[thisEnv].MCF3[thisH] = 0.0;
			mRuns[thisEnv].MCFgd[thisH] = 0.0;
			mRuns[thisEnv].MCF3gd[thisH] = 0.0;
		}
		mRuns[thisEnv].orderingIndex = 0.0;
		mRuns[thisEnv].isolationIndex = 0.0;
		mRuns[thisEnv].quadratStatMean = 0.0;
		mRuns[thisEnv].RPStatMean = 0.0;
		mRuns[thisEnv].avramiRatio = 0.0;
		mRuns[thisEnv].minSep = FLT_MAX;
		mRuns[thisEnv].meanSep = 0.0;
		mRuns[thisEnv].meanNNRad = 0.0;
		mRuns[thisEnv].circQuadratStatMean = 0.0;
		mRuns[thisEnv].meanExtV = 0.0;
	}
}

//**************************************************************************************
// Function:	Destructor
//
// Description: Destroys the EnvelopeData class.
//
// Inputs:	none
//		
// Outputs:	none	
//
//**************************************************************************************
EnvelopeData::~EnvelopeData(void)
{
	for (short thisEnv = 1; thisEnv <= mNumEnv; thisEnv++) {
		delete[] mRuns[thisEnv].L;
		delete[] mRuns[thisEnv].Lgd;
		delete[] mRuns[thisEnv].PCF;
		delete[] mRuns[thisEnv].PCFgd;
		delete[] mRuns[thisEnv].MCF;
		delete[] mRuns[thisEnv].MCF3;
		delete[] mRuns[thisEnv].MCFgd;
		delete[] mRuns[thisEnv].MCF3gd;
	}
	delete[] mRuns;
}

// ---------------------------------------------------------------------------
//		¥ Clear
// ---------------------------------------------------------------------------
void
EnvelopeData::Clear()
{
	for (short thisEnv = 1; thisEnv <= mNumEnv; thisEnv++) {
		for (short thisH = 1; thisH <= mNumH; thisH++) {
			mRuns[thisEnv].L[thisH] = 0.0;
			mRuns[thisEnv].Lgd[thisH] = 0.0;
			mRuns[thisEnv].PCF[thisH] = 0.0;
			mRuns[thisEnv].PCFgd[thisH] = 0.0;
			mRuns[thisEnv].MCF[thisH] = 0.0;
			mRuns[thisEnv].MCF3[thisH] = 0.0;
			mRuns[thisEnv].MCFgd[thisH] = 0.0;
			mRuns[thisEnv].MCF3gd[thisH] = 0.0;
		}
		mRuns[thisEnv].orderingIndex = 0.0;
		mRuns[thisEnv].isolationIndex = 0.0;
		mRuns[thisEnv].quadratStatMean = 0.0;
		mRuns[thisEnv].RPStatMean = 0.0;
		mRuns[thisEnv].avramiRatio = 0.0;
		mRuns[thisEnv].minSep = FLT_MAX;
		mRuns[thisEnv].meanSep = 0.0;
		mRuns[thisEnv].meanNNRad = 0.0;
		mRuns[thisEnv].circQuadratStatMean = 0.0;
		mRuns[thisEnv].meanExtV = 0.0;
	}
}

// ---------------------------------------------------------------------------
//		¥ ClearFirstCFs
// ---------------------------------------------------------------------------
void
EnvelopeData::ClearFirstCFs()
{
	for (short thisH = 1; thisH <= mNumH; thisH++) {
			mRuns[1].L[thisH] = 0.0;
			mRuns[1].Lgd[thisH] = 0.0;
			mRuns[1].PCF[thisH] = 0.0;
			mRuns[1].PCFgd[thisH] = 0.0;
			mRuns[1].MCF[thisH] = 0.0;
			mRuns[1].MCF3[thisH] = 0.0;
			mRuns[1].MCFgd[thisH] = 0.0;
			mRuns[1].MCF3gd[thisH] = 0.0;
	}
}

// ---------------------------------------------------------------------------
//		¥ GetArray
// ---------------------------------------------------------------------------
double *
EnvelopeData::GetArray(short inEnvNum, short inKind)
{
	if (!mDoSigmas)
		inEnvNum = 1;

	switch (inKind) {
		case LKind:
			return mRuns[inEnvNum].L;
		break;
		case LgdKind:
			return mRuns[inEnvNum].Lgd;
		break;
		case PCFKind:
			return mRuns[inEnvNum].PCF;
		break;
		case PCFgdKind:
			return mRuns[inEnvNum].PCFgd;
		break;
		case MCFKind:
			return mRuns[inEnvNum].MCF;
		break;
		case MCF3Kind:
			return mRuns[inEnvNum].MCF3;
		break;
		case MCFgdKind:
			return mRuns[inEnvNum].MCFgd;
		break;
		case MCF3gdKind:
			return mRuns[inEnvNum].MCF3gd;
		break;
		default:
			throw("Fell through switch in EnvelopeData::GetArray");
		break;
	}
	return NULL;
}

// ---------------------------------------------------------------------------
//		¥ GetVal
// ---------------------------------------------------------------------------
double
EnvelopeData::GetVal(short EnvNum, short inKind, short inHNum)
{
	switch (inKind) {
		case LKind:
			return mRuns[EnvNum].L[inHNum];
		break;
		case LgdKind:
			return mRuns[EnvNum].Lgd[inHNum];
		break;
		case PCFKind:
			return mRuns[EnvNum].PCF[inHNum];
		break;
		case PCFgdKind:
			return mRuns[EnvNum].PCFgd[inHNum];
		break;
		case MCFKind:
			return mRuns[EnvNum].MCF[inHNum];
		break;
		case MCF3Kind:
			return mRuns[EnvNum].MCF3[inHNum];
		break;
		case MCFgdKind:
			return mRuns[EnvNum].MCFgd[inHNum];
		break;
		case MCF3gdKind:
			return mRuns[EnvNum].MCF3gd[inHNum];
		break;
		case OIKind:
			return mRuns[EnvNum].orderingIndex;
		break;
		case IIKind:
			return mRuns[EnvNum].isolationIndex;
		break;
		case QSMKind:
			return mRuns[EnvNum].quadratStatMean;
		break;
		case CQSMKind:
			return mRuns[EnvNum].circQuadratStatMean;
		break;
		case RPSMKind:
			return mRuns[EnvNum].RPStatMean;
		break;
		case ARKind:
			return mRuns[EnvNum].avramiRatio;
		break;
		case MinSepKind:
			return mRuns[EnvNum].minSep;
		break;
		case MeanSepKind:
			return mRuns[EnvNum].meanSep;
		break;
		case MNNRKind:
			return mRuns[EnvNum].meanNNRad;
		break;
		case MEVKind:
			return mRuns[EnvNum].meanExtV;
		break;
	}
	return DBL_MIN;
}


// ---------------------------------------------------------------------------
//		¥ GetMinAndMax
// ---------------------------------------------------------------------------
void
EnvelopeData::GetMinAndMax(short inHNum, short inKind, short inConfidence,
			double &outMin, double &outMax)
{
	float numToTossPart = mNumEnv * (1.0 - inConfidence / 100.0);
	short numToToss = numToTossPart / 2;
	double *minHolders;
	minHolders = new double[numToToss+1];
	double *maxHolders;
	maxHolders = new double[numToToss+1];
	short thisEnv, thisHolder;

	for (thisHolder=0; thisHolder <= dmh_max(0,numToToss-1); thisHolder++) {
		minHolders[thisHolder] = GetVal(thisHolder+1, inKind, inHNum);
		maxHolders[thisHolder] = GetVal(thisHolder+1, inKind, inHNum);
	}
	
	if (numToToss > 1) {
		// Sort Holders
		bool madeChange;
		do {
			madeChange = false;
			for (short item = 0; item <= numToToss-1; item++) {
				if (minHolders[item+1] < minHolders[item]) {
					double temp = minHolders[item];
					minHolders[item] = minHolders[item+1];
					minHolders[item+1] = temp;
					madeChange = true;
				}
				if (maxHolders[item+1] > maxHolders[item]) {
					double temp = maxHolders[item];
					maxHolders[item] = maxHolders[item+1];
					maxHolders[item+1] = temp;
					madeChange = true;
				}
			}
		} while (madeChange);
	}

	for (thisEnv = numToToss+1; thisEnv <= mNumEnv; thisEnv++) {
		double num = GetVal(thisEnv, inKind, inHNum);
		for (thisHolder = 0; thisHolder <= dmh_max(0,numToToss-1); thisHolder++) {
			if (num < minHolders[thisHolder]) {
				for (short other = numToToss-1; other >= thisHolder + 1; other--) {
					minHolders[other] = minHolders[other-1];
				}
				minHolders[thisHolder] = num;
				break;	// break out of for
			}
		}
		for (thisHolder = 0; thisHolder <= dmh_max(0,numToToss-1); thisHolder++) {
			if (num > maxHolders[thisHolder]) {
				for (short other = numToToss-1; other >= thisHolder + 1; other--) {
					maxHolders[other] = maxHolders[other-1];
				}
				maxHolders[thisHolder] = num;
				break;  // break out of for
			}
		}
	}
	outMin = minHolders[dmh_max(0,numToToss-1)];
	outMax = maxHolders[dmh_max(0,numToToss-1)];
	
	delete[] minHolders;
	delete[] maxHolders;
	minHolders = maxHolders = NULL;
}


// ---------------------------------------------------------------------------
//		¥ GetMeanAndSD
// ---------------------------------------------------------------------------
void
EnvelopeData::GetMeanAndSD(short inHNum, short inKind, double &outMean, double &outSD)
{
	double mean = 0.0, sd = 0.0;
	short thisEnv;
	
	for (thisEnv = 1; thisEnv <= mNumEnv; thisEnv++) {
		mean += GetVal(thisEnv, inKind, inHNum);
	}
	mean /= mNumEnv;
	for (thisEnv = 1; thisEnv <= mNumEnv; thisEnv++) {
		sd += sqr(GetVal(thisEnv, inKind, inHNum) - mean);
	}
	sd /= (mNumEnv - 1);
	sd = sqrt(sd);

	outMean = mean;
	outSD = sd;
}