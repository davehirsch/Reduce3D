// ===========================================================================
//	Stats.h
// ===========================================================================
#pragma once
#import "ReduceConstants.h"

// ---------------------------------------------------------------------------
//		� Stats
// ---------------------------------------------------------------------------

#ifndef nil
#define nil 0
#endif

class	Stats {
	public:
				Stats(bool inRealDataset);
				~Stats();
	void		SetCFParams(short inNumKPoints, bool inSigma);
	void		Clear();
	void		ClearArrays();

	public:
		bool	forRealDataset;
		bool	mSigmas;
		float	*hDistances;

		float	actualVol;			
		float	AREnvMax;			
		float 	AREnvMean;			
		float 	AREnvMin;			
		float	AREnvStdDev;			
		float	avramiRatio;			
		short	BBSides;			// number of sides to the Bounding Box
		float	BBVolume;			// the total Volume in 3-D crystals; from Integrate file
		float	circQuadratStatEsd;			
		float	circQuadratStatMean;			
		float	CQSMEnvMax;			
		float	CQSMEnvMean;			
		float	CQSMEnvMin;			
		float	CQSMEnvStdDev;			
		float	deciles[10];			
		float	esdActVol;			
		float	esdExtVol;			
		float	esdNNRad;			
		float	esdRadius;			
		float	esdSep;			
		float	extendedVol;			
		float	extendedVolRand;			
		fileTypes	fileType;			// real data, or type of simulation
		float	IIEnvMax;			
		float	IIEnvMean;			
		float	IIEnvMin;			
		float	IIEnvStdDev;			
		float	intensity;			// mean number of xls per unit volume
		float	intensitySqd;			
		float	isolationIndex;			
		float	isolationTTest;			
		float	kurtosis;			
		float	maxActV;
		float	maxExtV;
		float	maxNNRad;
		float	maxOverlap;
		float	maxR;
		float	maxSep;
		float	meanActV;
		float	MeanEVEnvMax;
		float	MeanEVEnvMean;
		float	MeanEVEnvMin;
		float	MeanEVEnvStdDev;
		float	meanExtV;
		float	MeanNNEnvMax;
		float	MeanNNEnvMean;
		float	MeanNNEnvMin;
		float	MeanNNEnvStdDev;
		float	meanNNRad;
		float	meanR;
		float	meanSep;
		float	MeanSepEnvMax;
		float	MeanSepEnvMean;
		float	MeanSepEnvMin;
		float	MeanSepEnvStdDev;
		float	minActV;
		float	minDim;
		float	minExtV;
		float	minNNRad;
		float	minR;
		float	minSep;
		float	MinSepEnvMax;
		float	MinSepEnvMean;
		float	MinSepEnvMin;
		float	MinSepEnvStdDev;
		float	myStat1;
		float	myStat2;
		short	negRadii;
		float 	newMeanR;
		short	numCrystals;			// current number of 3-D crystals found
		short	numEngulfed;
		short	numLPoints;
		short	numNNRad;
		short	numSep;
		short	numXlsForL;
		float	OIEnvMax;
		float	OIEnvMean;
		float	OIEnvMin;
		float	OIEnvStdDev;
		float	orderingIndex;			
		float	orderingTTest;
		float	QSMEnvMax;
		float	QSMEnvMean;
		float	QSMEnvMin;
		float	QSMEnvStdDev;
		short	quadratStatDOF;
		float	quadratStatEsd;
		float	quadratStatMean;
		float	randSep;
		float	RPSMEnvMax;
		float	RPSMEnvMean;
		float	RPSMEnvMin;
		float	RPSMEnvStdDev;
		float	RPStatEsd;
		float	RPStatMean;
		float	skewness;
		float	surfaceArea;
		float	surfaceToVolRatio;
		short	totalPosXls;			
		float	totalVolume;			// the total Volume in 3-D crystals; from Integrate file
		float	volFrxn;
		float	xlDensity;
		float	xlVolFraction;
		float	xMin, xMax, yMin, yMax, zMin, zMax;

		double	*LValues;
		double	*LgdValues;
		double	*PCF;
		double	*PCFgd;
		double	*MCF;
		double	*MCF3;
		double	*MCFgd;
		double	*MCF3gd;

		double	*LValuesEnvMin;
		double	*LValuesEnvMax;
		double	*LgdValuesEnvMin;
		double	*LgdValuesEnvMax;
		double	*PCFValuesEnvMin;
		double	*PCFValuesEnvMax;
		double	*PCFgdValuesEnvMin;
		double	*PCFgdValuesEnvMax;
		double	*MCFValuesEnvMin;
		double	*MCFValuesEnvMax;
		double	*MCF3ValuesEnvMin;
		double	*MCF3ValuesEnvMax;
		double	*MCFgdValuesEnvMin;
		double	*MCFgdValuesEnvMax;
		double	*MCF3gdValuesEnvMin;
		double	*MCF3gdValuesEnvMax;

		double	*LValuesEnvMean;
		double	*LValuesEnvStdDev;
		double	*LgdValuesEnvMean;
		double	*LgdValuesEnvStdDev;
		double	*PCFValuesEnvMean;
		double	*PCFValuesEnvStdDev;
		double	*PCFgdValuesEnvMean;
		double	*PCFgdValuesEnvStdDev;
		double	*MCFValuesEnvMean;
		double	*MCFValuesEnvStdDev;
		double	*MCF3ValuesEnvMean;
		double	*MCF3ValuesEnvStdDev;
		double	*MCFgdValuesEnvMean;
		double	*MCFgdValuesEnvStdDev;
		double	*MCF3gdValuesEnvMean;
		double	*MCF3gdValuesEnvStdDev;
		
};
