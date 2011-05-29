// ===========================================================================
//	Stats.cp
// ===========================================================================
//	
#import "Stats.h"
#import "MathStuff.h"
#import <float.h>


// ---------------------------------------------------------------------------
//		¥ Stats
// ---------------------------------------------------------------------------
//	Default Constructor

Stats::Stats(bool inRealDataset)
{
	forRealDataset = inRealDataset;
	hDistances = nil;
	LValues = nil;
	LgdValues = nil;
	PCF = nil;
	PCFgd = nil;
	MCF = nil;
	MCF3 = nil;
	MCFgd = nil;
	MCF3gd = nil;
	LValuesEnvMean = nil;
	LValuesEnvStdDev = nil;
	LgdValuesEnvMean = nil;
	LgdValuesEnvStdDev = nil;
	PCFValuesEnvMean = nil;
	PCFValuesEnvStdDev = nil;
	PCFgdValuesEnvMean = nil;
	PCFgdValuesEnvStdDev = nil;
	MCFValuesEnvMean = nil;
	MCFValuesEnvStdDev = nil;
	MCF3ValuesEnvMean = nil;
	MCF3ValuesEnvStdDev = nil;
	MCFgdValuesEnvMean = nil;
	MCFgdValuesEnvStdDev = nil;
	MCF3gdValuesEnvMean = nil;
	MCF3gdValuesEnvStdDev = nil;
	LValuesEnvMin = nil;
	LValuesEnvMax = nil;
	LgdValuesEnvMin = nil;
	LgdValuesEnvMax = nil;
	PCFValuesEnvMin = nil;
	PCFValuesEnvMax = nil;
	PCFgdValuesEnvMin = nil;
	PCFgdValuesEnvMax = nil;
	MCFValuesEnvMin = nil;
	MCFValuesEnvMax = nil;
	MCF3ValuesEnvMin = nil;
	MCF3ValuesEnvMax = nil;
	MCFgdValuesEnvMin = nil;
	MCFgdValuesEnvMax = nil;
	MCF3gdValuesEnvMin = nil;
	MCF3gdValuesEnvMax = nil;

	mSigmas = -1;
	numLPoints = -1;
	Clear();
}

// ---------------------------------------------------------------------------
//		¥ SetCFParams
// ---------------------------------------------------------------------------
void
Stats::SetCFParams(short inNumKPoints, bool inSigma)
{
	mSigmas = inSigma;
	numLPoints = inNumKPoints;
	
	hDistances = new float[numLPoints + 1];
	LValues = new double[numLPoints + 1];
	LgdValues = new double[numLPoints + 1];
	PCF = new double[numLPoints + 1];
	PCFgd = new double[numLPoints + 1];
	MCF = new double[numLPoints + 1];
	MCF3 = new double[numLPoints + 1];
	MCFgd = new double[numLPoints + 1];
	MCF3gd = new double[numLPoints + 1];
	if (inSigma) {
		LValuesEnvMean = new double[numLPoints + 1];
		LValuesEnvStdDev = new double[numLPoints + 1];
		LgdValuesEnvMean = new double[numLPoints + 1];
		LgdValuesEnvStdDev = new double[numLPoints + 1];
		PCFValuesEnvMean = new double[numLPoints + 1];
		PCFValuesEnvStdDev = new double[numLPoints + 1];
		PCFgdValuesEnvMean = new double[numLPoints + 1];
		PCFgdValuesEnvStdDev = new double[numLPoints + 1];
		MCFValuesEnvMean = new double[numLPoints + 1];
		MCFValuesEnvStdDev = new double[numLPoints + 1];
		MCF3ValuesEnvMean = new double[numLPoints + 1];
		MCF3ValuesEnvStdDev = new double[numLPoints + 1];
		MCFgdValuesEnvMean = new double[numLPoints + 1];
		MCFgdValuesEnvStdDev = new double[numLPoints + 1];
		MCF3gdValuesEnvMean = new double[numLPoints + 1];
		MCF3gdValuesEnvStdDev = new double[numLPoints + 1];
	} else {
		LValuesEnvMin = new double[numLPoints + 1];
		LValuesEnvMax = new double[numLPoints + 1];
		LgdValuesEnvMin = new double[numLPoints + 1];
		LgdValuesEnvMax = new double[numLPoints + 1];
		PCFValuesEnvMin = new double[numLPoints + 1];
		PCFValuesEnvMax = new double[numLPoints + 1];
		PCFgdValuesEnvMin = new double[numLPoints + 1];
		PCFgdValuesEnvMax = new double[numLPoints + 1];
		MCFValuesEnvMin = new double[numLPoints + 1];
		MCFValuesEnvMax = new double[numLPoints + 1];
		MCF3ValuesEnvMin = new double[numLPoints + 1];
		MCF3ValuesEnvMax = new double[numLPoints + 1];
		MCFgdValuesEnvMin = new double[numLPoints + 1];
		MCFgdValuesEnvMax = new double[numLPoints + 1];
		MCF3gdValuesEnvMin = new double[numLPoints + 1];
		MCF3gdValuesEnvMax = new double[numLPoints + 1];
	}
	ClearArrays();
}

