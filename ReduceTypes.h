// ===========================================================================
//	ReduceTypes.h				
//  Reduce3D
//
//  Created by David Hirsch on 12/1/02.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================

typedef struct {
	bool		includeMeanCSD;
	bool		includeMaxCSD;
	bool		includeLogCSD;
	bool		includeRegCSD;
	bool		doQuadrat;
	bool		doOthers;
	bool		doRandomPt;
	bool		outputSigmas;
	bool		outputCrystals;
	bool		outputHull;
	bool		outputReduce;
	bool		doImpingement;
	bool		doLMcfPcf;
	bool		inflateBBox;
	bool		doShave;
	bool		keepAspectRatios;
	bool		observabilityFilter;
	bool		ShaveSave;
	bool		discardNegs;
	bool		useRaeburn;
	bool		useVolume;
	bool		matchVF;
	bool		makeDCEnv;
	bool		verbose;
	bool		playSoundWhenDone;
	bool		matchPDF;
	bool		exscribedPrimitive;
	bool		specifyTestDistance;
	bool		applyObservabilityFilter;
	short		numClassesMeanCSD;
	short		numClassesMaxCSD;
	short		numClassesLogCSD;
	short		quadratNumPlacings;
	short		quadratNumReps;
	short		RPNumPlacings;
	short		RPNumReps;
	short		numNNDist;
	short		numCFOffsetVolPts;
	short		sampleShape;
	short		numEnvelopeRuns;
	short		shaveXYZ;
	short		direction;
	short		minPopulation;
	short		confidence;
	short		overlap;
	short		shaveSmLg;
	short		observabilityMethod;
	long		seed;
	long		MCReps;
	double		maxValueMeanCSD;
	double		deltaLRegCSD;
	double		impingementMeanErr;
	double		impingementMaxErr;
	double		EpanecnikovCVal;
	double		testDistanceInterval;
	double		crit1Factor;
	double		crit2Factor;
	double		observabilityPercent;
	double		shaveIncrement;
	double		minPercent;
	double		maxAspectRatio;
	double		VFPercent;
	double		shrinkExscribedPrimitive;
} PrefStruct;

typedef struct {
	short	halfSines;
	short	start;
	double	minProb;
} nuclProb1D;

typedef struct {
	nuclProb1D x;
	nuclProb1D y;
	nuclProb1D z;
} nuclProbType;

typedef struct DistArrayElem {
	short	index;
	double	dist;
} DistArrayElem;
