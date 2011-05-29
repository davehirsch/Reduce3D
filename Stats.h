// ===========================================================================
//	Stats.h
//  Reduce3D
//
//  Created by David Hirsch on 11/25/07.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================

#pragma once
#import "ReduceConstants.h"

// ---------------------------------------------------------------------------
//		¥ Stats
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
		double	*hDistances;
		
		short	observabilityCrit1rejects;
		short	observabilityCrit2rejects;
		double	observabilityCrit1value;
		double	observabilityCrit2value;
	
		double	actualVol;			
		double	AREnvMax;			
		double 	AREnvMean;			
		double 	AREnvMin;			
		double	AREnvStdDev;			
		double	avramiRatio;			
		short	BBSides;			// number of sides to the Bounding Box
		double	BBVolume;			// the total Volume in 3-D crystals; from Integrate file
		double	circQuadratStatEsd;			
		double	circQuadratStatMean;			
		double	CQSMEnvMax;			
		double	CQSMEnvMean;			
		double	CQSMEnvMin;			
		double	CQSMEnvStdDev;			
		double	deciles[10];			
		double	esdActVol;			
		double	esdExtVol;			
		double	esdNNRad;			
		double	esdRadius;			
		double	esdSep;			
		double	extendedVol;			
		double	extendedVolRand;			
		fileTypes	fileType;			// real data, or type of simulation
		double	IIEnvMax;			
		double	IIEnvMean;			
		double	IIEnvMin;			
		double	IIEnvStdDev;			
		double	intensity;			// mean number of xls per unit volume
		double	intensitySqd;			
		double	isolationIndex;			
		double	isolationTTest;			
		double	kurtosis;			
		double	maxActV;
		double	maxExtV;
		double	maxNNRad;
		double	maxOverlap;
		double	maxR;
		double	maxSep;
		double	meanActV;
		double	MeanEVEnvMax;
		double	MeanEVEnvMean;
		double	MeanEVEnvMin;
		double	MeanEVEnvStdDev;
		double	meanExtV;
		double	MeanNNEnvMax;
		double	MeanNNEnvMean;
		double	MeanNNEnvMin;
		double	MeanNNEnvStdDev;
		double	meanNNRad;
		double	meanR;
		double	meanSep;
		double	MeanSepEnvMax;
		double	MeanSepEnvMean;
		double	MeanSepEnvMin;
		double	MeanSepEnvStdDev;
		double	minActV;
		double	minDim;
		double	minExtV;
		double	minNNRad;
		double	minR;
		double	minSep;
		double	MinSepEnvMax;
		double	MinSepEnvMean;
		double	MinSepEnvMin;
		double	MinSepEnvStdDev;
		double	myStat1;
		double	myStat2;
		short	negRadii;
		double 	newMeanR;
		short	numCrystals;			// current number of 3-D crystals found
		short	numEngulfed;
		short	numLPoints;
		short	numNNRad;
		short	numSep;
		short	numXlsForL;
		double	OIEnvMax;
		double	OIEnvMean;
		double	OIEnvMin;
		double	OIEnvStdDev;
		double	orderingIndex;			
		double	orderingTTest;
		double	QSMEnvMax;
		double	QSMEnvMean;
		double	QSMEnvMin;
		double	QSMEnvStdDev;
		short	quadratStatDOF;
		double	quadratStatEsd;
		double	quadratStatMean;
		double	randSep;
		double	RPSMEnvMax;
		double	RPSMEnvMean;
		double	RPSMEnvMin;
		double	RPSMEnvStdDev;
		double	RPStatEsd;
		double	RPStatMean;
		double	skewness;
		double	surfaceArea;
		double	surfaceToVolRatio;
		short	totalPosXls;			
		double	totalVolume;			// the total Volume in 3-D crystals; from Integrate file
		double	volFrxn;
		double	xlDensity;
		double	xlVolFraction;
		double	xMin, xMax, yMin, yMax, zMin, zMax;

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