// ---------------------------------------------------------------------------
//		¥ Stats
// ---------------------------------------------------------------------------
//	Default Destructor

Stats::~Stats()
{
	if (hDistances) {
		delete[] hDistances;
		hDistances = nil;
	}
	if (LValues) {
		delete[] LValues;
		LValues = nil;
	}
	if (LgdValues) {
		delete[] LgdValues;
		LgdValues = nil;
	}
	if (PCF) {
		delete[] PCF;
		PCF = nil;
	}
	if (PCFgd) {
		delete[] PCFgd;
		PCFgd = nil;
	}
	if (MCF) {
		delete[] MCF;
		MCF = nil;
	}
	if (MCF3) {
		delete[] MCF3;
		MCF3 = nil;
	}
	if (MCFgd) {
		delete[] MCFgd;
		MCFgd = nil;
	}
	if (MCF3gd) {
		delete[] MCF3gd;
		MCF3gd = nil;
	}
	if (mSigmas) {
		if (LValuesEnvMean) {
			delete[] LValuesEnvMean;
			LValuesEnvMean = nil;
		}
		if (LValuesEnvStdDev) {
			delete[] LValuesEnvStdDev;
			LValuesEnvStdDev = nil;
		}
		if (LgdValuesEnvMean) {
			delete[] LgdValuesEnvMean;
			LgdValuesEnvMean = nil;
		}
		if (LgdValuesEnvStdDev) {
			delete[] LgdValuesEnvStdDev;
			LgdValuesEnvStdDev = nil;
		}
		if (PCFValuesEnvMean) {
			delete[] PCFValuesEnvMean;
			PCFValuesEnvMean = nil;
		}
		if (PCFValuesEnvStdDev) {
			delete[] PCFValuesEnvStdDev;
			PCFValuesEnvStdDev = nil;
		}
		if (PCFgdValuesEnvMean) {
			delete[] PCFgdValuesEnvMean;
			PCFgdValuesEnvMean = nil;
		}
		if (PCFgdValuesEnvStdDev) {
			delete[] PCFgdValuesEnvStdDev;
			PCFgdValuesEnvStdDev = nil;
		}
		if (MCFValuesEnvMean) {
			delete[] MCFValuesEnvMean;
			MCFValuesEnvMean = nil;
		}
		if (MCFValuesEnvStdDev) {
			delete[] MCFValuesEnvStdDev;
			MCFValuesEnvStdDev = nil;
		}
		if (MCF3ValuesEnvMean) {
			delete[] MCF3ValuesEnvMean;
			MCF3ValuesEnvMean = nil;
		}
		if (MCF3ValuesEnvStdDev) {
			delete[] MCF3ValuesEnvStdDev;
			MCF3ValuesEnvStdDev = nil;
		}
		if (MCFgdValuesEnvMean) {
			delete[] MCFgdValuesEnvMean;
			MCFgdValuesEnvMean = nil;
		}
		if (MCFgdValuesEnvStdDev) {
			delete[] MCFgdValuesEnvStdDev;
			MCFgdValuesEnvStdDev = nil;
		}
		if (MCF3gdValuesEnvMean) {
			delete[] MCF3gdValuesEnvMean;
			MCF3gdValuesEnvMean = nil;
		}
		if (MCF3gdValuesEnvStdDev) {
			delete[] MCF3gdValuesEnvStdDev;
			MCF3gdValuesEnvStdDev = nil;
		}
	} else {
		if (LValuesEnvMin) {
			delete[] LValuesEnvMin;
			LValuesEnvMin = nil;
		}
		if (LValuesEnvMax) {
			delete[] LValuesEnvMax;
			LValuesEnvMax = nil;
		}
		if (LgdValuesEnvMin) {
			delete[] LgdValuesEnvMin;
			LgdValuesEnvMin = nil;
		}
		if (LgdValuesEnvMax) {
			delete[] LgdValuesEnvMax;
			LgdValuesEnvMax = nil;
		}
		if (PCFValuesEnvMin) {
			delete[] PCFValuesEnvMin;
			PCFValuesEnvMin = nil;
		}
		if (PCFValuesEnvMax) {
			delete[] PCFValuesEnvMax;
			PCFValuesEnvMax = nil;
		}
		if (PCFgdValuesEnvMin) {
			delete[] PCFgdValuesEnvMin;
			PCFgdValuesEnvMin = nil;
		}
		if (PCFgdValuesEnvMax) {
			delete[] PCFgdValuesEnvMax;
			PCFgdValuesEnvMax = nil;
		}
		if (MCFValuesEnvMin) {
			delete[] MCFValuesEnvMin;
			MCFValuesEnvMin = nil;
		}
		if (MCFValuesEnvMax) {
			delete[] MCFValuesEnvMax;
			MCFValuesEnvMax = nil;
		}
		if (MCF3ValuesEnvMin) {
			delete[] MCF3ValuesEnvMin;
			MCF3ValuesEnvMin = nil;
		}
		if (MCF3ValuesEnvMax) {
			delete[] MCF3ValuesEnvMax;
			MCF3ValuesEnvMax = nil;
		}
		if (MCFgdValuesEnvMin) {
			delete[] MCFgdValuesEnvMin;
			MCFgdValuesEnvMin = nil;
		}
		if (MCFgdValuesEnvMax) {
			delete[] MCFgdValuesEnvMax;
			MCFgdValuesEnvMax = nil;
		}
		if (MCF3gdValuesEnvMin) {
			delete[] MCF3gdValuesEnvMin;
			MCF3gdValuesEnvMin = nil;
		}
		if (MCF3gdValuesEnvMax) {
			delete[] MCF3gdValuesEnvMax;
			MCF3gdValuesEnvMax = nil;
		}
	}
}

