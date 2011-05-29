/*
 *  Calculator.h
 *  Reduce3D
 *
 *  Created by David Hirsch on 12/2/07.
 *  Copyright 2007 David Hirsch. All rights reserved.
 *
 *  This class exists actually do the work.  Everything conceptually "below" this should be pure C++.  This header is
 *  This header is pure C++ so it can be included by .cp files, but the implementation mixes Obj-C (to work with the 
 *  Mediator) and C++ (to work with things like CrystalArray).
 */

#include <Carbon/Carbon.h>
#include <string>
#include <vector>
#include "ReduceTypes.h"

class stringFile;
class BoundingBox;
class SideSet;
class Stats;
class HoleSet;
class CrystalArray;
class Point3DFloat;
class Crystal;
class OneSet;
class CFloatArray;
class EnvelopeData;

class Calculator {
public:
	class CalcError{};
	class QuadratFailed{};
	class SimulationErr{};
	
	Calculator(void *inMed, void *inCont, PrefStruct *inPrefs);
	bool			runAnalysis(std::vector<std::string> inputFiles, std::string holesFile);
	void			reduceData(std::string inputFilePath, std::string holesFilePath);
	bool			shaveAndReduceData(std::string inputFilePath);
	bool			reduceDataInFile(std::string inputFilePath, std::string holesFilePath);
	bool			shaveData(BoundingBox *inStartBox, BoundingBox *ioBBox, short iteration);
	void			reduceOneDataset(BoundingBox *ioBBox, HoleSet *inHoles);
	void			setProgMessage(const char *inMessage);
	void			setProgTitle(const char *inStr);
	void			setShaveMessage(const char *inMessage);
	void			setEnvMessage(const char *inEnvMessage);
	void			setupProgress(const char *inMainMessage, 
								  const char *inShaveMessage, 
								  const char *inEnvMessage, 
								  const char *inTitle, 
								  short inInc, 
								  double inMin, 
								  double inMax, 
								  double inCur, 
								  bool inInd);
	void			progress(double inCur);
	void			log(const char *inMessage);
	void			log(std::string inMessage);
	PrefStruct *	getPrefs();
	short			postError(const char *inMessage, 
							  const char *inTitle, 
							  const char *inButtonStr, 
							  short inDefault, 
							  float inDismiss);
	bool			shouldStopCalculating();
	void			calcStats(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles);
	void			saveResults(Stats *inStats, BoundingBox *inBBox, HoleSet *inHoles, short shaveIteration);
	void			DoEnvelopeSimulations(BoundingBox *ioBBox, Stats *ioStats, HoleSet *inHoles);
	float			OffsetIntersectVol(BoundingBox *inBBox, Point3DFloat inPt1, Point3DFloat inPt2);
	
protected:
		// CalculatorComputation methods:
	void					FindBestPrimitive(BoundingBox *ioBBox, HoleSet *inHoles);
	void					FindBestExscRPPrimitive(BoundingBox *ioBBox);
	void					FindBestExscCylPrimitive(BoundingBox *ioBBox);
	void					NewFindBestRPPrimitive(SideSet &ioPrim, BoundingBox *inBBox, HoleSet *inHoles);
	void					PutPrimInMiddle(SideSet &ioPrim, BoundingBox *inBBox, short tryNum = -1);
	void					AdaptToPrimitiveBox(Stats *stats, BoundingBox *ioBBox);

	void					DoInitialStats(Stats *ioStats, BoundingBox *inBBox, HoleSet *inHoles);
	void					ComputeVolumes(Stats *ioStats, BoundingBox *inBBox, HoleSet *inHoles);
	void					ComputeRadiusStats(Stats *ioStats, BoundingBox *inBBox, HoleSet *inHoles);
	void					ComputeNeighbors(Stats *ioStats, BoundingBox *inBBox, HoleSet *inHoles);
	void					ComputeNNStats(Stats *ioStats, BoundingBox *inBBox, HoleSet *inHoles);
	void					ComputeAvramiTest(Stats *ioStats, BoundingBox *inBBox, HoleSet *inHoles);
	void					ComputeCSDStats(Stats *ioStats, BoundingBox *inBBox, HoleSet *inHoles);
	void					ComputeQuadrat(Stats *ioStats, BoundingBox *inBBox, HoleSet *inHoles);
	void					ComputeRandomPt(Stats *ioStats, BoundingBox *inBBox, HoleSet *inHoles);
	void					ComputeCFs_Both(Stats *ioStats, BoundingBox *inBBox, HoleSet *inHoles);
	float					FindSwissVolume(BoundingBox *inBBox, HoleSet *inHoles);
	Point3DFloat &			RandPtNearBox(Point3DFloat &inMinPoint, Point3DFloat &inMaxPoint);
	bool					InBoxNotHole(BoundingBox *inBBox, HoleSet *inHoles, Point3DFloat &inPoint);
	double					CorrectedXlVol(Stats *stats, CrystalArray *inXls, short curIndex, Crystal &currXl);
	void					ExtractEnvData(Stats *inStats, OneSet *ioEnv);
	Point3DFloat &			Get3DRandPt(BoundingBox *inBBox, HoleSet *inHoles);
	Point3DFloat &			Get3DRandQuad(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles, float quadSize);
	Point3DFloat &			Get3DRandSphere(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles, float quadSize);
	double					NearestSideHoleDist(BoundingBox *inBBox, HoleSet *inHoles, Point3DFloat *inPt);
	void					DoMeanCumCSD(Stats *stats, BoundingBox *inBBox, short shaveIteration);
	void					DoMaxCumCSD(Stats *stats, BoundingBox *inBBox, short shaveIteration);
	void					DoLogCSD(Stats *stats, BoundingBox *inBBox, short shaveIteration);
	void					DoNNCSD(Stats *stats, BoundingBox *inBBox, short shaveIteration);
	void					DoRegCSD(Stats *stats, BoundingBox *inBBox, short shaveIteration);
	void					DoReduce3DFile(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles, short shaveIteration);
	void					DoRedStatsFile(Stats *stats, BoundingBox *inBBox, short shaveIteration);
	void					SaveCSD(BoundingBox *inBBox, Stats *stats, short shaveIteration, CFloatArray &inCSD, short inCSDKind, float binSize=0);
	const char *			MakeCSDFilePath(BoundingBox *inBBox, short inCSDKind);
	void					WritePrefsSettings(stringFile *saveFile);
	void					WriteIntFile(short inEnvRun = -1, float inOldMeanR = -1.0, float inNewMeanR = -1.0);
	short					FindInsertIndex(DistArrayElem *inArray, short inNumElems, float &inVal);
	float					Epanecnikov(float h, float t);
	float					FindIntensity(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles);
	float					FindIntensitySquared(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles);
	void					EvaluateCFStats(Stats *stats, EnvelopeData &Envelopes, bool inDoSigmas, short inConfidence);
	float					MakeGoodRandomSimulation(BoundingBox *inBBox, HoleSet *inHoles, std::vector<double> &inRadiiList, float inVolFraction, bool inMatchingCTDataSet);
	void					MakeShuffleArray(short *inArray);
	bool					GoodCFVal(bool inDoSigmas, double inVal);
	double					FlaggedCFVal(bool inDoSigmas, double inVal);
		
protected:
	void *					mediator;	// the Mediator object, given here as a void * so we don't have to include the Obj-C Mediator header
										// (and we can't forward declare an Obj-C class
	void *					controller; // Just as above, this is the AppController, given as a void *
	PrefStruct *			prefs;
	stringFile *			mLogFile;
	double					progMin, progMax, lastProgCall, progInc;
};