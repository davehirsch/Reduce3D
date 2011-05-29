// CReducePrefs.cp 2.0// This is a major revision that now uses CFPreferences functions to make dealing with//	prefs much easier#include "CReducePrefs.h"#include "ReduceConstants.h"/* Tasks: When launching, find the preference file, or make one with default values if not found.	Then, load preference values into the preference structure.	Later, we must write any new values to the preference file after the prefStruct has changed.*///----------------------------------// CReducePrefs constructor//----------------------------------CReducePrefs::CReducePrefs(){	mPrefsPtr = new PrefStruct();	mDefaultValuesPtr = new PrefStruct();	mDefaultValuesPtr->includeMeanCSD = false;	mDefaultValuesPtr->includeMaxCSD = false;	mDefaultValuesPtr->includeLogCSD = false;	mDefaultValuesPtr->includeRegCSD = false;	mDefaultValuesPtr->doQuadrat = true;	mDefaultValuesPtr->doOthers = true;	mDefaultValuesPtr->doRandomPt = true;	mDefaultValuesPtr->make3DModel = false;	mDefaultValuesPtr->outputSigmas = false;	mDefaultValuesPtr->outputCrystals = false;	mDefaultValuesPtr->outputR3STT = false;	mDefaultValuesPtr->outputReduce = true;	mDefaultValuesPtr->doImpingement = true;	mDefaultValuesPtr->numClassesMeanCSD = 63;	mDefaultValuesPtr->numClassesMaxCSD = 63;	mDefaultValuesPtr->numClassesLogCSD = 63;	mDefaultValuesPtr->quadratNumPlacings = 1000;	mDefaultValuesPtr->quadratNumReps = 10;	mDefaultValuesPtr->RPNumPlacings = 1000;	mDefaultValuesPtr->RPNumReps = 10;	mDefaultValuesPtr->maxValueMeanCSD = 3.00;	mDefaultValuesPtr->deltaLRegCSD = 0.10;	mDefaultValuesPtr->impingementMeanErr = 0.10;	mDefaultValuesPtr->impingementMaxErr = 20.00;	mDefaultValuesPtr->doLMcfPcf = true;	mDefaultValuesPtr->numNNDist = 6;	mDefaultValuesPtr->numCFOffsetVolPts = 500;	mDefaultValuesPtr->EpanecnikovCVal = 0.1;	mDefaultValuesPtr->numQD3DEqPts = 5;	mDefaultValuesPtr->numQD3DLngPts = 5;	mDefaultValuesPtr->sampleShape = kRectPrism;	mDefaultValuesPtr->numEnvelopeRuns = 100;	mDefaultValuesPtr->inflateBBox = false;	mDefaultValuesPtr->observabilityFilter = true;	mDefaultValuesPtr->crit1Factor = 0.85;	mDefaultValuesPtr->crit2Factor = 3.0;	mDefaultValuesPtr->doShave = false;	mDefaultValuesPtr->keepAspectRatios = true;	mDefaultValuesPtr->shaveXYZ = 1;	mDefaultValuesPtr->shaveIncrement = 10.0;	mDefaultValuesPtr->direction = 2;	mDefaultValuesPtr->minPercent = 10.0;	mDefaultValuesPtr->minPopulation = 200;	mDefaultValuesPtr->maxAspectRatio = 1000.0;	mDefaultValuesPtr->smaller = false;	mDefaultValuesPtr->ShaveSave = false;	mDefaultValuesPtr->discardNegs = true;	mDefaultValuesPtr->useRaeburn = false;	mDefaultValuesPtr->confidence = 96;	mDefaultValuesPtr->seed = 1234567;	mDefaultValuesPtr->MCReps = 5000;	mDefaultValuesPtr->tidyUp = false;	mDefaultValuesPtr->useVolume = false;	mDefaultValuesPtr->overlap = 50;	mDefaultValuesPtr->matchVF = true;	mDefaultValuesPtr->makeDCEnv = false;	mDefaultValuesPtr->verbose = false;	mDefaultValuesPtr->VFPercent = 10.0;	mDefaultValuesPtr->matchPDF = false;	mDefaultValuesPtr->discard = 0.0;	InitPreferences();}//----------------------------------// CReducePrefs default destructor//	delete file object and any other handles.//----------------------------------CReducePrefs::~CReducePrefs(void){	if (mPrefsPtr != nil)		delete mPrefsPtr;	mPrefsPtr = nil;	if (mDefaultValuesPtr != nil)		delete mDefaultValuesPtr;	mDefaultValuesPtr = nil;}PrefStruct *CReducePrefs::GetPreferencesPtr(){	return mPrefsPtr;}PrefStruct *CReducePrefs::GetDefaultPreferencesPtr(){	return mDefaultValuesPtr;}//----------------------------------// InitPreferences//	For each preference, this function will check to see if there is a value stored.//	If so, it will put that value into the primary preference structure.//	If not, it will set the default value as the preference value.//----------------------------------voidCReducePrefs::InitPreferences(){	Boolean	validResult;	// Boolean preferences	mPrefsPtr->includeMeanCSD = CFPreferencesGetAppBooleanValue(					CFSTR("includeMeanCSD"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->includeMeanCSD = mDefaultValuesPtr->includeMeanCSD;	mPrefsPtr->includeMaxCSD = CFPreferencesGetAppBooleanValue(					CFSTR("includeMaxCSD"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->includeMaxCSD = mDefaultValuesPtr->includeMaxCSD;	mPrefsPtr->includeLogCSD = CFPreferencesGetAppBooleanValue(					CFSTR("includeLogCSD"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->includeLogCSD = mDefaultValuesPtr->includeLogCSD;	mPrefsPtr->includeRegCSD = CFPreferencesGetAppBooleanValue(					CFSTR("includeRegCSD"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->includeRegCSD = mDefaultValuesPtr->includeRegCSD;	mPrefsPtr->doQuadrat = CFPreferencesGetAppBooleanValue(					CFSTR("doQuadrat"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->doQuadrat = mDefaultValuesPtr->doQuadrat;	mPrefsPtr->doOthers = CFPreferencesGetAppBooleanValue(					CFSTR("doOthers"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->doOthers = mDefaultValuesPtr->doOthers;	mPrefsPtr->doRandomPt = CFPreferencesGetAppBooleanValue(					CFSTR("doRandomPt"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->doRandomPt = mDefaultValuesPtr->doRandomPt;	mPrefsPtr->make3DModel = CFPreferencesGetAppBooleanValue(					CFSTR("make3DModel"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->make3DModel = mDefaultValuesPtr->make3DModel;	mPrefsPtr->outputSigmas = CFPreferencesGetAppBooleanValue(					CFSTR("outputSigmas"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->outputSigmas = mDefaultValuesPtr->outputSigmas;	mPrefsPtr->outputCrystals = CFPreferencesGetAppBooleanValue(					CFSTR("outputCrystals"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->outputCrystals = mDefaultValuesPtr->outputCrystals;	mPrefsPtr->outputR3STT = CFPreferencesGetAppBooleanValue(					CFSTR("outputR3STT"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->outputR3STT = mDefaultValuesPtr->outputR3STT;	mPrefsPtr->outputReduce = CFPreferencesGetAppBooleanValue(					CFSTR("outputReduce"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->outputReduce = mDefaultValuesPtr->outputReduce;	mPrefsPtr->doImpingement = CFPreferencesGetAppBooleanValue(					CFSTR("doImpingement"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->doImpingement = mDefaultValuesPtr->doImpingement;	mPrefsPtr->doLMcfPcf = CFPreferencesGetAppBooleanValue(					CFSTR("doLMcfPcf"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->doLMcfPcf = mDefaultValuesPtr->doLMcfPcf;	mPrefsPtr->inflateBBox = CFPreferencesGetAppBooleanValue(					CFSTR("inflateBBox"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->inflateBBox = mDefaultValuesPtr->inflateBBox;	mPrefsPtr->doShave = CFPreferencesGetAppBooleanValue(					CFSTR("doShave"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->doShave = mDefaultValuesPtr->doShave;	mPrefsPtr->keepAspectRatios = CFPreferencesGetAppBooleanValue(					CFSTR("keepAspectRatios"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->keepAspectRatios = mDefaultValuesPtr->keepAspectRatios;	mPrefsPtr->observabilityFilter = CFPreferencesGetAppBooleanValue(					CFSTR("observabilityFilter"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->observabilityFilter = mDefaultValuesPtr->observabilityFilter;	mPrefsPtr->smaller = CFPreferencesGetAppBooleanValue(					CFSTR("smaller"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->smaller = mDefaultValuesPtr->smaller;	mPrefsPtr->ShaveSave = CFPreferencesGetAppBooleanValue(					CFSTR("ShaveSave"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->ShaveSave = mDefaultValuesPtr->ShaveSave;	mPrefsPtr->discardNegs = CFPreferencesGetAppBooleanValue(					CFSTR("discardNegs"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->discardNegs = mDefaultValuesPtr->discardNegs;	mPrefsPtr->useRaeburn = CFPreferencesGetAppBooleanValue(					CFSTR("useRaeburn"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->useRaeburn = mDefaultValuesPtr->useRaeburn;	mPrefsPtr->tidyUp = CFPreferencesGetAppBooleanValue(					CFSTR("tidyUp"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->tidyUp = mDefaultValuesPtr->tidyUp;	mPrefsPtr->useVolume = CFPreferencesGetAppBooleanValue(					CFSTR("useVolume"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->useVolume = mDefaultValuesPtr->useVolume;	mPrefsPtr->matchVF = CFPreferencesGetAppBooleanValue(					CFSTR("matchVF"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->matchVF = mDefaultValuesPtr->matchVF;	mPrefsPtr->makeDCEnv = CFPreferencesGetAppBooleanValue(					CFSTR("makeDCEnv"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->makeDCEnv = mDefaultValuesPtr->makeDCEnv;	mPrefsPtr->verbose = CFPreferencesGetAppBooleanValue(					CFSTR("verbose"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->verbose = mDefaultValuesPtr->verbose;	mPrefsPtr->matchPDF = CFPreferencesGetAppBooleanValue(					CFSTR("matchPDF"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->matchPDF = mDefaultValuesPtr->matchPDF;	mPrefsPtr->fixClaude = CFPreferencesGetAppBooleanValue(					CFSTR("fixClaude"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->fixClaude = mDefaultValuesPtr->fixClaude;	// SInt16 Preferences	mPrefsPtr->numClassesMeanCSD = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("numClassesMeanCSD"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->numClassesMeanCSD = mDefaultValuesPtr->numClassesMeanCSD;		mPrefsPtr->numClassesMaxCSD = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("numClassesMaxCSD"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->numClassesMaxCSD = mDefaultValuesPtr->numClassesMaxCSD;	mPrefsPtr->numClassesLogCSD = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("numClassesLogCSD"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->numClassesLogCSD = mDefaultValuesPtr->numClassesLogCSD;	mPrefsPtr->quadratNumPlacings = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("quadratNumPlacings"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->quadratNumPlacings = mDefaultValuesPtr->quadratNumPlacings;	mPrefsPtr->quadratNumReps = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("quadratNumReps"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->quadratNumReps = mDefaultValuesPtr->quadratNumReps;	mPrefsPtr->RPNumPlacings = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("RPNumPlacings"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->RPNumPlacings = mDefaultValuesPtr->RPNumPlacings;	mPrefsPtr->RPNumReps = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("RPNumReps"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->RPNumReps = mDefaultValuesPtr->RPNumReps;	mPrefsPtr->numNNDist = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("numNNDist"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->numNNDist = mDefaultValuesPtr->numNNDist;	mPrefsPtr->numCFOffsetVolPts = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("numCFOffsetVolPts"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->numCFOffsetVolPts = mDefaultValuesPtr->numCFOffsetVolPts;	mPrefsPtr->numQD3DEqPts = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("numQD3DEqPts"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->numQD3DEqPts = mDefaultValuesPtr->numQD3DEqPts;	mPrefsPtr->numQD3DLngPts = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("numQD3DLngPts"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->numQD3DLngPts = mDefaultValuesPtr->numQD3DLngPts;	mPrefsPtr->sampleShape = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("sampleShape"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->sampleShape = mDefaultValuesPtr->sampleShape;	mPrefsPtr->numEnvelopeRuns = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("numEnvelopeRuns"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->numEnvelopeRuns = mDefaultValuesPtr->numEnvelopeRuns;	mPrefsPtr->shaveXYZ = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("shaveXYZ"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->shaveXYZ = mDefaultValuesPtr->shaveXYZ;	mPrefsPtr->direction = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("direction"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->direction = mDefaultValuesPtr->direction;	mPrefsPtr->minPopulation = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("minPopulation"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->minPopulation = mDefaultValuesPtr->minPopulation;	mPrefsPtr->confidence = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("confidence"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->confidence = mDefaultValuesPtr->confidence;	mPrefsPtr->overlap = (SInt16) CFPreferencesGetAppIntegerValue(					CFSTR("overlap"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->overlap = mDefaultValuesPtr->overlap;	// SInt32 Preferences	mPrefsPtr->seed = (SInt32) CFPreferencesGetAppIntegerValue(					CFSTR("seed"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->seed = mDefaultValuesPtr->seed;	mPrefsPtr->MCReps = (SInt32) CFPreferencesGetAppIntegerValue(					CFSTR("MCReps"), 					kCFPreferencesCurrentApplication, 					&validResult);	if (!validResult) mPrefsPtr->MCReps = mDefaultValuesPtr->MCReps;	// Float Preferences	CFNumberRef    tempNumRef;	tempNumRef = (CFNumberRef) CFPreferencesCopyAppValue(					CFSTR("maxValueMeanCSD"), 					kCFPreferencesCurrentApplication);	if (tempNumRef == NULL) {		mPrefsPtr->maxValueMeanCSD = mDefaultValuesPtr->maxValueMeanCSD;	} else {		CFNumberGetValue( tempNumRef, kCFNumberFloatType, &(mPrefsPtr->maxValueMeanCSD) );		CFRelease(tempNumRef);	}	tempNumRef = (CFNumberRef) CFPreferencesCopyAppValue(					CFSTR("deltaLRegCSD"), 					kCFPreferencesCurrentApplication);	if (tempNumRef == NULL) {		mPrefsPtr->deltaLRegCSD = mDefaultValuesPtr->deltaLRegCSD;	} else {		CFNumberGetValue( tempNumRef, kCFNumberFloatType, &(mPrefsPtr->deltaLRegCSD) );		CFRelease(tempNumRef);	}	tempNumRef = (CFNumberRef) CFPreferencesCopyAppValue(					CFSTR("impingementMeanErr"), 					kCFPreferencesCurrentApplication);	if (tempNumRef == NULL) {		mPrefsPtr->impingementMeanErr = mDefaultValuesPtr->impingementMeanErr;	} else {		CFNumberGetValue( tempNumRef, kCFNumberFloatType, &(mPrefsPtr->impingementMeanErr) );		CFRelease(tempNumRef);	}	tempNumRef = (CFNumberRef) CFPreferencesCopyAppValue(					CFSTR("impingementMaxErr"), 					kCFPreferencesCurrentApplication);	if (tempNumRef == NULL) {		mPrefsPtr->impingementMaxErr = mDefaultValuesPtr->impingementMaxErr;	} else {		CFNumberGetValue( tempNumRef, kCFNumberFloatType, &(mPrefsPtr->impingementMaxErr) );		CFRelease(tempNumRef);	}	tempNumRef = (CFNumberRef) CFPreferencesCopyAppValue(					CFSTR("EpanecnikovCVal"), 					kCFPreferencesCurrentApplication);	if (tempNumRef == NULL) {		mPrefsPtr->EpanecnikovCVal = mDefaultValuesPtr->EpanecnikovCVal;	} else {		CFNumberGetValue( tempNumRef, kCFNumberFloatType, &(mPrefsPtr->EpanecnikovCVal) );		CFRelease(tempNumRef);	}	tempNumRef = (CFNumberRef) CFPreferencesCopyAppValue(					CFSTR("crit1Factor"), 					kCFPreferencesCurrentApplication);	if (tempNumRef == NULL) {		mPrefsPtr->crit1Factor = mDefaultValuesPtr->crit1Factor;	} else {		CFNumberGetValue( tempNumRef, kCFNumberFloatType, &(mPrefsPtr->crit1Factor) );		CFRelease(tempNumRef);	}	tempNumRef = (CFNumberRef) CFPreferencesCopyAppValue(					CFSTR("crit2Factor"), 					kCFPreferencesCurrentApplication);	if (tempNumRef == NULL) {		mPrefsPtr->crit2Factor = mDefaultValuesPtr->crit2Factor;	} else {		CFNumberGetValue( tempNumRef, kCFNumberFloatType, &(mPrefsPtr->crit2Factor) );		CFRelease(tempNumRef);	}	tempNumRef = (CFNumberRef) CFPreferencesCopyAppValue(					CFSTR("shaveIncrement"), 					kCFPreferencesCurrentApplication);	if (tempNumRef == NULL) {		mPrefsPtr->shaveIncrement = mDefaultValuesPtr->shaveIncrement;	} else {		CFNumberGetValue( tempNumRef, kCFNumberFloatType, &(mPrefsPtr->shaveIncrement) );		CFRelease(tempNumRef);	}	tempNumRef = (CFNumberRef) CFPreferencesCopyAppValue(					CFSTR("minPercent"), 					kCFPreferencesCurrentApplication);	if (tempNumRef == NULL) {		mPrefsPtr->minPercent = mDefaultValuesPtr->minPercent;	} else {		CFNumberGetValue( tempNumRef, kCFNumberFloatType, &(mPrefsPtr->minPercent) );		CFRelease(tempNumRef);	}	tempNumRef = (CFNumberRef) CFPreferencesCopyAppValue(					CFSTR("maxAspectRatio"), 					kCFPreferencesCurrentApplication);	if (tempNumRef == NULL) {		mPrefsPtr->maxAspectRatio = mDefaultValuesPtr->maxAspectRatio;	} else {		CFNumberGetValue( tempNumRef, kCFNumberFloatType, &(mPrefsPtr->maxAspectRatio) );		CFRelease(tempNumRef);	}	tempNumRef = (CFNumberRef) CFPreferencesCopyAppValue(					CFSTR("VFPercent"), 					kCFPreferencesCurrentApplication);	if (tempNumRef == NULL) {		mPrefsPtr->VFPercent = mDefaultValuesPtr->VFPercent;	} else {		CFNumberGetValue( tempNumRef, kCFNumberFloatType, &(mPrefsPtr->VFPercent) );		CFRelease(tempNumRef);	}	tempNumRef = (CFNumberRef) CFPreferencesCopyAppValue(					CFSTR("discard"), 					kCFPreferencesCurrentApplication);	if (tempNumRef == NULL) {		mPrefsPtr->discard = mDefaultValuesPtr->discard;	} else {		CFNumberGetValue( tempNumRef, kCFNumberFloatType, &(mPrefsPtr->discard) );		CFRelease(tempNumRef);	}	SavePreferences();}//----------------------------------// SavePreferences//	For each preference, this function stores the current value as the default//----------------------------------voidCReducePrefs::SavePreferences(){	// Boolean Preferences	CFPreferencesSetAppValue (CFSTR("includeMeanCSD"),		(mPrefsPtr->includeMeanCSD ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("includeMaxCSD"),		(mPrefsPtr->includeMaxCSD ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("includeLogCSD"),		(mPrefsPtr->includeLogCSD ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("includeRegCSD"),		(mPrefsPtr->includeRegCSD ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("doQuadrat"),		(mPrefsPtr->doQuadrat ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("doOthers"),		(mPrefsPtr->doOthers ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("doRandomPt"),		(mPrefsPtr->doRandomPt ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("make3DModel"),		(mPrefsPtr->make3DModel ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("outputSigmas"),		(mPrefsPtr->outputSigmas ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("outputCrystals"),		(mPrefsPtr->outputCrystals ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("outputR3STT"),		(mPrefsPtr->outputR3STT ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("outputReduce"),		(mPrefsPtr->outputReduce ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("doImpingement"),		(mPrefsPtr->doImpingement ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("doLMcfPcf"),		(mPrefsPtr->doLMcfPcf ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("inflateBBox"),		(mPrefsPtr->inflateBBox ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("doShave"),		(mPrefsPtr->doShave ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("keepAspectRatios"),		(mPrefsPtr->keepAspectRatios ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("observabilityFilter"),		(mPrefsPtr->observabilityFilter ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("smaller"),		(mPrefsPtr->smaller ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("ShaveSave"),		(mPrefsPtr->ShaveSave ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("discardNegs"),		(mPrefsPtr->discardNegs ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("useRaeburn"),		(mPrefsPtr->useRaeburn ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("tidyUp"),		(mPrefsPtr->tidyUp ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("useVolume"),		(mPrefsPtr->useVolume ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("matchVF"),		(mPrefsPtr->matchVF ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("makeDCEnv"),		(mPrefsPtr->makeDCEnv ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("verbose"),		(mPrefsPtr->verbose ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("matchPDF"),		(mPrefsPtr->matchPDF ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	CFPreferencesSetAppValue (CFSTR("fixClaude"),		(mPrefsPtr->fixClaude ? kCFBooleanTrue : kCFBooleanFalse),		kCFPreferencesCurrentApplication);	// SInt16 Preferences	CFNumberRef	tempNmberRef;	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->numClassesMeanCSD)); 	CFPreferencesSetAppValue (CFSTR("numClassesMeanCSD"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->numClassesMaxCSD)); 	CFPreferencesSetAppValue (CFSTR("numClassesMaxCSD"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->numClassesLogCSD)); 	CFPreferencesSetAppValue (CFSTR("numClassesLogCSD"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->quadratNumPlacings)); 	CFPreferencesSetAppValue (CFSTR("quadratNumPlacings"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->quadratNumReps)); 	CFPreferencesSetAppValue (CFSTR("quadratNumReps"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->RPNumPlacings)); 	CFPreferencesSetAppValue (CFSTR("RPNumPlacings"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->RPNumReps)); 	CFPreferencesSetAppValue (CFSTR("RPNumReps"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->numNNDist)); 	CFPreferencesSetAppValue (CFSTR("numNNDist"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->numCFOffsetVolPts)); 	CFPreferencesSetAppValue (CFSTR("numCFOffsetVolPts"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->numQD3DEqPts)); 	CFPreferencesSetAppValue (CFSTR("numQD3DEqPts"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->numQD3DLngPts)); 	CFPreferencesSetAppValue (CFSTR("numQD3DLngPts"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->sampleShape)); 	CFPreferencesSetAppValue (CFSTR("sampleShape"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->numEnvelopeRuns)); 	CFPreferencesSetAppValue (CFSTR("numEnvelopeRuns"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->shaveXYZ)); 	CFPreferencesSetAppValue (CFSTR("shaveXYZ"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->direction)); 	CFPreferencesSetAppValue (CFSTR("direction"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->minPopulation)); 	CFPreferencesSetAppValue (CFSTR("minPopulation"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->confidence)); 	CFPreferencesSetAppValue (CFSTR("confidence"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt16Type, &(mPrefsPtr->overlap)); 	CFPreferencesSetAppValue (CFSTR("overlap"), tempNmberRef, kCFPreferencesCurrentApplication);	// SInt32 Preferences	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt32Type, &(mPrefsPtr->seed)); 	CFPreferencesSetAppValue (CFSTR("seed"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberSInt32Type, &(mPrefsPtr->MCReps)); 	CFPreferencesSetAppValue (CFSTR("MCReps"), tempNmberRef, kCFPreferencesCurrentApplication);		// float Preferences	tempNmberRef = CFNumberCreate(NULL, kCFNumberFloatType, &(mPrefsPtr->maxValueMeanCSD)); 	CFPreferencesSetAppValue (CFSTR("maxValueMeanCSD"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberFloatType, &(mPrefsPtr->deltaLRegCSD)); 	CFPreferencesSetAppValue (CFSTR("deltaLRegCSD"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberFloatType, &(mPrefsPtr->impingementMeanErr)); 	CFPreferencesSetAppValue (CFSTR("impingementMeanErr"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberFloatType, &(mPrefsPtr->impingementMaxErr)); 	CFPreferencesSetAppValue (CFSTR("impingementMaxErr"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberFloatType, &(mPrefsPtr->EpanecnikovCVal)); 	CFPreferencesSetAppValue (CFSTR("EpanecnikovCVal"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberFloatType, &(mPrefsPtr->crit1Factor)); 	CFPreferencesSetAppValue (CFSTR("crit1Factor"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberFloatType, &(mPrefsPtr->crit2Factor)); 	CFPreferencesSetAppValue (CFSTR("crit2Factor"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberFloatType, &(mPrefsPtr->shaveIncrement)); 	CFPreferencesSetAppValue (CFSTR("shaveIncrement"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberFloatType, &(mPrefsPtr->minPercent)); 	CFPreferencesSetAppValue (CFSTR("minPercent"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberFloatType, &(mPrefsPtr->maxAspectRatio)); 	CFPreferencesSetAppValue (CFSTR("maxAspectRatio"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberFloatType, &(mPrefsPtr->VFPercent)); 	CFPreferencesSetAppValue (CFSTR("VFPercent"), tempNmberRef, kCFPreferencesCurrentApplication);	tempNmberRef = CFNumberCreate(NULL, kCFNumberFloatType, &(mPrefsPtr->discard)); 	CFPreferencesSetAppValue (CFSTR("discard"), tempNmberRef, kCFPreferencesCurrentApplication);	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);}/*  Old version://----------------------------------// CReducePrefs constructor//	This will create a preference file named 'inName' with//	a creator of 'inCreator' in the Preferences folder, if there isn't one there already.//----------------------------------CReducePrefs::CReducePrefs()	: CPreference("\pReduce 3D Prefs"){	mPrefsPtr = new PrefStruct();	InitPreferences();}//----------------------------------// CReducePrefs default destructor//	delete file object and any other handles.//----------------------------------CReducePrefs::~CReducePrefs(void){	if (mPrefsPtr != nil)		delete mPrefsPtr;	mPrefsPtr = nil;}//----------------------------------// WritePrefsToStream//----------------------------------void CReducePrefs::WritePrefsToStream(LStream& inStream, PrefStruct *inPrefs){	inStream << inPrefs->includeMeanCSD;	inStream << inPrefs->includeMaxCSD;	inStream << inPrefs->includeLogCSD;	inStream << inPrefs->includeRegCSD;	inStream << inPrefs->doQuadrat;	inStream << inPrefs->doOthers;	inStream << inPrefs->doRandomPt;	inStream << inPrefs->make3DModel;	inStream << inPrefs->outputSigmas;	inStream << inPrefs->outputCrystals;	inStream << inPrefs->outputR3STT;	inStream << inPrefs->outputReduce;	inStream << inPrefs->doImpingement;	inStream << inPrefs->numClassesMeanCSD;	inStream << inPrefs->numClassesMaxCSD;	inStream << inPrefs->numClassesLogCSD;	inStream << inPrefs->quadratNumPlacings;	inStream << inPrefs->quadratNumReps;	inStream << inPrefs->RPNumPlacings;	inStream << inPrefs->RPNumReps;	inStream << inPrefs->maxValueMeanCSD;	inStream << inPrefs->deltaLRegCSD;	inStream << inPrefs->impingementMeanErr;	inStream << inPrefs->impingementMaxErr;	inStream << inPrefs->doLMcfPcf;	inStream << inPrefs->numNNDist;	inStream << inPrefs->numCFOffsetVolPts;	inStream << inPrefs->EpanecnikovCVal;	inStream << inPrefs->numQD3DEqPts;	inStream << inPrefs->numQD3DLngPts;	inStream << inPrefs->sampleShape;	inStream << inPrefs->numEnvelopeRuns;	inStream << inPrefs->inflateBBox;	inStream << inPrefs->observabilityFilter;	inStream << inPrefs->crit1Factor;	inStream << inPrefs->crit2Factor;	inStream << inPrefs->doShave;	inStream << inPrefs->keepAspectRatios;	inStream << inPrefs->shaveXYZ;	inStream << inPrefs->shaveIncrement;	inStream << inPrefs->direction;	inStream << inPrefs->minPercent;	inStream << inPrefs->minPopulation;	inStream << inPrefs->maxAspectRatio;	inStream << inPrefs->smaller;	inStream << inPrefs->ShaveSave;	inStream << inPrefs->discardNegs;	inStream << inPrefs->useRaeburn;	inStream << inPrefs->confidence;	inStream << inPrefs->seed;	inStream << inPrefs->MCReps;	inStream << inPrefs->tidyUp;	inStream << inPrefs->useVolume;	inStream << inPrefs->overlap;	inStream << inPrefs->matchVF;	inStream << inPrefs->makeDCEnv;	inStream << inPrefs->verbose;	inStream << inPrefs->VFPercent;	inStream << inPrefs->matchPDF;	inStream << inPrefs->discard;	inStream << inPrefs->fixClaude;}//----------------------------------// ReadPrefsFromStream//----------------------------------void CReducePrefs::ReadPrefsFromStream(LStream& inStream, PrefStruct *inPrefs){	inStream >> inPrefs->includeMeanCSD;	inStream >> inPrefs->includeMaxCSD;	inStream >> inPrefs->includeLogCSD;	inStream >> inPrefs->includeRegCSD;	inStream >> inPrefs->doQuadrat;	inStream >> inPrefs->doOthers;	inStream >> inPrefs->doRandomPt;	inStream >> inPrefs->make3DModel;	inStream >> inPrefs->outputSigmas;	inStream >> inPrefs->outputCrystals;	inStream >> inPrefs->outputR3STT;	inStream >> inPrefs->outputReduce;	inStream >> inPrefs->doImpingement;	inStream >> inPrefs->numClassesMeanCSD;	inStream >> inPrefs->numClassesMaxCSD;	inStream >> inPrefs->numClassesLogCSD;	inStream >> inPrefs->quadratNumPlacings;	inStream >> inPrefs->quadratNumReps;	inStream >> inPrefs->RPNumPlacings;	inStream >> inPrefs->RPNumReps;	inStream >> inPrefs->maxValueMeanCSD;	inStream >> inPrefs->deltaLRegCSD;	inStream >> inPrefs->impingementMeanErr;	inStream >> inPrefs->impingementMaxErr;	inStream >> inPrefs->doLMcfPcf;	inStream >> inPrefs->numNNDist;	inStream >> inPrefs->numCFOffsetVolPts;	inStream >> inPrefs->EpanecnikovCVal;	inStream >> inPrefs->numQD3DEqPts;	inStream >> inPrefs->numQD3DLngPts;	inStream >> inPrefs->sampleShape;	inStream >> inPrefs->numEnvelopeRuns;	inStream >> inPrefs->inflateBBox;	inStream >> inPrefs->observabilityFilter;	inStream >> inPrefs->crit1Factor;	inStream >> inPrefs->crit2Factor;	inStream >> inPrefs->doShave;	inStream >> inPrefs->keepAspectRatios;	inStream >> inPrefs->shaveXYZ;	inStream >> inPrefs->shaveIncrement;	inStream >> inPrefs->direction;	inStream >> inPrefs->minPercent;	inStream >> inPrefs->minPopulation;	inStream >> inPrefs->maxAspectRatio;	inStream >> inPrefs->smaller;	inStream >> inPrefs->ShaveSave;	inStream >> inPrefs->discardNegs;	inStream >> inPrefs->useRaeburn;	inStream >> inPrefs->confidence;	inStream >> inPrefs->seed;	inStream >> inPrefs->MCReps;	inStream >> inPrefs->tidyUp;	inStream >> inPrefs->useVolume;	inStream >> inPrefs->overlap;	inStream >> inPrefs->matchVF;	inStream >> inPrefs->makeDCEnv;	inStream >> inPrefs->verbose;	inStream >> inPrefs->VFPercent;	inStream >> inPrefs->matchPDF;	inStream >> inPrefs->discard;	inStream >> inPrefs->fixClaude;}*/