// ---------------------------------------------------------------------------
//		¥ Clear
// ---------------------------------------------------------------------------
void
Stats::Clear()
{
	short i;
	
	actualVol = 0;
	observabilityCrit1 = 0;
	observabilityCrit2 = 0;
	AREnvMax = FLT_MIN;
	AREnvMean = 0;
	AREnvMin = FLT_MAX;
	AREnvStdDev = 0;
	avramiRatio = 0;
	BBSides = 0;
	BBVolume = 0;
	circQuadratStatEsd = 0;
	circQuadratStatMean = 0;
	CQSMEnvMax = FLT_MIN;
	CQSMEnvMean = 0;
	CQSMEnvMin = FLT_MAX;
	CQSMEnvStdDev = 0;
	esdActVol = 0;
	esdExtVol = 0;
	esdNNRad = 0;
	esdRadius = 0;
	esdSep = 0;
	extendedVol = 0;
	extendedVolRand = 0;
	fileType = kUndefined;
	IIEnvMax = FLT_MIN;
	IIEnvMean = 0;
	IIEnvMin = FLT_MAX;
	IIEnvStdDev = 0;
	intensity = 0;
	intensitySqd = 0;
	isolationIndex = 0;
	isolationTTest = 0;
	kurtosis = 0;
	maxActV = 0;
	maxExtV = 0;
	maxNNRad = 0;
	maxOverlap = 0;
	maxR = 0;
	maxSep = 0;
	meanActV = 0;
	MeanEVEnvMax = FLT_MIN;
	MeanEVEnvMean = 0;
	MeanEVEnvMin = FLT_MAX;
	MeanEVEnvStdDev = 0;
	meanExtV = 0;
	MeanNNEnvMax = FLT_MIN;
	MeanNNEnvMean = 0;
	MeanNNEnvMin = FLT_MAX;
	MeanNNEnvStdDev = 0;
	meanNNRad = 0;
	meanR = 0;
	meanSep = 0;
	MeanSepEnvMax = FLT_MIN;
	MeanSepEnvMean = 0;
	MeanSepEnvMin = FLT_MAX;
	MeanSepEnvStdDev = 0;
	minActV = 0;
	minDim = 0;
	minExtV = 0;
	minNNRad = 0;
	minR = 0;
	minSep = 0;
	MinSepEnvMax = FLT_MIN;
	MinSepEnvMean = 0;
	MinSepEnvMin = FLT_MAX;
	MinSepEnvStdDev = 0;
	myStat1 = 0;
	myStat2 = 0;
	negRadii = 0;
	newMeanR = 0;
	numCrystals = 0;
	numEngulfed = 0;
	numNNRad = 0;
	numSep = 0;
	numXlsForL = 0;
	OIEnvMax = FLT_MIN;
	OIEnvMean = 0;
	OIEnvMin = FLT_MAX;
	OIEnvStdDev = 0;
	orderingIndex = 0;
	orderingTTest = 0;
	QSMEnvMax = FLT_MIN;
	QSMEnvMean = 0;
	QSMEnvMin = FLT_MAX;
	QSMEnvStdDev = 0;
	quadratStatDOF = 0;
	quadratStatEsd = 0;
	quadratStatMean = 0;
	randSep = 0;
	RPSMEnvMax = FLT_MIN;
	RPSMEnvMean = 0;
	RPSMEnvMin = FLT_MAX;
	RPSMEnvStdDev = 0;
	RPStatEsd = 0;
	RPStatMean = 0;
	skewness = 0;
	surfaceArea = 0;
	surfaceToVolRatio = 0;
	totalPosXls = 0;
	totalVolume = 0;
	volFrxn = 0;
	xlDensity = 0;
	xlVolFraction = 0;
	xMax = FLT_MIN;
	xMin = FLT_MAX;
	yMax = FLT_MIN;
	yMin = FLT_MAX;
	zMax = FLT_MIN;
	zMin = FLT_MAX;	

	for (i = 0; i <= 9; i++)
		deciles[i] = 0.0;
	ClearArrays();
}

// ---------------------------------------------------------------------------
//		¥ ClearArrays
// ---------------------------------------------------------------------------
void
Stats::ClearArrays()
{
	short i;
	
	if (mSigmas) {
		for (i = 1; i <= numLPoints; i++) {
			LValuesEnvMean[i] = 0;
			LValuesEnvStdDev[i] = 0;
			LgdValuesEnvMean[i] = 0;
			LgdValuesEnvStdDev[i] = 0;
			PCFValuesEnvMean[i] = 0;
			PCFValuesEnvStdDev[i] = 0;
			PCFgdValuesEnvMean[i] = 0;
			PCFgdValuesEnvStdDev[i] = 0;
			MCFValuesEnvMean[i] = 0;
			MCFValuesEnvStdDev[i] = 0;
			MCF3ValuesEnvMean[i] = 0;
			MCF3ValuesEnvStdDev[i] = 0;
			MCFgdValuesEnvMean[i] = 0;
			MCFgdValuesEnvStdDev[i] = 0;
			MCF3gdValuesEnvMean[i] = 0;
			MCF3gdValuesEnvStdDev[i] = 0;
		}
	} else {
		for (i = 1; i <= numLPoints; i++) {
			LValuesEnvMin[i] = DBL_MAX;
			LValuesEnvMax[i] = -(DBL_MAX-1);
			LgdValuesEnvMin[i] = DBL_MAX;
			LgdValuesEnvMax[i] = -(DBL_MAX-1);
			PCFValuesEnvMin[i] = DBL_MAX;
			PCFValuesEnvMax[i] = -(DBL_MAX-1);
			PCFgdValuesEnvMin[i] = DBL_MAX;
			PCFgdValuesEnvMax[i] = -(DBL_MAX-1);
			MCFValuesEnvMin[i] = DBL_MAX;
			MCFValuesEnvMax[i] = -(DBL_MAX-1);
			MCF3ValuesEnvMin[i] = DBL_MAX;
			MCF3ValuesEnvMax[i] = -(DBL_MAX-1);
			MCFgdValuesEnvMin[i] = DBL_MAX;
			MCFgdValuesEnvMax[i] = -(DBL_MAX-1);
			MCF3gdValuesEnvMin[i] = DBL_MAX;
			MCF3gdValuesEnvMax[i] = -(DBL_MAX-1);
		}
	}
	for (i = 1; i <= numLPoints; i++) {
		hDistances[i] = 0.0;
		LValues[i] = 0.0;
		LgdValues[i] = 0.0;
		PCF[i] = 0.0;
		PCFgd[i] = 0.0;
		MCF[i] = 0.0;
		MCF3[i] = 0.0;
		MCFgd[i] = 0.0;
		MCF3gd[i] = 0.0;
	}
}
