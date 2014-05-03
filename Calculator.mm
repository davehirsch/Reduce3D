// ===========================================================================
//	Calculator.mm
//  Reduce3D
//
//  Created by David Hirsch on 12/1/07.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================


#import "Calculator.h"
#import "Mediator.h"
#import "AppController.h"
#import "ReduceConstants.h"
#import "stringFile.h"
#import "BoundingBox.h"
#import "Stats.h"
#import "HoleSet.h"
#import "CrystalArray.h"
#import "Bounds.h"
#import "CFloatArray.h"
#import "EnvelopeData.h"
#import <Carbon/Carbon.h>

Calculator::Calculator(void *inMed, void *inCont, PrefStruct *inPrefs)
{
	mediator = inMed;
	controller = inCont;
	prefs = inPrefs;
//	mSimulation = nil;
	mLogFile = nil;
}

#pragma mark Functions for executing the computation
// This method gets called by Mediator to do the work
bool
Calculator::runAnalysis(std::vector<std::string> inputFiles, std::string holesFile)
{
	bool continueToProcess = true;
	
	for (short i = 0; continueToProcess && (i <= inputFiles.size()-1) ; i++) {
		std::string thisInputPath = inputFiles[i];
		try {
			if (prefs->doShave) {
				continueToProcess = shaveAndReduceData(thisInputPath);
			} else {
				continueToProcess = reduceDataInFile(thisInputPath, holesFile);
			}
		} catch (const char *msgString) {
			std::string logStr = "Calculator::runAnalysis Caught a thrown error: ";
			logStr += msgString;
			postError(msgString, "Reduce 3D Error", nil, -1, -1);
		} catch (...) {
            continueToProcess = false;
        }
	}
	if (!continueToProcess) {
		// then we bugged out due to an error / exception
		return false;
	} else {
		return true;
	}
}


// ---------------------------------------------------------------------------------
//		• reduceDataInFile
// ---------------------------------------------------------------------------------
bool
Calculator::reduceDataInFile(std::string inputFilePath, std::string holesFilePath)
{	
	stringFile *holesFile = nil;
	stringFile *curInputFile = nil;
	HoleSet * holes = nil;
	BoundingBox *curBBox = nil;
	
	// cast the stored AppController pointer into an Obj-C object for making Obj-C calls
	AppController *cont = (AppController *)controller;

	// Initialize Random Functions
	srand48(prefs->seed);

	try {
		// Set up the log file for verbose logging:
		NSString *filename = [[NSString stringWithUTF8String:inputFilePath.c_str()] lastPathComponent];
		if (prefs->verbose) {
			filename = [filename stringByAppendingString:@"_Log.txt"];
			NSString *curDesktop = @"~/Desktop";
			curDesktop = [curDesktop stringByExpandingTildeInPath];
			NSString *logPath = [curDesktop stringByAppendingPathComponent:filename];
			mLogFile = new stringFile(true, (CFStringRef) logPath, true);
		}
		
		std::string message = "Reading data file: ";
		message += inputFilePath.c_str();
		setupProgress(message.c_str(), nil, nil, nil, -1, -1, -1, -1, true);
		message += "\n";
		log(message.c_str());

		// Create the input file object.  This will read the file's data into a buffer
		curInputFile = new stringFile(false, inputFilePath.c_str());
		if ([cont shouldStopCalculating]) throw(kUserCanceledErr);

		// Create the bounding box, which also creates the crystal array from the file data
		curBBox = new BoundingBox(this, curInputFile);
		if ([cont shouldStopCalculating]) throw(kUserCanceledErr);
		
		// If a holes file was selected, then load that data in now	
		if (holesFilePath != "") {
			holesFile = new stringFile(false, holesFilePath.c_str());
			if (holes != nil)
				delete holes;
			holes = new HoleSet(this, 100, holesFile);
		} else {
			holes = nil;	// this is redundant, but included here for clarity
		}
		if ([cont shouldStopCalculating]) throw(kUserCanceledErr);

		reduceOneDataset(curBBox, holes);
		if ([cont shouldStopCalculating]) throw(kUserCanceledErr);

		if (holesFile != nil) delete holesFile;
		if (curBBox != nil) delete curBBox;
		if (curInputFile != nil) delete curInputFile;
		if (mLogFile != nil) {
			delete mLogFile;
			mLogFile = nil;
		}
		return true;
	} catch (int err) {
		if (err == kUserCanceledErr) {
			// then clean up quietly
		}
		if (holesFile != nil) delete holesFile;
		if (curBBox != nil) delete curBBox;
		if (curInputFile != nil)delete curInputFile;
		if (mLogFile != nil) {
			delete mLogFile;
			mLogFile = nil;
		}
        if (err == kProcessingError) {
            // then there was an ugly error, and the state is fucked up.  Try to quit.
            return kProcessingError;
        }
		return false;
	} catch (const char *msgString) {
		std::string logStr = "Calculator::reduceDataInFile Caught a thrown error: ";
		logStr += msgString;
		logStr += "\n";
		log(msgString);
		postError(msgString, "Reduce 3D Error", nil, -1, -1);

		// clean up
		if (holesFile != nil) delete holesFile;
		if (curBBox != nil) delete curBBox;
		if (curInputFile != nil)delete curInputFile;
		if (mLogFile != nil) {
			delete mLogFile;
			mLogFile = nil;
		}
		return false;
	}
}

// ---------------------------------------------------------------------------------
//		• ShaveAndReduceData
// ---------------------------------------------------------------------------------
bool 
Calculator::shaveAndReduceData(std::string inputFilePath)
{
	
	// cast the stored AppController pointer into an Obj-C object for making Obj-C calls
	AppController *cont = (AppController *)controller;
	
	// Initialize Random Functions
	srand48(prefs->seed);
	
	BoundingBox *curBBox = nil;
	BoundingBox *startBBox = nil;
	stringFile *curInputFile = nil;

	try {
		if (prefs->verbose) {
			NSString *filename = [[NSString stringWithUTF8String:inputFilePath.c_str()] lastPathComponent];
			filename = [filename stringByAppendingString:@"_log"];
			NSString *curDesktop = @"~/Desktop";
			curDesktop = [curDesktop stringByExpandingTildeInPath];
			NSString *logPath = [curDesktop stringByAppendingPathComponent:filename];
			mLogFile = new stringFile(true, (CFStringRef) logPath, true);
		}
		curInputFile = new stringFile(false, inputFilePath.c_str());
		startBBox = new BoundingBox(this, curInputFile);
		if ([cont shouldStopCalculating]) throw(kUserCanceledErr);
		CrystalArray *theXls = startBBox->GetXls();
		if (theXls->GetBounds() == kBoundsNone) {
			delete startBBox;
			startBBox = nil;
			postError("To do shave analysis, you must begin with a file that has explicitly stated bounds for the data.", 
					  "Need Bounds", nil, -1, -1);
				throw(kUserCanceledErr);
		} else {
			curBBox = new BoundingBox(*startBBox);
			short shaveIteration=0;
			char shaveStr[kStdStringSize];
			stringFile *inputFile = theXls->GetFile();
			std::string oldName = inputFile->getNameWithoutExtension();
			short maxRootLength = 255 - 16;	// 255 is the maximum filename length for OSX (10.4), and 13 is the longest suffix
			if (oldName.length() > maxRootLength) {
				// then quietly truncate it
				short extralength = oldName.length() - maxRootLength;
				oldName.resize(oldName.length() - extralength);
			}
					
			while (shaveData(startBBox, curBBox, shaveIteration)) {
				sprintf(shaveStr, "Shave number %d", shaveIteration);
				setShaveMessage(shaveStr);
				if (prefs->ShaveSave) {
					curBBox->saveShavedIntegrateFile(shaveIteration);
				} else {
					oldName.append("_");
					char tempNumStr[10];
					sprintf(tempNumStr, "%d", shaveIteration);
					oldName+=tempNumStr;
					oldName.append(".Int");
					inputFile->setName((char *)oldName.c_str());
					reduceOneDataset(curBBox, nil);
					if ([cont shouldStopCalculating]) throw(kUserCanceledErr);
				}
				shaveIteration++;
			}
			if (curBBox != nil) delete curBBox;
		}
		if (startBBox != nil) delete startBBox;
		if (curInputFile != nil) delete curInputFile;
		return true;
	} catch (int err) {
		// This is likely a kUserCanceledErr, so clean up quietly
		if (curBBox != nil) delete curBBox;
		if (startBBox != nil) delete startBBox;
		if (curInputFile != nil)delete curInputFile;
		if (mLogFile != nil) {
			delete mLogFile;
			mLogFile = nil;
		}
		return false;
	} catch (const char *msgString) {
		std::string logStr = "Calculator::shaveAndReduceData Caught a thrown error: ";
		logStr += msgString;
		logStr += "\n";
		log(msgString);
		postError(msgString, "Reduce 3D Error", nil, -1, -1);
		
		// clean up
		if (curBBox != nil) delete curBBox;
		if (startBBox != nil) delete startBBox;
		if (curInputFile != nil)delete curInputFile;
		if (mLogFile != nil) {
			delete mLogFile;
			mLogFile = nil;
		}
		return false;
	}
}

// ---------------------------------------------------------------------------------
//		• reduceOneDataset
// ---------------------------------------------------------------------------------
/* This method does the work: fitting the bounding box, measuring the statistics, and
writing the results to output files. */
void	
Calculator::reduceOneDataset(BoundingBox *ioBBox, HoleSet *inHoles)
{
	Stats *stats = new Stats(true);
	
	// cast the stored AppController pointer into an Obj-C object for making Obj-C calls
	AppController *cont = (AppController *)controller;
	
	try {
		if (prefs->observabilityFilter) {
			if (prefs->observabilityMethod == kSetFromData) {
				(ioBBox->GetXls())->FindObservabilityValues(prefs->observabilityPercent, &(stats->observabilityCrit1value), &(stats->observabilityCrit2value));
			} else {
				stats->observabilityCrit1value = prefs->crit1Factor;
				stats->observabilityCrit2value = prefs->crit2Factor;
			}
			if (prefs->applyObservabilityFilter) {
				(ioBBox->GetXls())->FilterForObservability(prefs->crit1Factor, prefs->crit2Factor, &(stats->observabilityCrit1rejects), &(stats->observabilityCrit2rejects)); 
			}
		}
		
		/* Have the bounding box find the convex hull; it will also find 
		the appropriate primitive as part of this process (if Sides wasn't 
		selected by the user), and adapt the crystal data set to that primitive */
		if ((prefs->sampleShape == kSides) || (!prefs->exscribedPrimitive))
			ioBBox->FindConvexHull();

		if ([cont shouldStopCalculating]) throw(kUserCanceledErr);

		FindBestPrimitive(ioBBox, inHoles);
		AdaptToPrimitiveBox(stats, ioBBox);
		
		calcStats(stats, ioBBox, inHoles);	// This funtion no longer includes DoEnvelopeSimulations()
		if ([cont shouldStopCalculating]) throw(kUserCanceledErr);

		if (prefs->numEnvelopeRuns > 0)
			DoEnvelopeSimulations(ioBBox, stats, inHoles);

		saveResults(stats, ioBBox, inHoles, -1);
		
		if (stats != nil) {
			delete stats;
			stats = nil;
		}
	} catch (int err) {
		if (stats != nil) {
			delete stats;
			stats = nil;
		}
		throw;	// rethrow
	} catch (CalcError err) {
		if (stats != nil) {
			delete stats;
			stats = nil;
		}
		throw kProcessingError;
	}
}


// ---------------------------------------------------------------------------------
//		• ShaveData
// ---------------------------------------------------------------------------------
/*	This routine takes the ioBBox (which began as a duplicate of the mStartBBox),
adjusts its bounds, and removes all crystals that fall outside the bounds.
It then checks the boundary conditions to see if the box is still legal. */
bool
Calculator::shaveData(BoundingBox *mStartBBox, BoundingBox *ioBBox, short iteration)
{
	CrystalArray *ioXls = ioBBox->GetXls();
	short primType = ioBBox->GetType();	
	double	incr = (prefs->shaveIncrement / 100.0);
	// should only get here if the primitive is Cylinder or RP, and explicit bounds
	//	were given in the input file.  The SideSet primitive should be the same type of bounds
	//	as given in the input file.
	if ((primType == kCubeBox) || (primType == kRPBox)) {
		Point3DFloat lower, upper, stLower, stUpper, stDim;
		lower = ioXls->LowerBound();
		upper = ioXls->UpperBound();
		stLower = (mStartBBox->GetXls())->LowerBound();
		stUpper = (mStartBBox->GetXls())->UpperBound();
		stDim = stUpper - stLower;
		if (prefs->keepAspectRatios) {
			switch (prefs->direction) {
				case kFromMax:
					upper = stUpper - (incr * iteration * stDim);
					break;
				case kSymmetric:
					upper = stUpper - (incr * 0.5 * iteration * stDim);
					lower = stLower + (incr * 0.5 * iteration * stDim);
					break;
				case kFromMin:
					lower = stLower + (incr * iteration * stDim);
					break;
			}
		} else {
			switch (prefs->direction) {
				case kFromMax:
					switch (prefs->shaveXYZ) {
						case kX:
							upper.x = stUpper.x - (incr * iteration * stDim.x);
							break;
						case kY:
							upper.y = stUpper.y - (incr * iteration * stDim.y);
							break;
						case kZ:
							upper.z = stUpper.z - (incr * iteration * stDim.z);
							break;
					}
					break;
				case kSymmetric:
					switch (prefs->shaveXYZ) {
						case kX:
							upper.x = stUpper.x - (incr * 0.5 * iteration * stDim.x);
							lower.x = stLower.x + (incr * 0.5 * iteration * stDim.x);
							break;
						case kY:
							upper.y = stUpper.y - (incr * 0.5 * iteration * stDim.y);
							lower.y = stLower.y + (incr * 0.5 * iteration * stDim.y);
							break;
						case kZ:
							upper.z = stUpper.z - (incr * 0.5 * iteration * stDim.z);
							lower.z = stLower.z + (incr * 0.5 * iteration * stDim.z);
							break;
					}
					break;
				case kFromMin:
					switch (prefs->shaveXYZ) {
						case kX:
							lower.x = stLower.x + (incr * iteration * stDim.x);
							break;
						case kY:
							lower.y = stLower.y + (incr * iteration * stDim.y);
							break;
						case kZ:
							lower.z = stLower.z + (incr * iteration * stDim.z);
							break;
					}
					break;
			}
		}
		ioXls->SetLower(lower);
		ioXls->SetUpper(upper);
		ioBBox->AdjustToBounds();
	} else { 	// then the primitive must be a Cylinder
		Point3DFloat ctr;
		Point3DFloat deltaH;
		Point3DFloat stCtr = (mStartBBox->GetXls())->GetCtr();
		double	stRad = (mStartBBox->GetXls())->GetRadius();
		double	stHt = (mStartBBox->GetXls())->GetHeight();
		double	rad, ht;
		if (prefs->keepAspectRatios) {
			rad = stRad - (incr * iteration * stRad);
			ht = stHt - (incr * iteration * stHt);
			switch (prefs->direction) {
				case kFromMax:	// places new cylinder within start cylinder but attached to base
					deltaH = Point3DFloat(0, 0, 0.5*(ht-stHt));
					ctr = stCtr + deltaH;
					break;
				case kSymmetric:	// places new cylinder centered within start cylinder
					ctr = stCtr;				// nothing else needed
					break;
				case kFromMin:	// places new cylinder within start cylinder but attached to top
					deltaH = Point3DFloat(0, 0, 0.5*(stHt - ht));
					ctr = stCtr + deltaH;
					break;
			}
		} else {
			Point3DFloat delta;
			switch (prefs->direction) {
				case kFromMax:
					switch (prefs->shaveXYZ) {
						case kX:
							rad = stRad - (incr * iteration * stRad);
							delta.x = stCtr.x - stRad + rad;
							break;
						case kY:
							rad = stRad - (incr * iteration * stRad);
							delta.y = stCtr.y - stRad + rad;
							break;
						case kZ:
							ht = stHt - (incr * iteration * stHt);
							delta.z = stCtr.z + 0.5 * (ht - stHt);
							break;
					}
					break;
				case kSymmetric:
					switch (prefs->shaveXYZ) {
						case kX:
							rad = stRad - (incr * iteration * stRad);
							break;
						case kY:
							rad = stRad - (incr * iteration * stRad);
							break;
						case kZ:
							ht = stHt - (incr * iteration * stHt);
							break;
					}
					break;
				case kFromMin:
					switch (prefs->shaveXYZ) {
						case kX:
							rad = stRad - (incr * iteration * stRad);
							delta.x = stCtr.x + stRad - rad;
							break;
						case kY:
							rad = stRad - (incr * iteration * stRad);
							delta.y = stCtr.y + stRad - rad;
							break;
						case kZ:
							ht = stHt - (incr * iteration * stHt);
							delta.z = stCtr.z + 0.5 * (stHt - ht);
							break;
					}
					break;
					ctr = stCtr + delta;
			}
		}
		ioXls->SetRadius(rad);
		ioXls->SetHeight(ht);
		ioXls->SetCtr(ctr);
		ioBBox->AdjustToBounds();
	}
	// now check to see if the box is still legal
	bool goodBox;
	if (prefs->shaveSmLg == kSmaller)	// smaller means ALL must be illegal for goodBox to be false
		goodBox = (((100 * (1 - iteration * incr) > prefs->minPercent))
				   || (ioXls->GetNumXls() > prefs->minPopulation)
				   || (prefs->keepAspectRatios ? false : (ioBBox->AspectRatio() < prefs->maxAspectRatio)));
	else	// if any are illegal then goodBox is false
		goodBox = !(((100 * (1 - iteration * incr) < prefs->minPercent))
					|| (ioXls->GetNumXls() < prefs->minPopulation)
					|| (prefs->keepAspectRatios ? false : (ioBBox->AspectRatio() > prefs->maxAspectRatio)));
	return goodBox;
}


#pragma mark Utility functions for communicating with controller
bool
Calculator::shouldStopCalculating()
{
	// cast the stored AppController pointer into an Obj-C object for making Obj-C calls
	AppController *cont = (AppController *)controller;
	
	return [cont shouldStopCalculating]; 
}

void
Calculator::setProgMessage(const char *inMessage)
{
	// cast the stored AppController pointer into an Obj-C object for making Obj-C calls
	AppController *cont = (AppController *)controller;
	
	[cont setProgMessage:(inMessage ? [NSString stringWithUTF8String:inMessage] : nil)]; 
}


void
Calculator::setProgTitle(const char *inStr)
{
	// cast the stored AppController pointer into an Obj-C object for making Obj-C calls
	AppController *cont = (AppController *)controller;
	
	[cont setProgTitle:(inStr ? [NSString stringWithUTF8String:inStr] : nil)]; 
}


void
Calculator::setEnvMessage(const char *inMessage)
{
	// cast the stored AppController pointer into an Obj-C object for making Obj-C calls
	AppController *cont = (AppController *)controller;
	
	[cont setEnvMessage:(inMessage ? [NSString stringWithUTF8String:inMessage] : nil)]; 
}

void
Calculator::setShaveMessage(const char *inMessage)
{
	// cast the stored AppController pointer into an Obj-C object for making Obj-C calls
	AppController *cont = (AppController *)controller;
	
	[cont setShaveMessage:(inMessage ? [NSString stringWithUTF8String:inMessage] : nil)]; 
}

void
Calculator::setupProgress(const char *inMainMessage,
						  const char *inShaveMessage, 
						  const char *inEnvMessage, 
						  const char *inTitle, 
						  short inInc, 
						  double inMin, 
						  double inMax, 
						  double inCur, 
						  bool inInd)
{
	// cast the stored AppController pointer into an Obj-C object for making Obj-C calls
	AppController *cont = (AppController *)controller;
	
	[cont setupProgress:(inMainMessage ? [NSString stringWithUTF8String:inMainMessage] : nil)
		   shaveMessage:(inShaveMessage ? [NSString stringWithUTF8String:inShaveMessage] : nil)
			 envMessage:(inEnvMessage ? [NSString stringWithUTF8String:inEnvMessage] : nil)
				  title:(inTitle ? [NSString stringWithUTF8String:inTitle] : nil)
			 increments:inInc
					min:inMin
					max:inMax
					cur:inCur
		  indeterminate:inInd]; 
	progMin = inMin;
	progMax = inMax;
	lastProgCall = inMin;
	progInc = (inMax - inMin) / kNumberOfProgressCalls;
}

void
Calculator::progress(double inCur)
{
	if (inCur > lastProgCall + progInc) {
		// cast the stored AppController pointer into an Obj-C object for making Obj-C calls
		AppController *cont = (AppController *)controller;
		[cont progress: inCur];
		lastProgCall = inCur;
	}
}

void
Calculator::log(const char *inMessage)
{
	if (prefs->verbose)
		mLogFile->putOneLine(inMessage);
}

void
Calculator::log(std::string inMessage)
{
	if (prefs->verbose)
		mLogFile->putOneLine(inMessage.c_str());
}

// ---------------------------------------------------------------------------------
//		• postError
// ---------------------------------------------------------------------------------
/* Note that inButtonStr is a string representing the button title in order which should
	appear in the Alert.  The titles are delimited by the "|" character. */
short
Calculator::postError(const char *inMessage, 
					  const char *inTitle, 
					  const char *inButtonStr, 
					  short inDefault,
					  double inDismiss)
{
	// cast the stored AppController pointer into an Obj-C object for making Obj-C calls
	AppController *cont = (AppController *)controller;
	NSArray * buttonTitles = nil;
	NSString *msg = nil;
	NSString *ttl = nil;
	
	if (inButtonStr != nil) {
		buttonTitles = [[NSString stringWithUTF8String:inButtonStr] componentsSeparatedByString:@"|"];
	}
	if (inMessage != nil) {
		msg = [NSString stringWithUTF8String:inMessage];
	}
	if (inTitle != nil) {
		ttl = [NSString stringWithUTF8String:inTitle];
	}
	short response = [cont postError:msg
							   title:ttl
							 buttons:buttonTitles
					   defaultButton:inDefault
						   dismissIn:inDismiss];
	if (response == NSAlertFirstButtonReturn)  return 0;
	if (response == NSAlertSecondButtonReturn)  return 1;
	if (response == NSAlertThirdButtonReturn)  return 2;
	return (response - NSAlertThirdButtonReturn + 2);
}

PrefStruct *	
Calculator::getPrefs()
{
	return prefs;
}

#pragma mark Computation methods

// ---------------------------------------------------------------------------------
//		• CalcStats
// ---------------------------------------------------------------------------------
void			
Calculator::calcStats(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	// cast the stored AppController pointer into an Obj-C object for making Obj-C calls
	AppController *cont = (AppController *)controller;

	setProgTitle("Computing Statistics...");
	
	CrystalArray * theXls = inBBox->GetXls();
	
	if (prefs->applyObservabilityFilter && inHoles) {
		inHoles->TidyCrystalsUp(theXls);
	}

	DoInitialStats(stats, inBBox, inHoles);
	if ([cont shouldStopCalculating]) throw(kUserCanceledErr);
	
	ComputeVolumes(stats, inBBox, inHoles); 
	if ([cont shouldStopCalculating]) throw(kUserCanceledErr);
	
	if (prefs->doImpingement && !theXls->inputHasExtendedVolume()) {
		theXls->CorrectForImpingement();
	}
	if ([cont shouldStopCalculating]) throw(kUserCanceledErr);

	ComputeRadiusStats(stats, inBBox, inHoles);
	if ([cont shouldStopCalculating]) throw(kUserCanceledErr);

	if (prefs->doOthers || prefs->includeRegCSD || prefs->doLMcfPcf) {
		ComputeNeighbors(stats, inBBox, inHoles);
		if ([cont shouldStopCalculating]) throw(kUserCanceledErr);
		ComputeNNStats(stats, inBBox, inHoles);
		if ([cont shouldStopCalculating]) throw(kUserCanceledErr);
	}


	if (prefs->doOthers) {
		ComputeAvramiTest(stats, inBBox, inHoles);
		if ([cont shouldStopCalculating]) throw(kUserCanceledErr);
		ComputeCSDStats(stats, inBBox, inHoles);
		if ([cont shouldStopCalculating]) throw(kUserCanceledErr);
	}
	
	if (prefs->doQuadrat) {
		try {
			ComputeQuadrat(stats, inBBox, inHoles);
			if ([cont shouldStopCalculating]) throw(kUserCanceledErr);
		} catch (Calculator::QuadratFailed) {
			;	// no cleanup required
		}
	}
	
	if (prefs->doRandomPt)
		ComputeRandomPt(stats, inBBox, inHoles);
	if ([cont shouldStopCalculating]) throw(kUserCanceledErr);


	if (prefs->doLMcfPcf) {
		if (stats->forRealDataset) {	// for the envelope simulations, this is set in DoEnvelopeSimulations based
							// on the real data params (intensity, mean NN dist, etc.)
			double scaleIncrement, bandwidth;
			if (prefs->specifyTestDistance) {
				scaleIncrement = prefs->testDistanceInterval;
				bandwidth = scaleIncrement / (1.0 - prefs->overlap * 0.01);
			} else {
				// figure out the bandwidth - twice the calculated value is the whole width
				bandwidth = 2.0 * prefs->EpanecnikovCVal / CubeRoot(stats->intensity);
				scaleIncrement = bandwidth * (1.0 - prefs->overlap * 0.01);
			}

			double totScale = prefs->numNNDist * stats->meanSep;
			short numScales = trunc(totScale / scaleIncrement) + 1;

			stats->SetCFParams(numScales, prefs->outputSigmas);

			// set up HDistance Array
			for (short i = 1; i <= numScales; i++) {
				stats->hDistances[i] = scaleIncrement * i;
			}
		}
		
		ComputeCFs_Both(stats, inBBox, inHoles);
	}

}


#pragma mark ==================== Primitive Stuff
// ---------------------------------------------------------------------------
//		• FindBestPrimitive
// ---------------------------------------------------------------------------
/*	Since the processing for the correlation functions is *extremely* slow when
	their bounding boxes are made of sides, here we figure out the best geometrical
	primitive for the bounding box, guided by the option chosen in the settings
	window.
	Algorithm:
		1) Make a Primitive that is outside the box, based on the min & max points
			and the center that we've already calculated.
		2) Reduce the dimensions by small increments until the primitive is
			inside the box.
		3) Move the primitive in each of the 6 directions, to find the range of movement
			of the primitive inside the box.  Put the primitive in the middle of this range.
		4) Expand the primitive until it's just outside the box.
		5) Redo 2-4 a couple of times.
	* Unless we are doing the exscribed primitive instead *
*/
void
Calculator::FindBestPrimitive(BoundingBox *ioBBox, HoleSet *inHoles)
{
	Point3DFloat ctr, beforeCtr;
	SideSet curPrim;
	short tryNum;
	short numtries = 6;
	Point3DFloat curLengths;
	
	switch (prefs->sampleShape) {
		case kRectPrism:
			if (prefs->exscribedPrimitive) {
				FindBestExscRPPrimitive(ioBBox);
			} else {
				NewFindBestRPPrimitive(curPrim, ioBBox, inHoles);	// start with a Monte Carlo search for best box

				curLengths = curPrim.GetSideLenPt();
				
				do {		// expand box until just too big
					curLengths *= 1.01;
					curPrim.SetDimensions(curLengths);
				} while (ioBBox->PrimitiveInBox(&curPrim));

				for (tryNum = 1; tryNum <= numtries; tryNum++) {
						// set shrinking factors; get finer-grained with later tries
					double bigFactor = 1.0 - ((numtries - tryNum + 1) * 0.015);
					double smFactor = 1.0 - ((numtries - tryNum + 1) * 0.03);
					// Shrink until primitive is inside box: try for each dimension, pick version with largest volume
					// Try Shrinking X:
					Point3DFloat beforeLengths = curPrim.GetSideLenPt();
					beforeCtr = curPrim.GetCtr();

		//	OLD Version: keeps ratios among dimensions
					while (!ioBBox->PrimitiveInBox(&curPrim)) {	// as long as the RP isn't inside the box,
															// we shrink it's dimensions by 99%
						curLengths *= bigFactor;
						if (curLengths.x < (beforeLengths.x / 1000)) {
							postError("I couldn't make an inscribed box after many tries.  You should inspect the crystal array for an outlier, which is the typical cause of this error.", "Bad Shape", nil, -1, -1);
							throw CalcError();
						}
						curPrim.SetDimensions(curLengths);
					}
					PutPrimInMiddle(curPrim, ioBBox, tryNum);
					do {
						curLengths *= 1.01;
						curPrim.SetDimensions(curLengths);
					} while (ioBBox->PrimitiveInBox(&curPrim));
						// now the primitive is outside the box again, but not by much

					double oldVersionVolume = curPrim.Volume();
					Point3DFloat oldVersionLen = curLengths;
					Point3DFloat oldVersionCtr = curPrim.GetCtr();

		//  Shrink x faster:
					curLengths = beforeLengths;
					curPrim.SetDimensions(curLengths);
					curPrim.SetCtr(beforeCtr);
					while (!ioBBox->PrimitiveInBox(&curPrim)) {	// as long as the RP isn't inside the box,
															// we shrink it's dimensions by 99%
						curLengths.x *= smFactor;
						curLengths.y *= bigFactor;
						curLengths.z *= bigFactor;
						if (curLengths.y < (beforeLengths.y / 1000)) {
							postError("I couldn't make an inscribed box after many tries.  You should inspect the crystal array for an outlier, which is the typical cause of this error.", "Bad Shape", nil, -1, -1);
							throw CalcError();
						}
						curPrim.SetDimensions(curLengths);
					}
					PutPrimInMiddle(curPrim, ioBBox, tryNum);
					do {
						curLengths *= 1.01;
						curPrim.SetDimensions(curLengths);
					} while (ioBBox->PrimitiveInBox(&curPrim));
						// now the primitive is outside the box again, but not by much

					double yzVersionVolume = curPrim.Volume();
					Point3DFloat yzVersionLen = curLengths;
					Point3DFloat yzVersionCtr = curPrim.GetCtr();

		//  Shrink y faster:
					curLengths = beforeLengths;
					curPrim.SetDimensions(curLengths);
					curPrim.SetCtr(beforeCtr);
					while (!ioBBox->PrimitiveInBox(&curPrim)) {	// as long as the RP isn't inside the box,
															// we shrink it's dimensions by 99%
						curLengths.x *= bigFactor;
						curLengths.y *= smFactor;
						curLengths.z *= bigFactor;
						if (curLengths.z < (beforeLengths.z / 1000)) {
							postError("I couldn't make an inscribed box after many tries.  You should inspect the crystal array for an outlier, which is the typical cause of this error.", "Bad Shape", nil, -1, -1);
							throw CalcError();
						}
						curPrim.SetDimensions(curLengths);
					}
					PutPrimInMiddle(curPrim, ioBBox, tryNum);
					do {
						curLengths *= 1.01;
						curPrim.SetDimensions(curLengths);
					} while (ioBBox->PrimitiveInBox(&curPrim));
						// now the primitive is outside the box again, but not by much

					double xzVersionVolume = curPrim.Volume();
					Point3DFloat xzVersionLen = curLengths;
					Point3DFloat xzVersionCtr = curPrim.GetCtr();

		//  Shrink z faster:
					curLengths = beforeLengths;
					curPrim.SetDimensions(curLengths);
					curPrim.SetCtr(beforeCtr);
					while (!ioBBox->PrimitiveInBox(&curPrim)) {	// as long as the RP isn't inside the box,
															// we shrink it's dimensions by 99%
						curLengths.x *= bigFactor;
						curLengths.y *= bigFactor;
						curLengths.z *= smFactor;
						if (curLengths.x < (beforeLengths.x / 1000)) {
							postError("I couldn't make an inscribed box after many tries.  You should inspect the crystal array for an outlier, which is the typical cause of this error.", "Bad Shape", nil, -1, -1);
							throw CalcError();
						}
						curPrim.SetDimensions(curLengths);
					}
					PutPrimInMiddle(curPrim, ioBBox, tryNum);
					do {
						curLengths *= 1.01;
						curPrim.SetDimensions(curLengths);
					} while (ioBBox->PrimitiveInBox(&curPrim));
						// now the primitive is outside the box again, but not by much

					double xyVersionVolume = curPrim.Volume();
					Point3DFloat xyVersionLen = curLengths;
					Point3DFloat xyVersionCtr = curPrim.GetCtr();

		//  Shrink x slower:
					curLengths = beforeLengths;
					curPrim.SetDimensions(curLengths);
					curPrim.SetCtr(beforeCtr);
					while (!ioBBox->PrimitiveInBox(&curPrim)) {	// as long as the RP isn't inside the box,
															// we shrink it's dimensions by 99%
						curLengths.x *= bigFactor;
						curLengths.y *= smFactor;
						curLengths.z *= smFactor;
						if (curLengths.x < (beforeLengths.x / 1000)) {
							postError("I couldn't make an inscribed box after many tries.  You should inspect the crystal array for an outlier, which is the typical cause of this error.", "Bad Shape", nil, -1, -1);
							throw CalcError();
						}
						curPrim.SetDimensions(curLengths);
					}
					PutPrimInMiddle(curPrim, ioBBox, tryNum);
					do {
						curLengths *= 1.01;
						curPrim.SetDimensions(curLengths);
					} while (ioBBox->PrimitiveInBox(&curPrim));
						// now the primitive is outside the box again, but not by much

					double xVersionVolume = curPrim.Volume();
					Point3DFloat xVersionLen = curLengths;
					Point3DFloat xVersionCtr = curPrim.GetCtr();

		//  Shrink y slower:
					curLengths = beforeLengths;
					curPrim.SetDimensions(curLengths);
					curPrim.SetCtr(beforeCtr);
					while (!ioBBox->PrimitiveInBox(&curPrim)) {	// as long as the RP isn't inside the box,
															// we shrink it's dimensions by 99%
						curLengths.x *= smFactor;
						curLengths.y *= bigFactor;
						curLengths.z *= smFactor;
						if (curLengths.y < (beforeLengths.y / 1000)) {
							postError("I couldn't make an inscribed box after many tries.  You should inspect the crystal array for an outlier, which is the typical cause of this error.", "Bad Shape", nil, -1, -1);
							throw CalcError();
						}
						curPrim.SetDimensions(curLengths);
					}
					PutPrimInMiddle(curPrim, ioBBox, tryNum);
					do {
						curLengths *= 1.01;
						curPrim.SetDimensions(curLengths);
					} while (ioBBox->PrimitiveInBox(&curPrim));
						// now the primitive is outside the box again, but not by much

					double yVersionVolume = curPrim.Volume();
					Point3DFloat yVersionLen = curLengths;
					Point3DFloat yVersionCtr = curPrim.GetCtr();

		//  Shrink z slower:
					curLengths = beforeLengths;
					curPrim.SetDimensions(curLengths);
					curPrim.SetCtr(beforeCtr);
					while (!ioBBox->PrimitiveInBox(&curPrim)) {	// as long as the RP isn't inside the box,
															// we shrink it's dimensions by 99%
						curLengths.x *= smFactor;
						curLengths.y *= smFactor;
						curLengths.z *= bigFactor;
						if (curLengths.z < (beforeLengths.z / 1000)) {
							postError("I couldn't make an inscribed box after many tries.  You should inspect the crystal array for an outlier, which is the typical cause of this error.", "Bad Shape", nil, -1, -1);
							throw CalcError();
						}
						curPrim.SetDimensions(curLengths);
					}
					PutPrimInMiddle(curPrim, ioBBox, tryNum);
					do {
						curLengths *= 1.01;
						curPrim.SetDimensions(curLengths);
					} while (ioBBox->PrimitiveInBox(&curPrim));
						// now the primitive is outside the box again, but not by much

					double zVersionVolume = curPrim.Volume();
					Point3DFloat zVersionLen = curLengths;
					Point3DFloat zVersionCtr = curPrim.GetCtr();

		// Choose the best one:
					Point3DFloat maxVolLengths;
					Point3DFloat maxVolCtr;
					double	curMaxVol = 0;

					maxVolLengths = xVersionLen;
					maxVolCtr = xVersionCtr;
					curMaxVol = xVersionVolume;

					if (yVersionVolume > curMaxVol) {
						maxVolLengths = yVersionLen;
						maxVolCtr = yVersionCtr;
						curMaxVol = yVersionVolume;
					}
					if (zVersionVolume > curMaxVol) {
						maxVolLengths = zVersionLen;
						maxVolCtr = zVersionCtr;
						curMaxVol = zVersionVolume;
					}
					if (xyVersionVolume > curMaxVol) {
						maxVolLengths = xyVersionLen;
						maxVolCtr = xyVersionCtr;
						curMaxVol = xyVersionVolume;
					}
					if (yzVersionVolume > curMaxVol) {
						maxVolLengths = yzVersionLen;
						maxVolCtr = yzVersionCtr;
						curMaxVol = yzVersionVolume;
					}
					if (xzVersionVolume > curMaxVol) {
						maxVolLengths = xzVersionLen;
						maxVolCtr = xzVersionCtr;
						curMaxVol = xzVersionVolume;
					}
					if (oldVersionVolume > curMaxVol) {
						maxVolLengths = oldVersionLen;
						maxVolCtr = oldVersionCtr;
						curMaxVol = oldVersionVolume;
					}
					curPrim.SetDimensions(maxVolLengths);
					curPrim.SetCtr(maxVolCtr);
				}	// for tryNum

				while (!ioBBox->PrimitiveInBox(&curPrim)) {	// reduce by very small increments, until
					curLengths *= 0.999;							// it's inside the box
					curPrim.SetDimensions(curLengths);
				}

				log("\tFound RP primitive:");
				char logMsg[kStdStringSize];
				sprintf (logMsg, "\t\t center = (%f, %f, %f); x = %f; y = %f; z = %f\n", ctr.x, ctr.y, ctr.z, curLengths.x, curLengths.y, curLengths.z);
				log(logMsg);

				// take this primitive we've made, and make the BBox that primitive
				ioBBox->SetType(kRPBox);
				ioBBox->SetCtr(curPrim.GetCtr());
				ioBBox->SetDimensions(curPrim.GetSideLenPt());
				
				// make the inscribed box this primitive, too
				if (ioBBox->mInscribedBox == nil) {
					ioBBox->mInscribedBox = new SideSet();
				}
				ioBBox->mInscribedBox->SetType(kRPBox);
				ioBBox->mInscribedBox->SetCtr(curPrim.GetCtr());
				ioBBox->mInscribedBox->SetDimensions(curPrim.GetSideLenPt());
			}
			break;
		case kCylinder:
			if (prefs->verbose) log ("Trying to Find the Best Cylindrical Primitive, as selected by the user.\n");
			if (prefs->exscribedPrimitive) {
				FindBestExscCylPrimitive(ioBBox);
			} else {
				setupProgress("Looking for best primitive to fit in bounding box...", nil, nil, nil, -1, 1, numtries, 1, false);
				double	ht, rad;
				ctr = ioBBox->GetCtr();
				rad = dmh_max(ioBBox->xMax.x - ioBBox->xMin.x, ioBBox->yMax.y - ioBBox->yMin.y) / 2;
				ht = ioBBox->zMax.z - ioBBox->zMin.z;
				curPrim.SetType(kCylBox);
				curPrim.SetCtr(ctr);
				curPrim.SetDimensions(rad, ht);	// this Primitive is now the exscribed box

				if (prefs->verbose) {
					log("\tFound starting primitive:");
					char logMsg[kStdStringSize];
					sprintf (logMsg, "\t\t center = (%f, %f, %f); ht = %f; rad = %f\n", ctr.x, ctr.y, ctr.z, rad, ht);
					log(logMsg);
				}

				for (tryNum = 1; tryNum <= numtries; tryNum++) {
					progress(tryNum);
					while (!ioBBox->PrimitiveInBox(&curPrim)) {	// as long as the RP isn't inside the box,
																// we shrink it's dimensions by 99%
						rad *= 0.99;
						ht *= 0.99;
						curPrim.SetDimensions(rad, ht);
					}
					// now the RP should be inscribed, but might not be very large.  We'll try moving the
					// center, and then expanding the box a bit.
					Point3DFloat lastCtr = ctr;
					double minCtrX, maxCtrX, minCtrY, maxCtrY, minCtrZ, maxCtrZ;
					do {
						ctr.x -= rad * 0.01;
						curPrim.SetCtr(ctr);
					} while (ioBBox->PrimitiveInBox(&curPrim));
					ctr.x += rad * 0.01;
					minCtrX = ctr.x;
					ctr = lastCtr;
					curPrim.SetCtr(lastCtr);
					do {
						ctr.x += rad * 0.01;
						curPrim.SetCtr(ctr);
					} while (ioBBox->PrimitiveInBox(&curPrim));
					ctr.x -= rad * 0.01;
					maxCtrX = ctr.x;
					ctr = lastCtr;
					curPrim.SetCtr(lastCtr);

					do {
						ctr.y -= rad * 0.01;
						curPrim.SetCtr(ctr);
					} while (ioBBox->PrimitiveInBox(&curPrim));
					ctr.y += rad * 0.01;
					minCtrY = ctr.y;
					ctr = lastCtr;
					curPrim.SetCtr(lastCtr);
					do {
						ctr.y += rad * 0.01;
						curPrim.SetCtr(ctr);
					} while (ioBBox->PrimitiveInBox(&curPrim));
					ctr.y -= rad * 0.01;
					maxCtrY = ctr.y;
					ctr = lastCtr;
					curPrim.SetCtr(lastCtr);

					do {
						ctr.z -= ht * 0.01;
						curPrim.SetCtr(ctr);
					} while (ioBBox->PrimitiveInBox(&curPrim));
					ctr.z += ht * 0.01;
					minCtrZ = ctr.z;
					ctr = lastCtr;
					curPrim.SetCtr(lastCtr);
					do {
						ctr.z += ht * 0.01;
						curPrim.SetCtr(ctr);
					} while (ioBBox->PrimitiveInBox(&curPrim));
					ctr.z -= ht * 0.01;
					maxCtrZ = ctr.z;
					curPrim.SetCtr(lastCtr);

					ctr.x = (maxCtrX + minCtrX) / 2.0;
					ctr.y = (maxCtrY + minCtrY) / 2.0;
					ctr.z = (maxCtrZ + minCtrZ) / 2.0;
					curPrim.SetCtr(ctr);
					
					do {
						ht *= 1.01;
						rad *= 1.01;
						curPrim.SetDimensions(rad, ht);
					} while (ioBBox->PrimitiveInBox(&curPrim));
						// now the primitive is outside the box again, but not by much
				}	// for tryNum

				if (prefs->verbose) {
					log("\tAdjusted the primitive.  It is now this:\n");
					char logMsg[kStdStringSize];
					ctr = curPrim.GetCtr();
					rad  = curPrim.GetRadius();
					ht = curPrim.GetHeight();
					sprintf (logMsg, "\t\t center = (%f, %f, %f); ht = %f; rad = %f\n", ctr.x, ctr.y, ctr.z, rad, ht);
					log(logMsg);
				}
				
				while (!ioBBox->PrimitiveInBox(&curPrim)) {	// reduce by very small increments, until
						ht *= 0.999;						// it's inside the box
						rad *= 0.999;
					curPrim.SetDimensions(rad, ht);
				}

				log("\tFound Cyl primitive:\n");
				char logMsg[kStdStringSize];
				sprintf (logMsg, "\t\t center = (%f, %f, %f); ht = %f; rad = %f\n", ctr.x, ctr.y, ctr.z, rad, ht);
				log(logMsg);

				// take this primitive we've made, and make the BBox that primitive
				ioBBox->SetType(kCylBox);
				ioBBox->SetCtr(ctr);
				ioBBox->SetDimensions(rad, ht);
			
				// make the inscribed box this primitive, too
				if (ioBBox->mInscribedBox == nil) {
					ioBBox->mInscribedBox = new SideSet();
				}
				ioBBox->mInscribedBox->SetType(kCylBox);
				ioBBox->mInscribedBox->SetCtr(ctr);
				ioBBox->mInscribedBox->SetDimensions(rad, ht);
			}
			break;
		case kSides: // if kSides is selected, then leave the BBox the way it is - a collection of sides
		default:
			break;
	}
}

// ---------------------------------------------------------------------------
//		• FindBestExscRPPrimitive
// ---------------------------------------------------------------------------
void
Calculator::FindBestExscRPPrimitive(BoundingBox *ioBBox)
{
	SideSet curPrim;
	Point3DFloat ctr, beforeCtr, offset;
	short tryNum;
	short numtries = 100;
	short numOffsetAttempts = 100;
	short thisOffsetAttempt;
	short startXls, nowXls;
	double xDim, yDim, zDim;
	double scale;
	
	ioBBox->PrepForHullOrPrimitive();
	ctr = ioBBox->GetCtr();

	xDim = (ioBBox->xMax.x - ioBBox->xMin.x) * 1.0001;
	yDim = (ioBBox->yMax.y - ioBBox->yMin.y) * 1.0001;
	zDim = (ioBBox->zMax.z - ioBBox->zMin.z) * 1.0001;
	curPrim.SetType(kRPBox);
	curPrim.SetCtr(ctr);
	curPrim.SetDimensions(xDim, yDim, zDim);	// this Primitive is now roughly the exscribed RP

	startXls = curPrim.NumPointsInBox(ioBBox->GetXls());
	
	setupProgress("Looking for best rectangular prism to fit around dataset...", nil, nil, nil, -1, 1, numtries, 1, false);
	
	if (prefs->verbose) {
		log("\tFound starting primitive:\n");
		char logMsg[kStdStringSize];
		sprintf (logMsg, "\t\t center = (%f, %f, %f); dimensions = (%f, %f, %f)\n", ctr.x, ctr.y, ctr.z, xDim, yDim, zDim);
		log(logMsg);
	}
	
	for (tryNum = 1; tryNum <= numtries; tryNum++) {
		progress(tryNum);
		beforeCtr = ctr;
		Boolean goodOffset = false;
		for (thisOffsetAttempt = 1; !goodOffset && (thisOffsetAttempt <= numOffsetAttempts); thisOffsetAttempt++) {
			scale = (numOffsetAttempts - thisOffsetAttempt + 1)/numOffsetAttempts;
			offset.x = RandomDbl(-xDim * scale / 100.0, xDim * scale / 100.0);
			offset.y = RandomDbl(-yDim * scale / 100.0, yDim * scale / 100.0);
			offset.z = RandomDbl(-zDim * scale / 100.0, zDim * scale / 100.0);
			ctr = beforeCtr + offset;
			curPrim.SetCtr(ctr);
			nowXls = curPrim.NumPointsInBox(ioBBox->GetXls());
			if (nowXls == startXls) goodOffset = true;
		}
		if (goodOffset) {
			double increment = xDim/1000.0;
			do {
				// reduce radius by tiny increments until we lose a crystal from the primitive
				xDim -= increment;
				curPrim.SetDimensions(xDim, yDim, zDim);
				nowXls = curPrim.NumPointsInBox(ioBBox->GetXls());				
			} while (nowXls == startXls);
			xDim += increment;
			curPrim.SetDimensions(xDim, yDim, zDim);

			increment = yDim/1000.0;
			do {
				// reduce radius by tiny increments until we lose a crystal from the primitive
				yDim -= increment;
				curPrim.SetDimensions(xDim, yDim, zDim);
				nowXls = curPrim.NumPointsInBox(ioBBox->GetXls());				
			} while (nowXls == startXls);
			yDim += increment;
			curPrim.SetDimensions(xDim, yDim, zDim);

			increment = zDim/1000.0;
			do {
				// reduce radius by tiny increments until we lose a crystal from the primitive
				yDim -= increment;
				curPrim.SetDimensions(xDim, yDim, zDim);
				nowXls = curPrim.NumPointsInBox(ioBBox->GetXls());				
			} while (nowXls == startXls);
			zDim += increment;
			curPrim.SetDimensions(xDim, yDim, zDim);
		}
	}	// for tryNum
	
	if (prefs->verbose) {
		log("\tAdjusted the primitive.  It is now this:\n");
		char logMsg[kStdStringSize];
		ctr = curPrim.GetCtr();
		xDim  = curPrim.GetXLen();
		yDim  = curPrim.GetYLen();
		zDim  = curPrim.GetZLen();
		sprintf (logMsg, "\t\t center = (%f, %f, %f); dimensions = (%f, %f, %f)\n", ctr.x, ctr.y, ctr.z, xDim, yDim, zDim);
		log(logMsg);
	}
	
	xDim = xDim - prefs->shrinkExscribedPrimitive;
	yDim = yDim - prefs->shrinkExscribedPrimitive;
	zDim = zDim - prefs->shrinkExscribedPrimitive;
	
	// take this primitive we've made, and make the BBox that primitive
	ioBBox->SetType(kRPBox);
	ioBBox->SetCtr(ctr);
	ioBBox->SetDimensions(xDim, yDim, zDim);
	
	// make the inscribed box this primitive, too
	if (ioBBox->mInscribedBox == nil) {
		ioBBox->mInscribedBox = new SideSet();
	}
	ioBBox->mInscribedBox->SetType(kRPBox);
	ioBBox->mInscribedBox->SetCtr(ctr);
	ioBBox->mInscribedBox->SetDimensions(xDim, yDim, zDim);	
}

// ---------------------------------------------------------------------------
//		• FindBestExscCylPrimitive
// ---------------------------------------------------------------------------
/*	We'll do numtries iterations of this:
	(1) Move the center a small random X-Y vector until doing do loses no crystals.
		(a) if no moves are possible after 100 tries (getting smaller each try),
			we'll assume we are done.
	(2) Reduce the radius by small amounts until we lose a crystal, then expand 
		it back a bit.
	We then shrink the whole thing by the user-specified value.
*/
void
Calculator::FindBestExscCylPrimitive(BoundingBox *ioBBox)
{
	SideSet curPrim;
	Point3DFloat ctr, beforeCtr, offset;
	short tryNum;
	short numtries = 100;
	short numOffsetAttempts = 100;
	short thisOffsetAttempt;
	short failedOffsets = 0;
	short startXls, nowXls;
	double ht, rad;
	double scale;

	ioBBox->PrepForHullOrPrimitive();
	ctr = ioBBox->GetCtr();
	rad = 1.25 * dmh_max(ioBBox->xMax.x - ioBBox->xMin.x, ioBBox->yMax.y - ioBBox->yMin.y) / 2.0;	// this is expanded (the 1.25) in order to be sure to accommodate all the crystals
	ht = (ioBBox->zMax.z - ioBBox->zMin.z) * 1.0001;	// this doesn't need to be expanded much to capture all the crystals
	curPrim.SetType(kCylBox);
	curPrim.SetCtr(ctr);
	curPrim.SetDimensions(rad, ht);	// this Primitive is now roughly the exscribed cylinder

	startXls = curPrim.NumPointsInBox(ioBBox->GetXls());
	
	setupProgress("Looking for best cylinder to fit around dataset...", nil, nil, nil, -1, 1, numtries, 1, false);

	if (prefs->verbose) {
		log("\tFound starting primitive:\n");
		char logMsg[kStdStringSize];
		sprintf (logMsg, "\t\t center = (%f, %f, %f); ht = %f; rad = %f\n", ctr.x, ctr.y, ctr.z, rad, ht);
		log(logMsg);
	}
	
	for (tryNum = 1; tryNum <= numtries; tryNum++) {
		progress(tryNum);
		beforeCtr = ctr;
		Boolean goodOffset = false;
		for (thisOffsetAttempt = 1; !goodOffset && (thisOffsetAttempt <= numOffsetAttempts); thisOffsetAttempt++) {
			scale = (numOffsetAttempts - thisOffsetAttempt + 1.0) / numOffsetAttempts;
			double XYOffsetRange = rad * 2 * scale / 100.0;
			offset.x = RandomDbl(-XYOffsetRange, XYOffsetRange);
			offset.y = RandomDbl(-XYOffsetRange, XYOffsetRange);
			offset.z = 0;
			ctr = beforeCtr + offset;
			curPrim.SetCtr(ctr);
			nowXls = curPrim.NumPointsInBox(ioBBox->GetXls());
			if (nowXls == startXls) goodOffset = true;
		}
		if (goodOffset) {
			double increment = rad/1000.0;
			do {
				// reduce radius by tiny increments until we lose a crystal from the cylinder
				rad -= increment;
				curPrim.SetDimensions(rad, ht);
				nowXls = curPrim.NumPointsInBox(ioBBox->GetXls());				
			} while (nowXls == startXls);
			rad += increment;
			curPrim.SetDimensions(rad, ht);
		} else {
			failedOffsets++;
			curPrim.SetCtr(beforeCtr);
		}
	}	// for tryNum
	
	if (prefs->verbose) {
		log("\tAdjusted the primitive.  It is now this:\n");
		char logMsg[kStdStringSize];
		ctr = curPrim.GetCtr();
		rad  = curPrim.GetRadius();
		ht = curPrim.GetHeight();
		sprintf (logMsg, "\t\t center = (%f, %f, %f); ht = %f; rad = %f\n", ctr.x, ctr.y, ctr.z, rad, ht);
		log(logMsg);
	}
	
	rad = rad - prefs->shrinkExscribedPrimitive / 2.0;
	ht = ht - prefs->shrinkExscribedPrimitive;
	
	if (prefs->verbose) {
		log("\tShrunk the primitive.  It is now this:\n");
		char logMsg[kStdStringSize];
		sprintf (logMsg, "\t\t center = (%f, %f, %f); ht = %f; rad = %f\n", ctr.x, ctr.y, ctr.z, rad, ht);
		log(logMsg);
	}
	// take this primitive we've made, and make the BBox that primitive
	ioBBox->SetType(kCylBox);
	ioBBox->SetCtr(ctr);
	ioBBox->SetDimensions(rad, ht);
	
	// make the inscribed box this primitive, too
	if (ioBBox->mInscribedBox == nil) {
		ioBBox->mInscribedBox = new SideSet();
	}
	ioBBox->mInscribedBox->SetType(kCylBox);
	ioBBox->mInscribedBox->SetCtr(ctr);
	ioBBox->mInscribedBox->SetDimensions(rad, ht);	
}

// ---------------------------------------------------------------------------
//		• NewFindBestRPPrimitive
// ---------------------------------------------------------------------------
void
Calculator::NewFindBestRPPrimitive(SideSet &ioPrim, BoundingBox *inBBox, HoleSet *inHoles)
{
	Point3DFloat sampleCtr, bestLen, bestCtr, tempLen, randomCtr, randomSidesLength;
	double curVolume = 0, bestVolume = 0;
	long numMCTries = prefs->MCReps;
	
	ioPrim.SetType(kRPBox);
	setupProgress("Looking for best primitive to fit in bounding box...", nil, nil, nil, -1, 1, numMCTries, 1, false);
	
	// Find Center
	sampleCtr = inBBox->GetCtr();
	// Find Bounds
	Point3DFloat lengths(inBBox->xMax.x - inBBox->xMin.x,
						 inBBox->yMax.y - inBBox->yMin.y,
						 inBBox->zMax.z - inBBox->zMin.z);
	// Make box for random centers - centered at sample center, with sides 1/3 bound length
	SideSet centerBox;
	centerBox.SetType(kRPBox);
	centerBox.SetCtr(sampleCtr);
	tempLen = lengths/3;
	centerBox.SetDimensions(tempLen);
	// Make boxes (min & max) for random side lengths - not for real spatial consideration,
	// just ease of coding (we treat xLen, yLen and zLen as 3 dimensions, and then
	// randomize inside a box in those dimensions) - that's why box center is at (xLen/2, yLen/2, ZLen/2)
	SideSet maxSideBox;
	maxSideBox.SetType(kRPBox);
	tempLen = lengths * 1.3;
	maxSideBox.SetDimensions(tempLen);
	maxSideBox.SetCtr(tempLen/2);
	SideSet minSideBox;
	minSideBox.SetType(kRPBox);
	tempLen = lengths * 0.05;
	minSideBox.SetDimensions(tempLen);
	maxSideBox.SetCtr(tempLen/2);
	// For each Try:
	for (long tryNum = 1; tryNum <= numMCTries; tryNum++) {
		progress(tryNum);
		// get random center
		randomCtr = centerBox.RandPtInPrimitive();
		// get random lengths
		do {
			randomSidesLength = maxSideBox.RandPtInPrimitive();
		} while ((randomSidesLength.x < minSideBox.GetXLen()) ||
				 (randomSidesLength.y < minSideBox.GetYLen()) ||
				 (randomSidesLength.z < minSideBox.GetZLen()));
		// adjust ioPrim
		ioPrim.SetCtr(randomCtr);
		ioPrim.SetDimensions(randomSidesLength);
		// if primitive is inside box and
		if (inBBox->PrimitiveInBox(&ioPrim)) {
			// if it has the biggest volume so far, then
			curVolume = ioPrim.Volume();
			if (curVolume > bestVolume) {
				// record these settings as the best so far
				bestVolume = curVolume;
				bestCtr = randomCtr;
				bestLen = randomSidesLength;
			}
		}
	}
	ioPrim.SetCtr(bestCtr);
	ioPrim.SetDimensions(bestLen);
}


// ---------------------------------------------------------------------------
//		• PutPrimInMiddle
// ---------------------------------------------------------------------------
/* This function finds the limits on the where we can place a primitive, and puts
	it in the middle of these limits */
void
Calculator::PutPrimInMiddle(SideSet &ioPrim, BoundingBox *inBBox, short tryNum)
{
	// now the RP should be inscribed, but might not be very large.  We'll try moving the
	// center (faster for early tries), and then expanding the box a bit.
	Point3DFloat firstCtr = ioPrim.GetCtr();
	Point3DFloat ctr = firstCtr;
	Point3DFloat mins, maxes;
	Point3DFloat lengths = ioPrim.GetSideLenPt();

	if (tryNum == -1)
		tryNum = 1;

		// Find lower x bound
	do {
		ctr.x -= (lengths.x/tryNum) * 0.01;
		ioPrim.SetCtr(ctr);
	} while (inBBox->PrimitiveInBox(&ioPrim));
	ctr.x += (lengths.x/tryNum) * 0.01;
	mins.x = ctr.x;

		// Find upper x bound
	ctr = firstCtr;
	ioPrim.SetCtr(firstCtr);
	do {
		ctr.x += (lengths.x/tryNum) * 0.01;
		ioPrim.SetCtr(ctr);
	} while (inBBox->PrimitiveInBox(&ioPrim));
	ctr.x -= (lengths.x/tryNum) * 0.01;
	maxes.x = ctr.x;

		// Find lower y bound
	ctr = firstCtr;
	ioPrim.SetCtr(firstCtr);
	do {
		ctr.y -= (lengths.y/tryNum) * 0.01;
		ioPrim.SetCtr(ctr);
	} while (inBBox->PrimitiveInBox(&ioPrim));
	ctr.y += (lengths.y/tryNum) * 0.01;
	mins.y = ctr.y;

		// Find upper y bound
	ctr = firstCtr;
	ioPrim.SetCtr(firstCtr);
	do {
		ctr.y += (lengths.y/tryNum) * 0.01;
		ioPrim.SetCtr(ctr);
	} while (inBBox->PrimitiveInBox(&ioPrim));
	ctr.y -= (lengths.y/tryNum) * 0.01;
	maxes.y = ctr.y;

		// Find lower z bound
	ctr = firstCtr;
	ioPrim.SetCtr(firstCtr);
	do {
		ctr.z -= (lengths.z/tryNum) * 0.01;
		ioPrim.SetCtr(ctr);
	} while (inBBox->PrimitiveInBox(&ioPrim));
	ctr.z += (lengths.z/tryNum) * 0.01;
	mins.z = ctr.z;

		// Find upper z bound
	ctr = firstCtr;
	ioPrim.SetCtr(firstCtr);
	do {
		ctr.z += (lengths.z/tryNum) * 0.01;
		ioPrim.SetCtr(ctr);
	} while (inBBox->PrimitiveInBox(&ioPrim));
	ctr.z -= (lengths.z/tryNum) * 0.01;
	maxes.z = ctr.z;
	
	ctr = (maxes + mins) / 2.0;
	ioPrim.SetCtr(ctr);
}


// ---------------------------------------------------------------------------
//		• AdaptToPrimitiveBox
// ---------------------------------------------------------------------------
void
Calculator::AdaptToPrimitiveBox(Stats *stats, BoundingBox *inBBox)
{
	Crystal thisXl;
	Crystal *thisXlPtr;
	short thisXlNum;
	CrystalArray *theXls = inBBox->GetXls();
	
	for (thisXlNum = 0; thisXlNum <= theXls->GetNumXls()-1; thisXlNum++) {	// for each Xl
		thisXl = *(theXls->GetItemPtr(thisXlNum));
		if ((thisXlNum <= theXls->GetNumXls()) && (!inBBox->PointInBox(thisXl.ctr))) {
			theXls->RemoveItem(thisXlNum, false);
			thisXlNum--;	// need this, because there's now a different crystal in spot number thisXlNum
		}
	}

	stats->numXlsForL = theXls->GetNumXls();
	for (thisXlNum = 0; thisXlNum <= stats->numXlsForL - 1; thisXlNum++) {	// for each Xl
		thisXlPtr = (Crystal *) theXls->GetItemPtr(thisXlNum);
		stats->newMeanR += thisXlPtr->r;
	}
	stats->newMeanR /=  stats->numXlsForL;
	theXls->RebuildList();
}


#pragma mark ==================== Main Stats
// ---------------------------------------------------------------------------------
//		• DoInitialStats
// ---------------------------------------------------------------------------------
void
Calculator::DoInitialStats(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	CrystalArray *theXls = inBBox->GetXls();
	stats->negRadii = theXls->GetNegRadii();
	
	if (inHoles) {
		stats->BBVolume = FindSwissVolume(inBBox, inHoles);
		stats->surfaceArea = -1;	// Surface area is fairly meaningless for the
									// swiss cheese case
		stats->surfaceToVolRatio = -1;
	} else {
		stats->BBVolume = inBBox->Volume();
		stats->surfaceArea = inBBox->SurfaceArea();
		stats->surfaceToVolRatio = stats->surfaceArea / stats->BBVolume;
	}
	stats->BBSides = inBBox->GetCount();
	stats->numCrystals = theXls->GetNumXls();
	log("Finding Intensity\n");
	stats->intensity = FindIntensity(stats, inBBox, inHoles);
	
	Crystal *thisXl;
	double maxR = 0;
	for (short i=0; i <= theXls->GetNumXls() - 1; i++) {
		thisXl = (Crystal *) theXls->GetItemPtr(i);
		maxR = dmh_max(maxR, thisXl->r);
	}
	stats->numXlsForL = theXls->GetNumXls();
	stats->maxR = maxR;	// needed to compute Volume Frxn
	
	log("Getting Volume Fraction\n");
	stats->volFrxn = inBBox->GetVolumeFraction(maxR, kPrefVal);	// calculates bulk VF even if holes are present (contrasts with VF calc for making env sims)
	stats->totalPosXls = stats->numCrystals - stats->negRadii;

	stats->xMin = inBBox->xMin.x;
	stats->xMax = inBBox->xMax.x;
	stats->yMin = inBBox->yMin.y;
	stats->yMax = inBBox->yMax.y;
	stats->zMin = inBBox->zMin.z;
	stats->zMax = inBBox->zMax.z;
	
	stats->fileType = theXls->GetFileType();

}

// ---------------------------------------------------------------------------
//		• ComputeVolumes
// ---------------------------------------------------------------------------
void
Calculator::ComputeVolumes(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	short	index;
	CrystalArray *theXls = inBBox->GetXls();
	short numXls = theXls->GetNumXls();
	Crystal	*currXl;
	
	setupProgress("Computing volumes...", nil, nil, nil, -1, 0, numXls-1, 0, false);

	for (index = 0; index <= numXls-1; index++) {
		progress(index);
		currXl = (Crystal *) theXls->GetItemPtr(index);
		currXl->extV = fabs((4.0/3.0) * M_PI * pow((double)(fabs(currXl->r)), (double)(3)));	// here we set the extended volume from the radius given in the input file
		if (stats->fileType == kDiffSimulation) {
			currXl->actV = currXl->extV;	// for simulations from Crystallize, the volumes are adjusted in CorrectForImpingement
		} else {
			currXl->actV = CorrectedXlVol(stats, theXls, index, *currXl);
		}
	}
}

// ---------------------------------------------------------------------------
//		• ComputeRadiusStats
// ---------------------------------------------------------------------------
void
Calculator::ComputeRadiusStats(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	double	realNumPosXls;
	double	esdPosDenom;
	double	variance;
	CrystalArray *theXls = inBBox->GetXls();
	int		numXls = theXls->GetNumXls();
	setupProgress("Calculating radius and volume stats...", nil, nil, nil, -1, 0, numXls - 1, 0, false);

	realNumPosXls = stats->totalPosXls;
	esdPosDenom = realNumPosXls * (realNumPosXls - 1);
	
	// compute min, max, & mean for radius & volume
	Bounds radiusBounds, extVolBounds, actVolBounds;
	Crystal *thisXl;
		for (int i=0; i <= numXls-1; i++) {
		progress(i);
		thisXl = (Crystal *) theXls->GetItemPtr(i);
		radiusBounds.Update(fabs(thisXl->r));
		extVolBounds.Update(thisXl->extV);
		actVolBounds.Update(thisXl->actV);
	}
	stats->minR = radiusBounds.min;
	stats->maxR = radiusBounds.max;
	stats->meanR = radiusBounds.sum / stats->numCrystals;
	if (stats->newMeanR == 0.0)
		stats->newMeanR = stats->meanR;
	variance = (stats->numCrystals * radiusBounds.sumsq - sqr(radiusBounds.sum)) / esdPosDenom;
	if (variance > 0)
		stats->esdRadius = sqrt(variance);
	else
		stats->esdRadius = 0.0;
		
	stats->minExtV = extVolBounds.min;
	stats->maxExtV = extVolBounds.max;
	stats->meanExtV = extVolBounds.sum / stats->numCrystals;
	variance = (stats->numCrystals * extVolBounds.sumsq - sqr(extVolBounds.sum)) / esdPosDenom;
	if (variance > 0)
		stats->esdExtVol = sqrt(variance);
	else
		stats->esdExtVol = 0.0;
		
	stats->minActV = actVolBounds.min;
	stats->maxActV = actVolBounds.max;
	stats->meanActV = actVolBounds.sum / stats->numCrystals;
	variance = (stats->numCrystals * actVolBounds.sumsq - sqr(actVolBounds.sum)) / esdPosDenom;
	if (variance > 0)
		stats->esdActVol = sqrt(variance);
	else
		stats->esdActVol = 0.0;
	
	stats->xlVolFraction = actVolBounds.sum / stats->BBVolume;
	stats->xlDensity = stats->numCrystals / stats->BBVolume;
}

// ---------------------------------------------------------------------------
//		• ComputeNeighbors
// ---------------------------------------------------------------------------
void
Calculator::ComputeNeighbors(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	Crystal	*currXl, *otherXl;
	double	testDistSq, minDistSq, boundDist;
	double	testDistEdge, minDistEdge;
	CrystalArray *theXls = inBBox->GetXls();
	short numXls = theXls->GetNumXls();
	Point3DFloat	pt1, pt2;

	setupProgress("Calculating Nearest Neighbors...", nil, nil, nil, -1, 0, numXls-1, 0, false);

		for (short i = 0; i <= numXls - 1; i++) {
		currXl = (Crystal *) theXls->GetItemPtr(i);
		progress(i);
		pt1 = currXl->ctr;
		boundDist = inBBox->NearestSideDist(pt1);
		if (inHoles != nil) {
			boundDist = dmh_min(boundDist, inHoles->NearestHoleDist(pt1));
		}
		minDistSq = sqr(boundDist);
		minDistEdge = boundDist;
		currXl->neighbor = 0; // this will only be changed if a xl is closer then the BBox or a hole
		for (short j = 0; j <= numXls-1; j++) {
			otherXl = (Crystal *) theXls->GetItemPtr(j);
			if ((i != j) && (otherXl->r > 0.0)) {
				pt2 = otherXl->ctr;
				testDistSq = sqr(pt1.Distance(pt2));
				if (testDistSq < minDistSq) {
					minDistSq = testDistSq;
					currXl->neighbor = j;
				}
				testDistEdge = pt1.Distance(pt2) - currXl->r - otherXl->r;
				if (testDistEdge < minDistEdge)
					minDistEdge = testDistEdge;
			}
		}	// for otherXl 
		currXl->ctcDist = sqrt(minDistSq);
		currXl->diffV = minDistEdge;
		if (minDistEdge < 0)
			stats->maxOverlap = dmh_max(-minDistEdge, stats->maxOverlap);
	}	// for currXl 
	}

// ---------------------------------------------------------------------------
//		• ComputeRandomPt
// ---------------------------------------------------------------------------
void
Calculator::ComputeRandomPt(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	CrystalArray *theXls = inBBox->GetXls();
	int	i, currRep, currXl;
	double	minDist, xlDist, sumCubDist;
	double	*means = new double[prefs->RPNumReps + 1];
	double	thisMean;
	Point3DFloat	randPt, pt1, pt2;
	Crystal *thisXl;
	int numXls = theXls->GetNumXls();
	
	setupProgress("Computing random point test...", nil, nil, nil, -1, 0, prefs->RPNumReps, 0, false);
	
		for (currRep = 1; currRep <= prefs->RPNumReps; currRep++) {
		sumCubDist = 0.0;
		progress(currRep);
		for (i = 1; i <= prefs->RPNumPlacings; i++) {
			randPt = Get3DRandPt(inBBox, inHoles);
			thisXl = (Crystal *) theXls->GetItemPtr(1);
			minDist = thisXl->ctr.Distance(randPt);
			for (currXl = 0; currXl <= numXls - 1; currXl++) {
				thisXl = (Crystal *) theXls->GetItemPtr(currXl);
				xlDist = thisXl->ctr.Distance(randPt);
				if (xlDist < minDist)
					minDist = xlDist;
			}	// for currXl 
			double sideDist = NearestSideHoleDist(inBBox, inHoles, &randPt);
			if (minDist > sideDist) {
				i--;
			} else {
				sumCubDist += pow(minDist, 3);
			}
		} // for i
		double intensity = stats->intensity; //(theXls->GetNumXls() / stats->BBVolume);
//		thisMean = sumCubDist / prefs->RPNumPlacings;
//		thisMean *= intensity;
//		thisMean *= (8.0/3.0) * M_PI * prefs->RPNumPlacings;
//		thisMean /= (2.0 * prefs->RPNumPlacings);
		thisMean = (8.0/3.0) * M_PI * intensity * sumCubDist;
//		thisMean = (8.0/3.0) * M_PI * (theXls->GetNumXls() / stats->BBVolume) * sumCubDist;
		means[currRep] = thisMean;
	} // for currRep 
	means[0] = prefs->RPNumReps;
		
	if (prefs->RPNumReps == 1) {
		stats->RPStatMean = means[1];
		stats->RPStatEsd = 0.0;
	} else {
		stats->RPStatMean = stats->RPStatEsd = 0.0;
		for (i = 1; i <= prefs->RPNumReps; i++) {
			stats->RPStatMean += means[i];
			stats->RPStatEsd += sqr(means[i]);
		}
		stats->RPStatMean /= prefs->RPNumReps;
		stats->RPStatEsd = sqrt((stats->RPStatEsd - (prefs->RPNumReps *
							 sqr(stats->RPStatMean))) / (prefs->RPNumReps - 1));
	}
	stats->RPStatMean /= (2.0 * prefs->RPNumPlacings);
	stats->RPStatEsd /= (2.0 * prefs->RPNumPlacings);
	delete[] means;
}

// ---------------------------------------------------------------------------
//		• ComputeCSDStats
// ---------------------------------------------------------------------------
void
Calculator::ComputeCSDStats(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	CrystalArray *	theXls = inBBox->GetXls();
	int				numXls = theXls->GetNumXls();
	double			moment2, moment3, moment4, calc, mean;
	double			p00228, p09772, p01587, p08413;
	short			validNum, temp;
	std::vector<double>		radiusSortArray;
	Crystal			*thisXl;
	short			i;

	validNum = 0;
	mean = 0;
	stats->skewness = 0;
	stats->kurtosis = 0;
		for (i = 0; i <= numXls - 1; i++) {
		thisXl = (Crystal *) theXls->GetItemPtr(i);
		if (thisXl->r > 0) {
			validNum++;
			mean += thisXl->r;
			radiusSortArray.push_back(thisXl->r);
		}
	}
	std::sort(radiusSortArray.begin(), radiusSortArray.end());
	if (validNum > 0) {
		mean /= validNum;
		moment2 = 0;
		moment3 = 0;
		moment4 = 0;
		for (i = 0; i <=numXls - 1; i++) {
			thisXl = (Crystal *) theXls->GetItemPtr(i);
			if (thisXl->r > 0) {
				calc = (thisXl->r - mean);
				moment2 += sqr(calc);
				moment3 += pow(calc, 3);
				moment4 += pow(calc, 4);
			}
		}
		moment2 /= validNum;
		moment3 /= validNum;
		moment4 /= validNum;
		stats->skewness = moment3 / exp(::log(moment2) * 1.5);
		stats->kurtosis = moment4 / (sqr(moment2)) - 3;
			// Do deciles
		for (i = 1; i <= 9; i++) {
			temp = round(i * (validNum / 10.0));
			stats->deciles[i] = radiusSortArray[temp];
		}
		temp = round(0.0228 * validNum);
		p00228 = radiusSortArray[temp];
		temp = round(0.9772 * validNum);
		p09772 = radiusSortArray[temp];
		temp = round(0.1587 * validNum);
		p01587 = radiusSortArray[temp];
		temp = round(0.8413 * validNum);
		p08413 = radiusSortArray[temp];
		stats->myStat1 = 2 * (stats->deciles[5] - p00228) / (p09772 - p00228) - 1;
		stats->myStat2 = 2 * (p08413 - p01587) / (p09772 - p00228) - 1;
	}
}

// ---------------------------------------------------------------------------
//		• ComputeNNStats
// ---------------------------------------------------------------------------
/* computes max, min, etc. for nearest-neighbor-related stuff 
*/
void
Calculator::ComputeNNStats(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	CrystalArray *theXls = inBBox->GetXls();
//	double	uSum, uSumSq, uConst, u, q;
//	short	uNum, currRad, currNNRad;
	short	i;
//	short numRad;
	short numXls = theXls->GetNumXls();
//	bool	kosher;
	Bounds	SepBounds, NNRadBounds;
	Crystal	*thisXl, *neighborXl;

	setupProgress("Calculating ordering stats...", nil, nil, nil, -1, 0,  numXls,  0, false);

	stats->numSep = 0;
	stats->numNNRad = 0;
		for (i = 0; i<= numXls - 1; i++) {
		progress(i);
		thisXl = (Crystal *) theXls->GetItemPtr(i);
		if ((thisXl->r > 0) && (thisXl->neighbor > 0)) {
			(stats->numSep)++;
			SepBounds.Update(thisXl->ctcDist);
			neighborXl = (Crystal *) theXls->GetItemPtr(thisXl->neighbor);
			if (neighborXl->r > 0) {
				stats->numNNRad++;
				NNRadBounds.Update(neighborXl->r);
			}	// if this crystal's neighbor's radius is greater than 0 
		}	// if radius > 0, neighbor > 0 
	} // for i
		stats->minSep = SepBounds.min;
	stats->maxSep = SepBounds.max;
	stats->meanSep = SepBounds.sum / stats->numSep;
	stats->esdSep = sqrt((stats->numSep * SepBounds.sumsq - sqr(SepBounds.sum)) / (stats->numNNRad * (stats->numNNRad - 1)));
	
	stats->minNNRad = NNRadBounds.min;
	stats->maxNNRad = NNRadBounds.max;
	stats->meanNNRad = NNRadBounds.sum / stats->numNNRad;
	stats->esdNNRad = sqrt((stats->numNNRad * NNRadBounds.sumsq - sqr(NNRadBounds.sum)) / (stats->numNNRad * (stats->numNNRad - 1)));

	// compute ordering index 
	stats->randSep = 0.554 / CubeRoot(stats->intensity); //numXls / stats->BBVolume);
	stats->orderingIndex = stats->meanSep / stats->randSep;

	// compute ordering t-test 
	stats->orderingTTest = ::fabs(stats->meanSep - stats->randSep) / (stats->esdSep / sqrt((double)stats->numSep));

	// compute isolation index and t-test 
	stats->isolationIndex = ((stats->meanR - stats->meanNNRad) / stats->meanR) * 100.0;
				// this is the difference between NNRad and Rad as a % of Rad 

/*	This code doesn't work - it reads past end of the two shuffle arrays into random memory locations.
	This failed to crash in versions < 2.6, but crashed reliably on 2.6.  The algorithm is from Reduce 2D
	I think, and I can't figure it out well enough to recode it.

	numRad = numXls - stats->negRadii;
	uConst = sqrt((double) (stats->numNNRad / numRad));
	uSum = 0;
	uSumSq = 0;
	uNum = 0;
	currNNRad = 1;
	currRad = 0;
	
	
	setupProgress("Calculating isolation stats...", nil, nil, nil, -1, 0,  stats->numNNRad,  0, false);


	ShuffleArray iNNRad(theXls->GetNumXls());
	ShuffleArray iRad(theXls->GetNumXls());
	
	short elementToGet;
	Crystal *otherXl;
	while (uNum < stats->numNNRad) {
		progress(uNum);
			// Skip through index lists until the next valid record is found in each
		do{
			currRad++;
			elementToGet = iRad[currRad];
			thisXl = (Crystal *) theXls->GetItemPtr(elementToGet);
		} while (thisXl->r <= 0);

		do {
			elementToGet = iNNRad[currNNRad];
			thisXl = (Crystal *) theXls->GetItemPtr(elementToGet);
			if ((thisXl->r <= 0) || (thisXl->neighbor == 0))
				kosher = false;
			else {
				neighborXl = (Crystal *) theXls->GetItemPtr(thisXl->neighbor);
				if (neighborXl->r <= 0)
					kosher = false;
				else
					kosher = true;
			}
			if (!kosher)
				currNNRad++;
		} while (!kosher);
		uNum++;
		elementToGet = iRad[currNNRad];
		thisXl = (Crystal *) theXls->GetItemPtr(elementToGet);
		elementToGet = iNNRad[currNNRad];
		otherXl = (Crystal *) theXls->GetItemPtr(elementToGet);
		neighborXl = (Crystal *) theXls->GetItemPtr(otherXl->neighbor);
		u = thisXl->r - neighborXl->r * uConst;
		uSum += u;
		uSumSq += sqr(u);
		currNNRad++;
		currRad++;
	}
		delete[] iNNRad;
	delete[] iRad;
	q = stats->numNNRad * uSumSq - sqr(uSum);
	stats->isolationTTest = (stats->meanNNRad - stats->meanR) / sqrt(q / (sqr(stats->numNNRad) * (stats->numNNRad - 1)));
*/
}

// ---------------------------------------------------------------------------
//		• ComputeAvramiTest
// ---------------------------------------------------------------------------
void
Calculator::ComputeAvramiTest(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	CrystalArray *	theXls = inBBox->GetXls();
	int				numXls = theXls->GetNumXls();
	short	currXl;
	
	stats->actualVol = 0.0;
	stats->extendedVol = 0.0;
		for (currXl = 0; currXl <= numXls-1; currXl++) {
		Crystal *thisXl = (Crystal *) theXls->GetItemPtr(currXl);
		stats->actualVol += ::fabs(thisXl->actV);
		stats->extendedVol += ::fabs(thisXl->extV);
	}
		stats->actualVol /= stats->BBVolume;
	stats->extendedVol /= stats->BBVolume;
	stats->extendedVolRand = ::log(1.0 / (1.0 - stats->actualVol));
	stats->avramiRatio = stats->extendedVol / stats->extendedVolRand;
}

// ---------------------------------------------------------------------------
//		• ComputeQuadrat
// ---------------------------------------------------------------------------
void
Calculator::ComputeQuadrat(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	double			quadSize,	// for cube, this is the length of a side, for a sphere, it's
									// the radius.  Either way, it's the size that should enclose 2
									// crystals on average
					quadSizeSq, avgDensity, diff, expected, cumExpectation;
	short			numInQuad, numBins;
	short			currRep, i;
	short			quadBin[kMaxNumBins];
	CFloatArray		results;
	double			thisResult;
	Point3DFloat	thePt, minPt, maxPt, pt1, pt2;
	double			minDim;
	Crystal			*thisXl;
	CrystalArray *	theXls = inBBox->GetXls();
	int				numXls = theXls->GetNumXls();

	// The original version of the Quadrat test used a cubic volume.  This was later adjusted
	//	to use a sphere (for performance reasons, I believe).  If you change the first kSphere in
	//	the following for() statement to kCube, you will iterate over both versions.  The
	//	current code uses only the spherical version.
	for (short circOrQuad = kSphere; circOrQuad <= kSphere; circOrQuad++) {   //Change the first back to "cube" if you want to do the "original" form
		// Calculate optimum quadrat size 
		if (circOrQuad == kCube) {
			quadSize = CubeRoot(stats->BBVolume * 2.0 / theXls->GetNumXls());
			avgDensity = pow(quadSize, 3) * theXls->GetNumXls() / stats->BBVolume;
		} else {
			quadSize = CubeRoot((3.0 * stats->BBVolume * 2.0 / theXls->GetNumXls()) / (4.0 * M_PI));
			quadSizeSq = sqr(quadSize);
			avgDensity = 2.0;
		}

						// See if either is too big for the sample space... 
		minDim = stats->xMax - stats->xMin;
		if (minDim > (stats->yMax - stats->yMin))
			minDim = stats->yMax - stats->yMin;
		if (minDim > (stats->zMax - stats->zMin))
			minDim = stats->zMax - stats->zMin;
		if ((circOrQuad == kCube) && (quadSize >= minDim)) {
			postError("Can't do a quadrat test on this sample.  Evidently, the sample space is too lopsided to be able to fit the right sized cube.", "Quadrat Failed", nil, -1, 3);
			stats->quadratStatMean = 0.0;
			stats->quadratStatEsd = 0.0;
			throw QuadratFailed();
		} else if ((circOrQuad == kSphere) && (quadSize * 2 >= minDim)) {
			postError("Can't do a spherical quadrat test on this sample.  Evidently the sample space is too lopsided to be able to fit the right sized sphere inside.", "Quadrat Failed", nil, -1, 3);
			stats->circQuadratStatMean = 0.0;
			stats->circQuadratStatEsd = 0.0;
			throw QuadratFailed();
		}
		// Figure out when to start lumping bins together (when expected value < 5, according to Kretz, 1969) 
		numBins = 0;
		do {
			numBins++;
			expected = prefs->quadratNumPlacings * exp(-avgDensity) * pow(avgDensity, numBins) / factorial(numBins);
		} while (expected >= 5.0);
		stats->quadratStatDOF = numBins - 1;   // According to Kretz, DOF == numBins (i.e. categories) - 2, and our count starts with 0 

		if (circOrQuad == kCube)
			setupProgress("Computing quadrat test...", nil, nil, nil, -1, 0, prefs->quadratNumReps, 0, false);
		else
			setupProgress("Computing spherical quadrat test...", nil, nil, nil, -1, 0, prefs->quadratNumReps, 0, false);

		for (currRep = 1; currRep <= prefs->quadratNumReps; currRep++) {
			progress(currRep);
			for (i = 0; i <= numBins; i++)
				quadBin[i] = 0;
			for (i = 1; i <= prefs->quadratNumPlacings; i++) {
				if (circOrQuad == kCube) {
					thePt = Get3DRandQuad(stats, inBBox, inHoles, quadSize);
				} else {
					thePt = Get3DRandSphere(stats, inBBox, inHoles, quadSize);
				}
				numInQuad = 0;
				for (int thisXlNum = 0; thisXlNum <= numXls - 1; thisXlNum++) {
					thisXl = (Crystal *) theXls->GetItemPtr(thisXlNum);
					if (circOrQuad == kCube) {
						if ((thisXl->ctr.x > thePt.x) &&
							(thisXl->ctr.x < thePt.x + quadSize) &&
							(thisXl->ctr.y > thePt.y) &&
							(thisXl->ctr.y < thePt.y + quadSize) &&
							(thisXl->ctr.z > thePt.z) &&
							(thisXl->ctr.z < thePt.z + quadSize))
										numInQuad++;
					} else {	// else kSphere
						if (sqr(thePt.Distance(thisXl->ctr)) <= quadSizeSq)
							numInQuad++;
					}
				}
								if (numInQuad > numBins)
					numInQuad = numBins;
				quadBin[numInQuad]++;
			}   // for i
			thisResult = 0;
			cumExpectation = 0;
			for (i = 0; i <= numBins - 1; i++) {
				expected = prefs->quadratNumPlacings * exp(-avgDensity) * pow(avgDensity, i) / factorial(i);
				cumExpectation = cumExpectation + expected;
				diff = quadBin[i] - expected;
				thisResult += diff * diff / expected;
			}
			diff = quadBin[numBins] - (prefs->quadratNumPlacings - cumExpectation);
			thisResult += sqr(diff) / (prefs->quadratNumPlacings - cumExpectation);
			results.Push(&thisResult);
		}   // for currRep 
		if (circOrQuad == kCube) {
			if (prefs->quadratNumReps == 1) {
				stats->quadratStatMean = results[0];
				stats->quadratStatEsd = 0.0;
			} else {
				stats->quadratStatMean = 0.0;
				stats->quadratStatEsd = 0.0;
				for (i = 0; i <= prefs->quadratNumReps - 1; i++) {
					stats->quadratStatMean +=  results[i];
					stats->quadratStatEsd += sqr(results[i]);
				}
				stats->quadratStatMean /=  prefs->quadratNumReps;
				stats->quadratStatEsd = sqrt((stats->quadratStatEsd - (prefs->quadratNumReps * sqr(stats->quadratStatMean))) / (prefs->quadratNumReps - 1));
			}
		} else {
			if (prefs->quadratNumReps == 1) {
				stats->circQuadratStatMean = results[0];
				stats->circQuadratStatEsd = 0.0;
			} else {
				stats->circQuadratStatMean = 0.0;
				stats->circQuadratStatEsd = 0.0;
				for (i = 0; i <= prefs->quadratNumReps - 1; i++) {
					stats->circQuadratStatMean += results[i];
					stats->circQuadratStatEsd += sqr(results[i]);
				}
				stats->circQuadratStatMean /= prefs->quadratNumReps;
				stats->circQuadratStatEsd = sqrt((stats->circQuadratStatEsd - (prefs->quadratNumReps * sqr(stats->circQuadratStatMean))) / (prefs->quadratNumReps - 1));
			}
		}
	}
}

// ---------------------------------------------------------------------------
//		• CorrectedXlVol
// ---------------------------------------------------------------------------
/* This returns a volume reduced by the amount it overlaps with 
other crystals.  This is used to set the actual volume (actV) for the crystal.

DMH: Note that this code is essentially duplicated in CrystalArray::CorrectedCrystalVolume.  Why?
*/
double
Calculator::CorrectedXlVol(Stats *stats, CrystalArray *inXls, short curIndex, Crystal &currXl)
{
	double	vol;		// volume of the crystal - total volume minus sectors
							// belonging to other xls 
	double	dist;		// dist between 2 sphere ctrs 
	double	sumRad;		// sum of two sphere radii 
	double	cosAlpha;	// cos(angle between line connecting 2 xl ctrs &
							// the bdry of their intersxn) 
	double	h;			// distance between dividing plane and the edge of
							// the sphere whose volume we want 
	double	segmentVol;	// volume of the part of the sphere shaved
							// off by the dividing plane 
	Crystal	*thisXl;
	short numXls = inXls->GetNumXls();
	
	vol = ((4.0/3.0) * M_PI * pow((double)(currXl.r), (double)(3)));
		for (int i = 0; (i <= numXls-1 && (vol > 0)); i++) {
		if (i != curIndex) {
			thisXl = (Crystal *) inXls->GetItemPtr(i);
			dist = thisXl->ctr.Distance(currXl.ctr);
			sumRad = thisXl->r + currXl.r;
			if (sumRad > dist) {	// radii longer than dist between spheres, so spheres intersect 
				if ((dist + currXl.r) < thisXl->r) {	// other sphere engulfs this one 
					stats->numEngulfed++;
					vol = 0;
				} else if ((dist + thisXl->r) > currXl.r) {	// make sure other xl isn't engulfed 
					cosAlpha = sqr(dist) + sqr(currXl.r) - sqr(thisXl->r);	// law of cosines 
					cosAlpha /= (2.0 * dist * currXl.r);					// law of cosines, ctd. 
					if (cosAlpha > 0) {
						h = currXl.r * (1.0 - cosAlpha);
						segmentVol = M_PI * sqr(h) * (3.0 * currXl.r - h) / 3.0;	// segment vol, from CRC handbook 
						vol -= segmentVol;
					} else if (cosAlpha == 0) {
						vol -= 2.0 * M_PI * pow((double)(currXl.r), (double)(3)) / 3.0;
					} else {
						cosAlpha = cos(M_PI - acos(cosAlpha));
						h = currXl.r * (1.0 - cosAlpha);
// DMH: The next line had an "=" not an "-=".  That seems wrong.
						vol -= M_PI * sqr(h) * (3.0 * currXl.r - h) / 3.0;	// segment vol, from CRC handbook 
					}
				}	// engulfing checks 
			}	// if sumRad > dist 
		}	// if i != currXl 
	}	// for i 
	if (vol < 0)
		vol = 0;
		return vol;
}

// ---------------------------------------------------------------------------
//		• FindSwissVolume
// ---------------------------------------------------------------------------
/*	Can't just subtract sum of volumes of holes, because there will be overlap.
	Ironing that issue out would be difficult, and very CPU-time-consuming for
	large numbers of hole crystals, so we'll just Monte Carlo it.  */
double
Calculator::FindSwissVolume(BoundingBox *inBBox, HoleSet *inHoles)
{
	Point3DFloat minPoint, maxPoint, tryPoint;
	long inPoints = 0;
	static double vol;
	
	minPoint.x = inBBox->xMin.x;
	minPoint.y = inBBox->yMin.y;
	minPoint.z = inBBox->zMin.z;
	maxPoint.x = inBBox->xMax.x;
	maxPoint.y = inBBox->yMax.y;
	maxPoint.z = inBBox->zMax.z;	// these two points define opposite corners of a rectangular
									// prism that just fits around the Bounding Box
	long MCTries = prefs->MCReps;
	for (long i = 1; i <= MCTries; i++) {
		tryPoint = RandPtNearBox(minPoint, maxPoint);
		if (InBoxNotHole(inBBox, inHoles, tryPoint))
			inPoints++;
	}
	vol = (maxPoint.x - minPoint.x) * (maxPoint.y - minPoint.y) * (maxPoint.z - minPoint.z);
	vol *= ((double) inPoints) / ((double) MCTries);	// multiply volume by fraction
																	// of tries that were "hits"
	return vol;
}

// ---------------------------------------------------------------------------
//		• RandPtNearBox
// ---------------------------------------------------------------------------
/*	Returns a 3D Point that is within the rectangular prism defined by inMinPoint
	and inMaxPoint.  */
Point3DFloat &
Calculator::RandPtNearBox(Point3DFloat &inMinPoint, Point3DFloat &inMaxPoint)
{
	static Point3DFloat outPt;
	outPt.x = RandomDbl(inMinPoint.x, inMaxPoint.x);
	outPt.y = RandomDbl(inMinPoint.y, inMaxPoint.y);
	outPt.z = RandomDbl(inMinPoint.z, inMaxPoint.z);
	return outPt;
}

// ---------------------------------------------------------------------------
//		• InBoxNotHole
// ---------------------------------------------------------------------------
bool
Calculator::InBoxNotHole(BoundingBox *inBBox, HoleSet *inHoles, Point3DFloat &inPoint)
{
	return ((inBBox->PointInBox(inPoint)) && !(inHoles->PointInHole(inPoint)));
}


// ---------------------------------------------------------------------------
//		• Get3DRandPt
// ---------------------------------------------------------------------------
/*	Returns a 3D Point that is within the Bounding Box, and not in a hole. */
Point3DFloat &
Calculator::Get3DRandPt(BoundingBox *inBBox, HoleSet *inHoles)
{
	static Point3DFloat thisPt;
	if (inHoles != nil) {
		do {
			thisPt = inBBox->RandPtInPrimitive();
		} while (inHoles->PointInHole(thisPt));
	} else {
		return inBBox->RandPtInPrimitive();
	}
	return thisPt;
}

// ---------------------------------------------------------------------------
//		• NearestSideHoleDist
// ---------------------------------------------------------------------------
double
Calculator::NearestSideHoleDist(BoundingBox *inBBox, HoleSet *inHoles, Point3DFloat *inPt)
{
	if (inHoles != nil) {
		return dmh_min(inBBox->NearestSideDist(*inPt), inHoles->NearestHoleDist(*inPt));
	} else {
		return inBBox->NearestSideDist(*inPt);
	}
}

// ---------------------------------------------------------------------------
//		• Get3DRandQuad
// ---------------------------------------------------------------------------
Point3DFloat &
Calculator::Get3DRandQuad(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles, double quadSize)
{
	Point3DFloat 		savePt;
	static Point3DFloat	outPt,		// this represents the low-coordinate corner of the quadrat cube
						maxPt, minPt;
	bool				valid;
	
	if (inHoles) {
		postError("You can't currently do a cubic quadrat test on a dataset that contains holes.", "Quadrat problem", nil, -1, 10);
		throw(QuadratFailed());
	}
	if (minPt.x != stats->xMin  || maxPt.x != stats->xMax - quadSize) {	// haven't been here yet
		minPt.x = stats->xMin;
		minPt.y = stats->yMin;
		minPt.z = stats->zMin;
		maxPt.x = stats->xMax - quadSize;
		maxPt.y = stats->yMax - quadSize;
		maxPt.z = stats->zMax - quadSize;
	}
	do {
		outPt = RandPtNearBox(minPt, maxPt);
		savePt = outPt;
		valid = inBBox->PointInBox(outPt);	// x, y, z
		if (valid) {
			outPt.x += quadSize;
			valid = inBBox->PointInBox(outPt);	// x+qS, y, z
		}
		if (valid) {
			outPt.y += quadSize;
			valid = inBBox->PointInBox(outPt);	// x+qS, y+qS, z
		}
		if (valid) {
			outPt.z += quadSize;
			valid = inBBox->PointInBox(outPt);	// x+qS, y+qS, z+qS
		}
		if (valid) {
			outPt.y = savePt.y;
			valid = inBBox->PointInBox(outPt);	// x+qS, y, z+qS
		}
		if (valid) {
			outPt.x = savePt.x;
			valid = inBBox->PointInBox(outPt);	// x, y, z+qS
		}
		if (valid) {
			outPt.y += quadSize;
			valid = inBBox->PointInBox(outPt);	// x, y+qS, z+qS
		}
		if (valid) {
			outPt.z = savePt.z;
			valid = inBBox->PointInBox(outPt);	// x, y+qS, z
		}
	} while (!valid);
	return outPt;
}

// ---------------------------------------------------------------------------
//		• Get3DRandSphere
// ---------------------------------------------------------------------------
Point3DFloat &
Calculator::Get3DRandSphere(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles, double quadSize)
{
	static Point3DFloat	outPt,		// this represents the center of the quadrat sphere
						maxPt, minPt;

	if (minPt.x != stats->xMin || maxPt.x != stats->xMax) {	// haven't been here yet
		minPt.x = stats->xMin;
		minPt.y = stats->yMin;
		minPt.z = stats->zMin;
		maxPt.x = stats->xMax;
		maxPt.y = stats->yMax;
		maxPt.z = stats->zMax;
	}

	if (inHoles) {
		do {
			outPt = RandPtNearBox(minPt, maxPt);
		} while (!(inBBox->PointInBox(outPt))
				 && (inBBox->NearestSideDist(outPt) < quadSize)
				 && (inHoles->NearestHoleDist(outPt) < quadSize));
	} else {
		do {
			outPt = RandPtNearBox(minPt, maxPt);
		} while (!(inBBox->PointInBox(outPt)) && (inBBox->NearestSideDist(outPt) < quadSize));
	}
	return outPt;
}

// Apparently not called:
// ---------------------------------------------------------------------------
//		• GetPercentSphereInsideBoxNotHoles
// ---------------------------------------------------------------------------
/* Although there are probably more elegant ways to do this, we'll use a Monte Carlo
	to minimize programming time.  We get even sampling over a sphere by randomizing
	an azimuth in the x-y plane, and then randomizing a normalized inclination. 
double
Calculator::GetPercentSphereInsideBoxNotHoles(Point3DFloat inCtr, double inRadius, short inNumSpherePts)
{

	long	numInBox = 0;
	double azimuth;
	double radiusAtZ;
	Point3DFloat thisPt;
	
	for (long i = 1; i <= inNumSpherePts; i++) {
		thisPt.z = inRadius * cos(RandomDbl(0, M_PI));
		radiusAtZ = sqrt(sqr(inRadius) - sqr(thisPt.z));

		azimuth = RandomDbl(0, 2*M_PI);
		thisPt.x = radiusAtZ * cos(azimuth);
		thisPt.y = radiusAtZ * sin(azimuth);
		thisPt += inCtr;
		if (InBoxNotHole(inBBox, inHoles, thisPt))
			numInBox++;
	}

	return ((double) numInBox / (double) inNumSpherePts);
}
*/


#pragma mark ==================== Correlation Function Stuff
// ---------------------------------------------------------------------------
//		• ComputeCFs_Both
// ---------------------------------------------------------------------------
/*	Doing this calculation can be exceedingly slow, so this routine has been
	highly optimized for speed & efficiency (I hope).  The basic idea is this:
	over a range of scales ("hDistances"), we count the number of pairs of crystals
	whose centers are (1) separated by less than or equal to that distance (L function),
	or (2) separated by about that distance (PCF, MCF).  Instead of doing that task
	this way:
	
	for each hDistance
		for each crystal
			for each other crystal
				are the two crystals separated by less than the hDistance??
			end for
		end for
	end for
	
	...which would require lots of distance calculations, we do it this way:
	
	for each crystal
		for each other crystal
			figure out separation
			for each hDistance
				is this hDistance greater than the separation?
			end for
		end for
	end for
*/
void
Calculator::ComputeCFs_Both(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	CrystalArray *	theXls = inBBox->GetXls();
	long numPts = stats->numLPoints;
	double EpBandwidth = prefs->EpanecnikovCVal / CubeRoot(stats->intensity);
	
	setupProgress("Calculating L, PCF, MCF...", nil, nil, nil, -1, 0,  stats->numXlsForL,  0, false);

	double			tempPCF, h, OIV;
 	double			curDist;
	long			thisHNum;
	double			nearestSideThis, nearestSideOther;
	
	// these are 1-based; I don't want to recode them
	long			*numXlsUsed = new long[numPts+1];	// N in K2 formula; N+ is stats->numCrystals
	long			*numMCFXlsUsed = new long[numPts+1];
	long			*numMCFgdXlsUsed = new long[numPts+1];
	Crystal			*otherXl;
	Crystal			*thisXl;

	for (thisHNum = 0; thisHNum <= numPts; thisHNum++) {
		numXlsUsed[thisHNum] = 0.0;
		numMCFXlsUsed[thisHNum] = 0.0;
		numMCFgdXlsUsed[thisHNum] = 0.0;
	}
	
	for (short thisXlNum = 0; thisXlNum <= stats->numXlsForL-1; thisXlNum++) {	// for each Xl
		progress(thisXlNum);
		thisXl = (Crystal *) theXls->GetItemPtr(thisXlNum);
		if (inHoles) {
			nearestSideThis = dmh_min(inBBox->NearestSideDist(thisXl->ctr), inHoles->NearestHoleDist(thisXl->ctr));
		} else {
			nearestSideThis = inBBox->NearestSideDist(thisXl->ctr);
		}
		for (thisHNum = 1; thisHNum <= numPts; thisHNum++) {	// these arrays are 1-based;  I don't want to recode them
			if (stats->hDistances[thisHNum] <= nearestSideThis)
				numXlsUsed[thisHNum]++;
		}
		for (short otherXlNum = thisXlNum+1; otherXlNum <= stats->numXlsForL-1; otherXlNum++) {
			otherXl = (Crystal *) theXls->GetItemPtr(otherXlNum);
			if (otherXlNum != thisXlNum) {
				if (inHoles) {
					nearestSideOther = dmh_min(inBBox->NearestSideDist(otherXl->ctr), inHoles->NearestHoleDist(otherXl->ctr));
				} else {
					nearestSideOther = inBBox->NearestSideDist(otherXl->ctr);
				}
				curDist = thisXl->ctr.Distance(otherXl->ctr);
				thisHNum = 1;
				if (curDist <= stats->hDistances[numPts] + EpBandwidth) {	// if the distance is in the range of our h's
					OIV = OffsetIntersectVol(inBBox, thisXl->ctr, otherXl->ctr);
					while (thisHNum <= numPts) {	// for each h, h > curDist
						h = stats->hDistances[thisHNum];
						if (curDist <= h) {
							stats->LValues[thisHNum] += 2.0/OIV;
							if (h <= nearestSideThis) {
								stats->LgdValues[thisHNum] += 1.0;
							}
							if (h <= nearestSideOther) {
								stats->LgdValues[thisHNum] += 1.0;
							}
						}
						if (prefs->useRaeburn)
							tempPCF = Epanecnikov(EpBandwidth, curDist - h) / (4 * M_PI * sqr(curDist));	// Raeburn p.132
						else
							tempPCF = Epanecnikov(EpBandwidth, curDist - h) / (4 * M_PI * sqr(h));	// Stoyan & Stoyan p.284

						if (tempPCF > 0) {
							stats->PCF[thisHNum] += 2.0 * tempPCF / OIV;
							double thisV = thisXl->extV;
							double otherV = otherXl->extV;
							if (prefs->useVolume) {
								stats->MCF[thisHNum] += 2.0 * thisV * otherV * tempPCF / OIV;
								stats->MCF3[thisHNum] += 0.5 * (thisV + otherV);
							} else {
								stats->MCF[thisHNum] += 2.0 * thisXl->r * otherXl->r * tempPCF / OIV;
								stats->MCF3[thisHNum] += 0.5 * (thisXl->r + otherXl->r);
							}
							numMCFXlsUsed[thisHNum]++;
							if (h <= nearestSideThis) {
								stats->PCFgd[thisHNum] += tempPCF;
								if (prefs->useVolume) {
									stats->MCFgd[thisHNum] += thisV * otherV * tempPCF;
									stats->MCF3gd[thisHNum] += 0.5 * (thisV + otherV);
								} else {
									stats->MCFgd[thisHNum] += thisXl->r * otherXl->r * tempPCF;
									stats->MCF3gd[thisHNum] += 0.5 * (thisXl->r + otherXl->r);
								}
								numMCFgdXlsUsed[thisHNum]++;
							}
							if (h <= nearestSideOther) {
								stats->PCFgd[thisHNum] += tempPCF;
								if (prefs->useVolume) {
									stats->MCFgd[thisHNum] += thisV * otherV * tempPCF;
									stats->MCF3gd[thisHNum] += 0.5 * (thisV + otherV);
								} else {
									stats->MCFgd[thisHNum] += thisXl->r * otherXl->r * tempPCF;
									stats->MCF3gd[thisHNum] += 0.5 * (thisXl->r + otherXl->r);
								}
								numMCFgdXlsUsed[thisHNum]++;
							}
						}
						thisHNum++;
					}
				}
			}
		}
			}
	stats->intensitySqd = FindIntensitySquared(stats, inBBox, inHoles);
	for (thisHNum = 1; thisHNum <= numPts; thisHNum++) {	// for each h:
		stats->LValues[thisHNum] = pow((double)(0.75 * (
			stats->LValues[thisHNum] / stats->intensitySqd		// <-- K function
			) / M_PI), (double)(1.0/3.0)) - stats->hDistances[thisHNum];
		stats->PCF[thisHNum] = stats->PCF[thisHNum] / stats->intensitySqd;
		if (prefs->useVolume) {
			stats->MCF[thisHNum] = stats->MCF[thisHNum] / (stats->intensitySqd * sqr(stats->meanExtV));
			if (stats->PCF[thisHNum] != 0) {
				stats->MCF[thisHNum] = stats->MCF[thisHNum] / stats->PCF[thisHNum];
			} else {
				// PCF is zero, so
				stats->MCF[thisHNum] = 0;
			}
			if (numMCFXlsUsed[thisHNum] != 0) {
				stats->MCF3[thisHNum] = stats->MCF3[thisHNum] / (numMCFXlsUsed[thisHNum] * stats->meanExtV);
			}
		} else {
			stats->MCF[thisHNum] = stats->MCF[thisHNum] / (stats->intensitySqd * sqr(stats->newMeanR));
			if (stats->PCF[thisHNum] != 0) {
				stats->MCF[thisHNum] = stats->MCF[thisHNum] / stats->PCF[thisHNum];
			} else {
				// PCF is zero, so
				stats->MCF[thisHNum] = 0;
			}
			if (numMCFXlsUsed[thisHNum] != 0) {
				stats->MCF3[thisHNum] = stats->MCF3[thisHNum] / (numMCFXlsUsed[thisHNum] * stats->newMeanR);
			}
		}

		if (numXlsUsed[thisHNum] > 0) {
			double KValue = stats->LgdValues[thisHNum] / (inBBox->VolumeMinusGuard(inHoles, stats->hDistances[thisHNum]) * stats->intensitySqd);
			stats->LgdValues[thisHNum] = pow((double)(0.75 * KValue / M_PI), 1.0/3.0) - stats->hDistances[thisHNum];
		}
		stats->PCFgd[thisHNum] = (stats->PCFgd[thisHNum] / (stats->intensity * numXlsUsed[thisHNum]));
		if (prefs->useVolume) {
			stats->MCFgd[thisHNum] = (stats->MCFgd[thisHNum] / (stats->intensity * sqr(stats->meanExtV) * numXlsUsed[thisHNum]));
			if (stats->PCFgd[thisHNum] != 0) {
				stats->MCFgd[thisHNum] = stats->MCFgd[thisHNum] / stats->PCFgd[thisHNum];
			} else {
				stats->MCFgd[thisHNum] = 0;
			}
			if (numMCFgdXlsUsed[thisHNum] != 0) {
				stats->MCF3gd[thisHNum] = (stats->MCF3gd[thisHNum] / (numMCFgdXlsUsed[thisHNum] * stats->meanExtV));
			}
		} else {
			stats->MCFgd[thisHNum] = (stats->MCFgd[thisHNum] / (stats->intensity * sqr(stats->newMeanR) * numXlsUsed[thisHNum]));
			if (stats->PCFgd[thisHNum] != 0) {
				stats->MCFgd[thisHNum] = stats->MCFgd[thisHNum] / stats->PCFgd[thisHNum];
			} else {
				stats->MCFgd[thisHNum] = 0;
			}
			if (numMCFgdXlsUsed[thisHNum] != 0) {
				stats->MCF3gd[thisHNum] = (stats->MCF3gd[thisHNum] / (numMCFgdXlsUsed[thisHNum] * stats->newMeanR));
			}
		}
	}
	delete[] numXlsUsed;
	delete[] numMCFXlsUsed;
	delete[] numMCFgdXlsUsed;
}

// ---------------------------------------------------------------------------
//		• Epanecnikov
// ---------------------------------------------------------------------------
double
Calculator::Epanecnikov(double bandwidth, double t)
{
	if (::fabs(t) > bandwidth)
		return 0;
	else
		return (0.75 * (1 - (sqr(t)/sqr(bandwidth))) / bandwidth);
}

// ---------------------------------------------------------------------------
//		• OffsetIntersectVol
// ---------------------------------------------------------------------------
double
Calculator::OffsetIntersectVol(BoundingBox *inBBox, Point3DFloat inPt1, Point3DFloat inPt2)
{
	Point3DFloat offSet = inPt1-inPt2;
	Point3DFloat curPt;
	short numInside;	

	switch (inBBox->GetType()) {
		case kCubeBox:
			if (((inBBox->GetSideLen() - ::fabs(offSet.x)) < 0) ||
				((inBBox->GetSideLen() - ::fabs(offSet.y)) < 0) ||
				((inBBox->GetSideLen() - ::fabs(offSet.z)) < 0))
				return 0;
			return ((inBBox->GetSideLen() - ::fabs(offSet.x)) *
					(inBBox->GetSideLen() - ::fabs(offSet.y)) *
					(inBBox->GetSideLen() - ::fabs(offSet.z)));
		break;
		case kRPBox:
			if (((inBBox->GetXLen() - ::fabs(offSet.x)) < 0) ||
				((inBBox->GetYLen() - ::fabs(offSet.y)) < 0) ||
				((inBBox->GetZLen() - ::fabs(offSet.z)) < 0))
				return 0;
			return ((inBBox->GetXLen() - ::fabs(offSet.x)) *
					(inBBox->GetYLen() - ::fabs(offSet.y)) *
					(inBBox->GetZLen() - ::fabs(offSet.z)));
		break;
		case kCylBox:
			// formula from Stoyan & Stoyan, appendix K; (version r=R)
			double t, r, h;
			t = sqrt(sqr(offSet.x) + sqr(offSet.y));
			h = inBBox->GetHeight() - ::fabs(offSet.z);	// Was in error - called getZLen here!  Fixed 8/8/07
			r = inBBox->GetRadius();
			if ((h < 0) || (t > 2*r))
				return 0;
			return h * ((2 * sqr(r) * acos(t/(2*r))) -
						(t/2) * sqrt(4*sqr(r) - sqr(t)));
		break;
		case kSidesBox:
			static BoundingBox offsetBox(this);
			static bool firstTimeThru = true;
			if (firstTimeThru)  {
				firstTimeThru = false;
				offsetBox.Copy(*inBBox);
			}
			offsetBox.SetOffset(inPt1-inPt2);
			
			static SideSet reducedCube;	// this is a cube that fits around the intersection volume
			reducedCube.SetType(kCubeBox);
			reducedCube.SetCtr(offSet/2 + inBBox->GetCtr());
			reducedCube.SetSideLen((inBBox->GetExscribedBox())->GetSideLen());
			numInside = 0;
			for (short thisMCTry = 1; thisMCTry <= prefs->numCFOffsetVolPts; thisMCTry++) {
				curPt = reducedCube.RandPtInPrimitive();
				if (offsetBox.PointInBox(curPt) && inBBox->PointInBox(curPt))
					numInside++;
			}
			return (pow((double)(reducedCube.GetSideLen()), (double)(3)) * numInside / prefs->numCFOffsetVolPts);
		break;
		default:
			throw("Problem in OffsetIntersectVolume");
		break;
	}
	return positiveInfinity;
}


// ---------------------------------------------------------------------------
//		• FindIntensitySquared
// ---------------------------------------------------------------------------
/*  The square of an estimator for intensity is not the best estimator for 
	the square of the intensity.  It's better to use N * N-1 / Vol^2.  That's
	what we do here.
	Since using the bounding box will overestimate the intensity, we'll use the
	inscribed box's volume and number of crystals.  If we've got Holes, then
	we'll do Monte Carlo to get the non-hole volume of the inscribed box. */
double
Calculator::FindIntensitySquared(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	CrystalArray * theXls = inBBox->GetXls();
	double intensity = 0;
	if (inHoles){
		SideSet *mIB = inBBox->GetInscribedBox();
		short n = mIB->NumPointsInBox(theXls);
		
		SideSet *mEC = inBBox->GetExscribedBox();
		long inPoints = 0;
		Point3DFloat tryPoint;
	
		long MCTries = prefs->MCReps;
		for (long i = 1; i <= MCTries; i++) {
			tryPoint = mEC->RandPtInPrimitive();
			if (mIB->RawPointInBox(tryPoint) && !inHoles->PointInHole(tryPoint))
				inPoints++;
		}
		double vol = mEC->Volume() * MCTries / inPoints;
		if (stats->forRealDataset) {
			intensity = n * (n-1) / sqr(vol);
		} else {
			intensity = stats->numXlsForL * (stats->numXlsForL-1) / sqr(vol);
		}
	} else {
		if (stats->forRealDataset) {
			SideSet *mIB = inBBox->GetInscribedBox();
			short n = mIB->NumPointsInBox(theXls);
			
			intensity = n * (n-1) / sqr(mIB->Volume());
		} else {
			intensity = stats->numXlsForL * (stats->numXlsForL-1) / sqr(inBBox->Volume());
		}
	}
	return intensity;
}


// ---------------------------------------------------------------------------
//		• FindIntensity
// ---------------------------------------------------------------------------
/* Since using the bounding box will overestimate the intensity, we'll use the
	inscribed box's volume and number of crystals.  If we've got Holes, then
	we'll do Monte Carlo to get the non-hole volume of the inscribed box. */
double
Calculator::FindIntensity(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles)
{
	CrystalArray * theXls = inBBox->GetXls();
	double intensity = 0;

	if (inHoles){
		SideSet *mIB = inBBox->GetInscribedBox();
		short n = mIB->NumPointsInBox(theXls);
		
		SideSet *mEC = inBBox->GetExscribedBox();
		long inPoints = 0;
		Point3DFloat tryPoint;
	
		long MCTries = prefs->MCReps;
		for (long i = 1; i <= MCTries; i++) {
			tryPoint = mEC->RandPtInPrimitive();
			if (mIB->RawPointInBox(tryPoint) && !inHoles->PointInHole(tryPoint))
				inPoints++;
		}
		double vol = mEC->Volume() * MCTries / inPoints;
		if (stats->forRealDataset) {
			intensity = n / vol;
		} else {
			intensity = stats->numXlsForL / vol;
		}
	} else {
		if (stats->forRealDataset) {
			SideSet *mIB = inBBox->GetInscribedBox();
			short n = mIB->NumPointsInBox(theXls);
			
			intensity = n / mIB->Volume();
		} else {
			intensity = stats->numXlsForL / inBBox->Volume();
		}
	}
	return intensity;
}

#pragma mark ============================= Envelope Stuff
// ---------------------------------------------------------------------------
//		• DoEnvelopeSimulations
// ---------------------------------------------------------------------------
/* This method is called from Calculator::reduceOneDataset, after all the statistics
	have been calculated for the input dataset.  This method creates a user-specified
	number of (generally) interface-controlled simulations designed to closely
	match the input dataset (at least the set of input radii), and calculates all the
	same statistics on these simulated datasets as well, in order to make an interface-controlled
	"envelope" to which we can compare the results calculated for the actual dataset. */
void
Calculator::DoEnvelopeSimulations(BoundingBox *inBBox, Stats *stats, HoleSet *inHoles)
{
	CrystalArray *theXls = inBBox->GetXls();
	int numXls = theXls->GetNumXls();

	setupProgress("Making and reducing IC simulations...", nil, "Preparing for Envelopes", "Calculating Envelope Simulations", -1, 1, prefs->numEnvelopeRuns, 0, false);
	log("Starting DoEnvelopeSimulations\n");

	// This line uses the comment string to look for Crystallize stuff.  That ought to be refined
	//	somehow, but for now, it will have to do.
	bool matchingCTDataSet = (theXls->GetFileType() == kReal);	
	short numPts = stats->numLPoints;
	short numEnv = prefs->numEnvelopeRuns;
	bool outputSigmas = prefs->outputSigmas;
	Stats *thisSistats = new Stats(false);	
	thisSistats->SetCFParams(stats->numLPoints, stats->mSigmas);

	// make a list of the radii, in order to make simulations:
	// sort Xls by radii, inverse, so as to simulate placing largest Xls first
	Crystal *thisXl;
	std::vector<double> radiiList;
	for (int i = 0; i <= numXls-1; i++) {
		thisXl = (Crystal *) theXls->GetItemPtr(i);
		radiiList.push_back(thisXl->r);
	}
	std::sort(radiiList.begin(), radiiList.end());
	std::reverse(radiiList.begin(), radiiList.end());
	// radiiList is now in order from largest to smallest radius

	// copy current BBox & Xls so that we can make the simulation in the same
	// Bounding Box, and calculate the stats, etc.
	BoundingBox *thisSimBBox = new BoundingBox(*inBBox);
	CrystalArray *thisXls = thisSimBBox->GetXls();
	thisXls->Clear();	// remove the current set of crystals; we'll be creating a new set shortly
	
	// Envelopes provides storage for the results of the envelope simulation statistics
	EnvelopeData Envelopes(numEnv, numPts, outputSigmas);
	
	try {
		time_t startTime = time(NULL);
		for (short thisEnvRun = 1; thisEnvRun <= numEnv; thisEnvRun++) {
			if (thisEnvRun > 1) {
				double fractionCompleted = ((double)thisEnvRun-1.0f) / numEnv;
				time_t	now = time(NULL);
				double timeSoFar = difftime(now, startTime);	// in seconds
				double totalTime = timeSoFar / fractionCompleted;
				double totalSecsToFinish = totalTime - timeSoFar;	// in seconds
				short hrsToFinish = totalSecsToFinish / 3600.0;
				short minsToFinish = fmod((double_t)(totalSecsToFinish), (double_t)(3600.0)) / 60;
				short secsToFinish = totalSecsToFinish - 60*minsToFinish - 3600*hrsToFinish;
				char logStr[kStdStringSize];
				sprintf(logStr, "\t\t\t\t\tBeginning Envelope #%d\n", thisEnvRun);
				log(logStr);
				char envStr[kStdStringSize];
				sprintf(envStr, "Env. simulation #%d/%d. (ETC:%02d:%02d:%02d)", thisEnvRun, numEnv, hrsToFinish, minsToFinish,secsToFinish);
				setEnvMessage(envStr);
			} else {
				char logStr[kStdStringSize];
				sprintf(logStr, "\t\t\t\t\tBeginning Envelope #%d\n", thisEnvRun);
				log(logStr);
				char envStr[kStdStringSize];
				sprintf(envStr, "Env. simulation #%d/%d", thisEnvRun, numEnv);
				setEnvMessage(envStr);
			}			
			// copy basic data from stats for real dataset; needed for calculations
			thisSistats->Clear();
			thisSistats->numXlsForL = stats->numXlsForL;
			thisSistats->numCrystals = stats->numCrystals;
			thisSistats->BBSides = stats->BBSides;
			thisSistats->BBVolume = stats->BBVolume;
			thisSistats->totalVolume = stats->totalVolume;
			thisSistats->totalPosXls = stats->totalPosXls;
			thisSistats->numLPoints = stats->numLPoints;
			thisSistats->maxR = stats->maxR;
			for (short i = 1; i <= stats->numLPoints; i++) {
				thisSistats->hDistances[i] = stats->hDistances[i];
			}
			
// I wrote this, and I'm no longer sure why (it was late at night): "need to be sure that the env xls doesn't think it's a diffsimulation (for example)"
// It seems wrong, because the CrystalArray::CrystalIntersects method knows when to use the DC criterion based on the
// prefs settings, and I can't find anyplace else that cares whether the CrystalArray is a DCSimulation or not.
// Also, it's important to distinguish between a DC simulation input file that was created by Crystallize (common), 
// and a pseudo-DC (a la Daniel & Spear) envelope simulation created here (uncommon).
			thisXls->InitXlArray(theXls);	//copy basic data for CrystalArray
			thisXls->Clear();
			double newMeanR = MakeGoodRandomSimulation(thisSimBBox, inHoles, radiiList, stats->volFrxn, matchingCTDataSet);

			// really just for debugging purposes: holding down shift-option at the end of making the envelope
			// will save the envelope simulation to a suitably-named integrate file that can then be rendered,
			// reduced, etc.
//			if (([[[NSApplication sharedApplication] currentEvent] modifierFlags] & (NSShiftKeyMask | NSAlternateKeyMask)) > 0) {
				// Not sure if the above Cocoa line is thread-safe
				// This is the Carbon version
//			KeyMap theMap;
			NSLock *theLock = [NSLock new];
			[theLock lock];
			UInt32 curKeys = GetCurrentKeyModifiers();
//			GetKeys(theMap);
			[theLock unlock];
			[theLock release];
//			BigEndianLong map1 = theMap[1];
			
			
//			long mapResult = map1 & (myShiftKey | myOptionKey);
			if ((curKeys & myShiftKey) && (curKeys & myOptionKey)) {
				stringFile *inFile = theXls->GetFile();
				NSString *inPath = (NSString *) inFile->getPath();
				const char *inPathCStr = [[inPath stringByDeletingPathExtension] UTF8String];
				char newPathCStr[kStdStringSize];
				sprintf(newPathCStr, "%s_Sim_%d.Int", inPathCStr, thisEnvRun);
				stringFile saveSimFile(true, newPathCStr);
				char firstLine[kStdStringSize];
				
				CFBundleRef mainBundle = CFBundleGetMainBundle();
				CFDictionaryRef bundleInfoDict = CFBundleGetInfoDictionary( mainBundle );
				CFStringRef mVersionString;
				// If we succeeded, look for our property.
				if ( bundleInfoDict != NULL ) {
					mVersionString = (CFStringRef) CFDictionaryGetValue( bundleInfoDict, CFSTR("CFBundleVersion") );
				}
				
				char versCStr[kStdStringSize];
				bool result = CFStringGetCString (mVersionString, versCStr, (CFIndex) kStdStringSize, kCFStringEncodingUTF8);
				(void)result;	// this is to suppress the compiler warning about "result" being an unused variable
				sprintf(firstLine, "Reduce3D version %s || Envelope output; Run number %d; Original mean radius=%f; New mean radius=%f", versCStr, thisEnvRun, stats->meanR, newMeanR);
				thisSimBBox->writeIntegrateFile(&saveSimFile, firstLine, "Envelope Simulation Integrate File");
			}

			calcStats(thisSistats, thisSimBBox, inHoles);
			ExtractEnvData(thisSistats, Envelopes.GetSet(thisEnvRun));

		}
	} catch (SimulationErr theSimErr) {
		throw theSimErr;
	}

	EvaluateCFStats(stats, Envelopes, outputSigmas, prefs->confidence);

	delete thisSimBBox;
	delete thisSistats;
	log("Finished doing envelope simulations\n");
}

// ---------------------------------------------------------------------------
//		• MakeGoodRandomSimulation
// ---------------------------------------------------------------------------
//	Make a good random simulation (has VF of inVolFraction) and put it into inBBox (i.e. theXls)
/*	Input:	inRadiiList - the list of radii to place in the simulation
			inVolFraction - the VF we're shooting for (the VF of the sample dataset)
			inMatchingCTDataSet - whether the dataset we whose VF we're trying to match
									was derived from CT data, or was manufactured by
									Crystallize or MakeSimulation
*/

double
Calculator::MakeGoodRandomSimulation(BoundingBox *inBBox, HoleSet *inHoles, std::vector<double> &inRadiiList, double inVolFraction, bool inMatchingCTDataSet)
{
	char logStr[kStdStringSize];
	double neededRadiusEachXl;
	CrystalArray *theXls = inBBox->GetXls();
	double roughTolerance = inVolFraction * prefs->VFPercent * 5.0 / 100.0;	// first-cut VF should be within 5 * x % of target VF
	double tolerance = inVolFraction * prefs->VFPercent / 100.0;
	long numXls = inRadiiList.size();
	bool goodSim = false;
	short direction, lastDirection;
	double BBVol = inBBox->Volume();
	static double roughGranularity = inBBox->GetMCVolFracGranularity(kPrefValRough);
	double meanR = 0;
	

	// copy inRadiiList to a working list that can be changed
	std::vector<double> workingList = inRadiiList;
	for (long i = 0; i <= numXls-1; i++) {
		meanR += inRadiiList[i];
	}
	meanR /= numXls;
	double startingMeanRadius = meanR;
	double factor = (meanR > 2) ? 1 : 32;

	setupProgress("Making Envelope Simulation", nil, nil, nil, -1, 0, 0, 0, true);
	
	short initialTries = (prefs->makeDCEnv) ? kNumSimMakerRestarts*10 : 1;
	double betafactorfactor = 1.0;
	bool goodFirstSim = false;

	long tryNum=1;
	do {
		try {
			inBBox->MakeRandomSimulation(workingList, inMatchingCTDataSet, inVolFraction, inHoles, betafactorfactor);	// make random simulation with unchanged radii list, put into inBBox
			goodFirstSim = true;	// we didn't throw an error, so this is a good first try (able to place all radii)
			sprintf(logStr, "Made a good first, possibly DC simulation with betafactorfactor of\t%.3f\t and a VF of\t%.3f\n", betafactorfactor, inVolFraction);
			log(logStr);
			progress(1);
		} catch (BoundingBox::SimulationErr theSimErr) {
			// couldn't make the first simulation - probably failed at "DC Envelopes"
			if (tryNum++ >= initialTries) {
				postError("I couldn't make even one good simulation.  Possibly corrupted data?  Or some odd dataset? Please quit, and complain to the programmer.", "Really Bad Simulation", nil, -1, -1);
			} else {
				// probably best not to show this error message, since it won't auto-dismiss at this time, and we're just going to deal with the problem in code anyhow.
				// postError("I couldn't make a good DC simulation.  Shrinking depleted zones and re-trying.", "Bad Simulation", nil, -1, 3);
				betafactorfactor *= 0.9;	// shrink depleted zones to make crystals fit
			}
		}
	} while (!goodFirstSim);

	if (prefs->matchVF) {	// This is deprecated because it is unwise, but the code should work
		double curVolFrac = inBBox->GetVolumeFraction(workingList[0], kPrefVal, inHoles);	// do one high-precision test - maybe we don't need to change anything!
		goodSim = (::fabs(curVolFrac - inVolFraction) <= tolerance);
		if (!goodSim) {
			try {
				if ((::fabs(curVolFrac - inVolFraction) > roughTolerance)	// we didn't get close enough to avoid the rough steps, AND
					&& (roughGranularity < roughTolerance)) {	// we can actually succeed, because the steps in the VF
																// numbers don't mean we'll jump over the correct range of values
					direction = lastDirection = sign(inVolFraction - curVolFrac);
					while (!goodSim && (factor > 1)) {
						while (!goodSim && (direction == lastDirection)) {
		//					neededVolume = (inVolFraction - curVolFrac) * BBVol;
		//					neededVolumeEachXl = neededVolume / numXls;
		//					neededRadiusEachXl = neededVolumeEachXl * 1.5 / (4.0 * M_PI * meanR);	// this is an empirical approximation
							neededRadiusEachXl = ((inVolFraction - curVolFrac) * BBVol / numXls) * 1.5;	// condensed version
							for (int i = 0; i <= numXls - 1; i++) {
								workingList[i] += neededRadiusEachXl * factor;
							}
							meanR += neededRadiusEachXl * factor;
							theXls->Clear();	// clear out CrystalArray
							inBBox->MakeRandomSimulation(workingList, inMatchingCTDataSet, inVolFraction, inHoles);	// make random simulation with new radii list
							curVolFrac = inBBox->GetVolumeFraction(workingList[0], kPrefValRough, inHoles);
							goodSim = (::fabs(curVolFrac - inVolFraction) <= roughTolerance);
							lastDirection = direction;
							direction = sign(inVolFraction - curVolFrac);
							progress(1);
						}
						factor /= 2.0;
						lastDirection = direction;
					}
				}
			} catch (BoundingBox::SimulationErr theSimErr) {
				// reset radii - they may have gotten far out-of-whack
				meanR = 0;
				for (int i = 0; i <= numXls - 1; i++) {
					workingList[i] = inRadiiList[i]; // reset working radii list
					meanR += workingList[i];
				}
				meanR /= numXls;
				inBBox->MakeRandomSimulation(workingList, inMatchingCTDataSet, inVolFraction, inHoles);	// make random simulation with new radii list
				// then just fall through to high-precision section
			}
			
			factor = 16.0;
			curVolFrac = inBBox->GetVolumeFraction(workingList[0], kPrefVal, inHoles);
			goodSim = (::fabs(curVolFrac - inVolFraction) <= tolerance);
			direction = lastDirection = sign(inVolFraction - curVolFrac);
			while (!goodSim) {
				try {
					while (!goodSim && (direction == lastDirection)) {
						neededRadiusEachXl = ((inVolFraction - curVolFrac) * BBVol / numXls) * 1.5;	// condensed version
						for (int i = 0; i <= numXls - 1; i++) {
							workingList[i] += neededRadiusEachXl * factor;	// this could be a BlockMove instead
						}
						meanR += neededRadiusEachXl * factor;
						theXls->Clear();
						inBBox->MakeRandomSimulation(workingList, inMatchingCTDataSet, inVolFraction, inHoles);	// make random simulation with new radii list
						curVolFrac = inBBox->GetVolumeFraction(workingList[0], kPrefVal, inHoles);
						goodSim = (::fabs(curVolFrac - inVolFraction) <= tolerance);
						lastDirection = direction;
						direction = sign(inVolFraction - curVolFrac);
						progress(1);
					}
				} catch (BoundingBox::SimulationErr theSimErr) {
					if (factor > 0.0001) {
						meanR = 0;
						for (long i = 1; i <= numXls; i++) {
							workingList[i] = inRadiiList[i] + neededRadiusEachXl * factor/2; // reset working radii list
							meanR += workingList[i];
						}
						meanR /= numXls;
					} else {
						postError("After repeated tries, I couldn't make a good simulation.  Please quit, and complain to the programmer.", "Really Bad Simulation", nil, -1, -1);
						throw SimulationErr();
					}
				}
				if (factor > 0.002)
					factor /= 2.0;
				lastDirection = direction;
			}
		}
		sprintf(logStr, "Got Good Sim.  Target VF:\t%.3f\tTarget MeanR:\t%.3f\tEnv VF:\t%.3f\tEnv MeanR:\t%.3f\n",
			inVolFraction, startingMeanRadius, curVolFrac, meanR);
		log(logStr);
	}
	return meanR;
}

// ---------------------------------------------------------------------------
//		• EvaluateCFStats
// ---------------------------------------------------------------------------
/* This takes the results of all the envelope runs, and measures the mean &
	sample standard deviation */
void
Calculator::EvaluateCFStats(Stats *stats, EnvelopeData &Envelopes, bool inoutputSigmas, short inConfidence)
{
	log("Starting to Evaluate the CF Stats\n");
	if (inoutputSigmas) {
		double tempMean, tempSD;
		short numPts = stats->numLPoints;
		
		log("Starting to Measure Means and SDs\n");
		Envelopes.GetMeanAndSD(0, OIKind, tempMean, tempSD);
		stats->OIEnvMean = tempMean;
		stats->OIEnvStdDev = tempSD;
		Envelopes.GetMeanAndSD(0, IIKind, tempMean, tempSD);
		stats->IIEnvMean = tempMean;
		stats->IIEnvMean = tempSD;
		Envelopes.GetMeanAndSD(0, QSMKind, tempMean, tempSD);
		stats->QSMEnvMean = tempMean;
		stats->QSMEnvStdDev = tempSD;
		Envelopes.GetMeanAndSD(0, CQSMKind, tempMean, tempSD);
		stats->CQSMEnvMean = tempMean;
		stats->CQSMEnvStdDev = tempSD;
		Envelopes.GetMeanAndSD(0, RPSMKind, tempMean, tempSD);
		stats->RPSMEnvMean = tempMean;
		stats->RPSMEnvStdDev = tempSD;
		Envelopes.GetMeanAndSD(0, ARKind, tempMean, tempSD);
		stats->AREnvMean = tempMean;
		stats->AREnvStdDev = tempSD;
		Envelopes.GetMeanAndSD(0, MinSepKind, tempMean, tempSD);
		stats->MinSepEnvMean = tempMean;
		stats->MinSepEnvStdDev = tempSD;
		Envelopes.GetMeanAndSD(0, MeanSepKind, tempMean, tempSD);
		stats->MeanSepEnvMean = tempMean;
		stats->MeanSepEnvStdDev = tempSD;
		Envelopes.GetMeanAndSD(0, MNNRKind, tempMean, tempSD);
		stats->MeanNNEnvMean = tempMean;
		stats->MeanNNEnvStdDev = tempSD;
		Envelopes.GetMeanAndSD(0, MEVKind, tempMean, tempSD);
		stats->MeanEVEnvMean = tempMean;
		stats->MeanEVEnvStdDev = tempSD;
		
		setupProgress("Measuring mean and standard deviation of envelopes...", nil, nil, nil, -1, 1,  numPts,  1, false);
		for (short thisHNum = 1; thisHNum <= numPts; thisHNum++) {	// for each h:
			Envelopes.GetMeanAndSD(thisHNum, LKind, tempMean, tempSD);
			stats->LValuesEnvMean[thisHNum] = tempMean;
			stats->LValuesEnvStdDev[thisHNum] = tempSD;
			Envelopes.GetMeanAndSD(thisHNum, LgdKind, tempMean, tempSD);
			stats->LgdValuesEnvMean[thisHNum] = tempMean;
			stats->LgdValuesEnvStdDev[thisHNum] = tempSD;
			Envelopes.GetMeanAndSD(thisHNum, PCFKind, tempMean, tempSD);
			stats->PCFValuesEnvMean[thisHNum] = tempMean;
			stats->PCFValuesEnvStdDev[thisHNum] = tempSD;
			Envelopes.GetMeanAndSD(thisHNum, PCFgdKind, tempMean, tempSD);
			stats->PCFgdValuesEnvMean[thisHNum] = tempMean;
			stats->PCFgdValuesEnvStdDev[thisHNum] = tempSD;
			Envelopes.GetMeanAndSD(thisHNum, MCFKind, tempMean, tempSD);
			stats->MCFValuesEnvMean[thisHNum] = tempMean;
			stats->MCFValuesEnvStdDev[thisHNum] = tempSD;
			Envelopes.GetMeanAndSD(thisHNum, MCF3Kind, tempMean, tempSD);
			stats->MCF3ValuesEnvMean[thisHNum] = tempMean;
			stats->MCF3ValuesEnvStdDev[thisHNum] = tempSD;
			Envelopes.GetMeanAndSD(thisHNum, MCFgdKind, tempMean, tempSD);
			stats->MCFgdValuesEnvMean[thisHNum] = tempMean;
			stats->MCFgdValuesEnvStdDev[thisHNum] = tempSD;
			Envelopes.GetMeanAndSD(thisHNum, MCF3gdKind, tempMean, tempSD);
			stats->MCF3gdValuesEnvMean[thisHNum] = tempMean;
			stats->MCF3gdValuesEnvStdDev[thisHNum] = tempSD;
			progress(thisHNum);
		}
	} else {
		double tempMin, tempMax;
		short numPts = stats->numLPoints;
		
		log("Starting to Measure Confidence Intervals\n");
		Envelopes.GetMinAndMax(0, OIKind, inConfidence, tempMin, tempMax);
		stats->OIEnvMin = tempMin;
		stats->OIEnvMax = tempMax;
		Envelopes.GetMinAndMax(0, IIKind, inConfidence, tempMin, tempMax);
		stats->IIEnvMin = tempMin;
		stats->IIEnvMax = tempMax;
		Envelopes.GetMinAndMax(0, QSMKind, inConfidence, tempMin, tempMax);
		stats->QSMEnvMin = tempMin;
		stats->QSMEnvMax = tempMax;
		Envelopes.GetMinAndMax(0, CQSMKind, inConfidence, tempMin, tempMax);
		stats->CQSMEnvMin = tempMin;
		stats->CQSMEnvMax = tempMax;
		Envelopes.GetMinAndMax(0, RPSMKind, inConfidence, tempMin, tempMax);
		stats->RPSMEnvMin = tempMin;
		stats->RPSMEnvMax = tempMax;
		Envelopes.GetMinAndMax(0, ARKind, inConfidence, tempMin, tempMax);
		stats->AREnvMin = tempMin;
		stats->AREnvMax = tempMax;
		Envelopes.GetMinAndMax(0, MinSepKind, inConfidence, tempMin, tempMax);
		stats->MinSepEnvMin = tempMin;
		stats->MinSepEnvMax = tempMax;
		Envelopes.GetMinAndMax(0, MeanSepKind, inConfidence, tempMin, tempMax);
		stats->MeanSepEnvMin = tempMin;
		stats->MeanSepEnvMax = tempMax;
		Envelopes.GetMinAndMax(0, MNNRKind, inConfidence, tempMin, tempMax);
		stats->MeanNNEnvMin = tempMin;
		stats->MeanNNEnvMax = tempMax;
		Envelopes.GetMinAndMax(0, MEVKind, inConfidence, tempMin, tempMax);
		stats->MeanEVEnvMin = tempMin;
		stats->MeanEVEnvMax = tempMax;
		
		setupProgress("Measuring confidence interval from envelope simulations...", nil, nil, nil, -1, 1,  numPts,  1, false);
		for (short thisHNum = 1; thisHNum <= numPts; thisHNum++) {	// for each h:
			Envelopes.GetMinAndMax(thisHNum, LKind, inConfidence, tempMin, tempMax);
			stats->LValuesEnvMin[thisHNum] = tempMin;
			stats->LValuesEnvMax[thisHNum] = tempMax;
			Envelopes.GetMinAndMax(thisHNum, LgdKind, inConfidence, tempMin, tempMax);
			stats->LgdValuesEnvMin[thisHNum] = tempMin;
			stats->LgdValuesEnvMax[thisHNum] = tempMax;
			Envelopes.GetMinAndMax(thisHNum, PCFKind, inConfidence, tempMin, tempMax);
			stats->PCFValuesEnvMin[thisHNum] = tempMin;
			stats->PCFValuesEnvMax[thisHNum] = tempMax;
			Envelopes.GetMinAndMax(thisHNum, PCFgdKind, inConfidence, tempMin, tempMax);
			stats->PCFgdValuesEnvMin[thisHNum] = tempMin;
			stats->PCFgdValuesEnvMax[thisHNum] = tempMax;
			Envelopes.GetMinAndMax(thisHNum, MCFKind, inConfidence, tempMin, tempMax);
			stats->MCFValuesEnvMin[thisHNum] = tempMin;
			stats->MCFValuesEnvMax[thisHNum] = tempMax;
			Envelopes.GetMinAndMax(thisHNum, MCF3Kind, inConfidence, tempMin, tempMax);
			stats->MCF3ValuesEnvMin[thisHNum] = tempMin;
			stats->MCF3ValuesEnvMax[thisHNum] = tempMax;
			Envelopes.GetMinAndMax(thisHNum, MCFgdKind, inConfidence, tempMin, tempMax);
			stats->MCFgdValuesEnvMin[thisHNum] = tempMin;
			stats->MCFgdValuesEnvMax[thisHNum] = tempMax;
			Envelopes.GetMinAndMax(thisHNum, MCF3gdKind, inConfidence, tempMin, tempMax);
			stats->MCF3gdValuesEnvMin[thisHNum] = tempMin;
			stats->MCF3gdValuesEnvMax[thisHNum] = tempMax;
			progress(thisHNum);
		}
	}
	log("Finished Evaluating CF Stats\n");
}

// ---------------------------------------------------------------------------
//		• ExtractEnvData
// ---------------------------------------------------------------------------
void
Calculator::ExtractEnvData(Stats *inStats, OneSet *ioEnv)
{

	// extract & store values from our Stats class for this evelope run:
	ioEnv->orderingIndex = inStats->orderingIndex;
	ioEnv->isolationIndex = inStats->isolationIndex;
	ioEnv->quadratStatMean = inStats->quadratStatMean;
	ioEnv->circQuadratStatMean = inStats->circQuadratStatMean;
	ioEnv->RPStatMean = inStats->RPStatMean;
	ioEnv->avramiRatio = inStats->avramiRatio;
	ioEnv->minSep = inStats->minSep;
	ioEnv->meanSep = inStats->meanSep;
	ioEnv->meanNNRad = inStats->meanNNRad;
	ioEnv->meanExtV = inStats->meanExtV;
	
	for (short thisHNum=1; thisHNum <= inStats->numLPoints; thisHNum++) {
		// extract CF data:
		ioEnv->L[thisHNum] = inStats->LValues[thisHNum];
		ioEnv->PCF[thisHNum] = inStats->PCF[thisHNum];
		ioEnv->MCF[thisHNum] = inStats->MCF[thisHNum];
		ioEnv->MCF3[thisHNum] = inStats->MCF3[thisHNum];
		ioEnv->Lgd[thisHNum] = inStats->LgdValues[thisHNum];
		ioEnv->PCFgd[thisHNum] = inStats->PCFgd[thisHNum];
		ioEnv->MCFgd[thisHNum] = inStats->MCFgd[thisHNum];
		ioEnv->MCF3gd[thisHNum] = inStats->MCF3gd[thisHNum];
	}
}


#pragma mark Output methods

// ---------------------------------------------------------------------------
//		• saveResults
// ---------------------------------------------------------------------------
void			
Calculator::saveResults(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles, short shaveIteration)
{
	setupProgress("Writing Output Files...", nil, nil, nil, 6, 0, 6, 0, false);
	log("Starting saveResults\n");

	if (prefs->includeMeanCSD)
		DoMeanCumCSD(stats, inBBox, shaveIteration);
	progress(1);
	
	if (prefs->includeMaxCSD)
		DoMaxCumCSD(stats, inBBox, shaveIteration);
	progress(2);
	
	if (prefs->includeLogCSD)
		DoLogCSD(stats, inBBox, shaveIteration);
	progress(3);
	
	if (prefs->includeRegCSD) {
		DoNNCSD(stats, inBBox, shaveIteration);
		progress(4);
		DoRegCSD(stats, inBBox, shaveIteration);
		progress(5);
	}
	
	if (prefs->outputReduce)
		DoReduce3DFile(stats, inBBox, inHoles, shaveIteration);
	progress(6);
		
}

// ---------------------------------------------------------------------------
//		• DoMeanCumCSD
// ---------------------------------------------------------------------------
void
Calculator::DoMeanCumCSD(Stats *stats, BoundingBox *inBBox, short shaveIteration)
{
	CrystalArray *	theXls = inBBox->GetXls();
	double		rad;
	CFloatArray	meanCSD;
	double		thisCSDVal;
	short		i, index;
	long		numXls=0;

	// Zero out the CSD array data
	double temp = 0.0;
	for (i = 0; i <= prefs->numClassesMeanCSD - 1; i++) {
		meanCSD.Push(&temp);
	}
	
	for (i=0; i <= theXls->GetNumXls() - 1; i++) {
		Crystal *thisXl = (Crystal *) theXls->GetItemPtr(i);
		if (thisXl->r > 0) {
			rad = thisXl->r;	// uses extV instead of actV 
			index = prefs->numClassesMeanCSD - trunc((rad /
					(stats->meanR * prefs->maxValueMeanCSD)) *
					(prefs->numClassesMeanCSD));
			if (index < 0)
				index = 0;
			numXls++;
			meanCSD.Update(index, (meanCSD[index] + 1));
		}
	}	
	
	for (i = 0; i <= prefs->numClassesMeanCSD - 1; i++) {	// convert to percent 
		thisCSDVal = meanCSD[i];
		thisCSDVal *= (100.0 / numXls);
		meanCSD.Update(i, thisCSDVal);
	}
	SaveCSD(inBBox, stats, shaveIteration, meanCSD, kMeanCumCSD);
}
// ---------------------------------------------------------------------------
//		• DoMaxCumCSD
// ---------------------------------------------------------------------------
void
Calculator::DoMaxCumCSD(Stats *stats, BoundingBox *inBBox, short shaveIteration)
{
	CrystalArray *	theXls = inBBox->GetXls();
	double		rad, maxRad;
	CFloatArray	maxCSD;
	double		thisCSDVal;
	short		i, index;
	
	rad = 0.0;	// temp use of rad
	for (i = 0; i <= prefs->numClassesMaxCSD - 1; i++) {
		maxCSD.Push(&rad);
	}
	
	maxRad = CubeRoot(3.0 * stats->maxActV / (4.0 * M_PI));
	
	for (i=0; i <= theXls->GetNumXls() - 1; i++) {
		Crystal *thisXl = (Crystal *) theXls->GetItemPtr(i);
		if (thisXl->r > 0) {
			rad = thisXl->r;	// uses extV instead of actV 
			index = prefs->numClassesMaxCSD - trunc((rad / stats->maxR) *
													(prefs->numClassesMaxCSD - 0.0001));
			if (index < 0)
				index = 0;
			maxCSD.Update(index, (maxCSD[index] + 1));
		}
	}
	
	
	for (i = 0; i <= prefs->numClassesMaxCSD - 1; i++) {	// convert to percent 
		thisCSDVal = maxCSD[i];
		thisCSDVal *= (100.0 / stats->numCrystals);
		maxCSD.Update(i, thisCSDVal);
	}
	
	SaveCSD(inBBox, stats, shaveIteration, maxCSD, kMaxCumCSD);
}

// ---------------------------------------------------------------------------
//		• DoLogCsd
// ---------------------------------------------------------------------------
void
Calculator::DoLogCSD(Stats *stats, BoundingBox *inBBox, short shaveIteration)
{
	CrystalArray *	theXls = inBBox->GetXls();
	double	rad, maxRad;
	CFloatArray	logCSD, tempCSD;
	short	i, index;
	double	binWidth, avgSlope, slope1, slope2;
	
	rad = 0.0;	// temp use of rad
	for (i = 0; i <= prefs->numClassesLogCSD - 1; i++) {
		logCSD.Push(&rad);
		tempCSD.Push(&rad);
	}
	
	maxRad = CubeRoot(3.0 * stats->maxActV / (4.0 * M_PI));
	// allocate crystals to bins 
	
	for (i=0; i <= theXls->GetNumXls() - 1; i++) {
		Crystal *thisXl = (Crystal *) theXls->GetItemPtr(i);
		if (thisXl->r > 0) {
			rad = thisXl->r;	// uses extV instead of actV 
			index = trunc((rad / stats->maxR) * (prefs->numClassesLogCSD - 0.0001));
			if (index < 1)
				index = 1;
			tempCSD.Update(index, (tempCSD[index] + 1));	// increment the count in that bin
		}
	}
	
	// convert to cumulate totals 
	for (i=1; i <= prefs->numClassesLogCSD-1; i++) {	// the first bin is the same in cumulative and non-cumulative
		tempCSD.Update(i, (tempCSD[i] + tempCSD[i-1]));
	}
	
	// normalize to unit volume 
	for (i = 0; i <= prefs->numClassesLogCSD - 1; i++) { 
		tempCSD.Update(i, (tempCSD[i] / stats->BBVolume));
	}
	
	binWidth = stats->maxR / prefs->numClassesLogCSD;
	
	// convert to logarithms of slopes of cumulate curves:
	// First do bin #1 (really zero, here)
	avgSlope = (tempCSD[1] - tempCSD[0]) / binWidth;	// the slope of the cumulative normalized histogram curve in bin 1
	if (avgSlope > 0)
		logCSD.Update(0, ::log(avgSlope));
	else
		logCSD.Update(0, 0.0);	// This is wrong, but avoids NaN problems
	
	// then do the last bin:
	avgSlope = (tempCSD[prefs->numClassesLogCSD-1] - tempCSD[prefs->numClassesLogCSD - 2]) / binWidth;
	if (avgSlope > 0)
		logCSD.Update(prefs->numClassesLogCSD-1, ::log(avgSlope));
	else
		logCSD.Update(prefs->numClassesLogCSD-1, 0.0);
	
	for (i=1; i <= prefs->numClassesLogCSD-2; i++) {
		slope1 = (tempCSD[i] - tempCSD[i-1]) / binWidth;
		slope2 = (tempCSD[i+1] - tempCSD[i]) / binWidth;
		avgSlope = (slope1 + slope2) / 2.0;
		if (avgSlope > 0)
			logCSD.Update(i, ::log(avgSlope));
		else
			logCSD.Update(i, 0.0);
	}
	
	SaveCSD(inBBox, stats, shaveIteration, logCSD, kLogCSD);
}

// ---------------------------------------------------------------------------
//		• DoNNCSD
// ---------------------------------------------------------------------------
void
Calculator::DoNNCSD(Stats *stats, BoundingBox *inBBox, short shaveIteration)
{
	CrystalArray *	theXls = inBBox->GetXls();
	CFloatArray	NNCSD;
	short		i, index;
	bool		overstepCSD;
	long		numXls=0;
	
	overstepCSD = false;
	
	double maxNNRad = stats->maxNNRad;
	double binSize = maxNNRad / kNumBinsNNCSD;
	
	double dummy = 0.0;
	for (i=0; i<=kNumBinsNNCSD-1; i++) {
		NNCSD.Push(&dummy);
	}
	
	
	for (i=0; i <= theXls->GetNumXls() - 1; i++) {
		Crystal *thisXl = (Crystal *) theXls->GetItemPtr(i);
		Crystal *neighborXl = (Crystal *) theXls->GetItemPtr(thisXl->neighbor);
		if ((thisXl->r > 0) && (thisXl->neighbor > 0)) {
			index = trunc(neighborXl->r / binSize);
			if (index > kNumBinsNNCSD-1) {
				index = kNumBinsNNCSD-1;
				overstepCSD = true;
			}
			numXls++;
			NNCSD.Update(index, (NNCSD[index] + 1));
		}
	}
	
	
	for (i=0; i <= kNumBinsNNCSD-1; i++) {	// convert to percent 
		NNCSD.Update(i, NNCSD[i] * 100.0 / numXls);
	}
	
	SaveCSD(inBBox, stats, shaveIteration, NNCSD, kNNCSD, binSize);
}

// ---------------------------------------------------------------------------
//		• DoRegCSD
// ---------------------------------------------------------------------------
void
Calculator::DoRegCSD(Stats *stats, BoundingBox *inBBox, short shaveIteration)
{
	CrystalArray *	theXls = inBBox->GetXls();
	CFloatArray	regCSD;
	short		i, index;
	bool		overstepCSD;
	long		numXls=0;
	
	overstepCSD = false;
	
	double dummy = 0.0;
	for (i=0; i<=kNumBinsRegCSD-1; i++) {
		regCSD.Push(&dummy);
	}
	double maxRad = stats->maxR;
	double binSize = maxRad / kNumBinsRegCSD;
	
	
	for (i=0; i <= theXls->GetNumXls() - 1; i++) {
		Crystal *thisXl = (Crystal *) theXls->GetItemPtr(i);
		// DMH 8/25/05 - this routine now ignores negative radii here and takes the absolute value.
		//	Formerly, it only wrote the radii data if the radii were positive.  (Radii can be
		//	negative as a signal from Crystallize that they have some problem).
		index = trunc(fabs(thisXl->r) / binSize);	
		if (index > kNumBinsRegCSD-1) {
			index = kNumBinsRegCSD-1;
			overstepCSD = true;
		}
		numXls++;
		regCSD.Update(index, (regCSD[index] + 1));
	}
	
	
	for (i=0; i <= kNumBinsRegCSD-1; i++) {	// convert to percent 
		regCSD.Update(i, regCSD[i] * (100.0 / numXls));
	}
	
	SaveCSD(inBBox, stats, shaveIteration, regCSD, kRegCSD, binSize);
}


// ---------------------------------------------------------------------------------
//		• SaveCSD
// ---------------------------------------------------------------------------------
void
Calculator::SaveCSD(BoundingBox *inBBox, Stats *stats, short shaveIteration, CFloatArray &inCSD, short inCSDKind, double binSize)
{
	const char * csdPath = MakeCSDFilePath(inBBox, inCSDKind);
	
	stringFile saveFile(true, csdPath);
	CrystalArray *theXls = inBBox->GetXls();
	std::string thisLine;
	char	theStr[kStdStringSize];
	char	theTitleStr[kStdStringSize];
	short	numBins;
	double	cumPct;
	short	i;
	
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFDictionaryRef bundleInfoDict = CFBundleGetInfoDictionary( mainBundle );
    CFStringRef mVersionString;
    // If we succeeded, look for our property.
    if ( bundleInfoDict != NULL ) {
        mVersionString = (CFStringRef) CFDictionaryGetValue( bundleInfoDict, CFSTR("CFBundleVersion") );
    }
	
	thisLine = "Reduce3D v";
	char tempLine[kStdStringSize];
	bool result = CFStringGetCString (mVersionString, tempLine, (CFIndex) kStdStringSize, kCFStringEncodingUTF8);
	(void) result;	// evade compiler error for unused variable
	thisLine += tempLine;

	switch (inCSDKind) {
		case kMeanCumCSD:
			thisLine += "; Mean Cumulative CSD (CSD1)";
			sprintf(theTitleStr, "Mean (x%3.2f) normalized size distribution data", prefs->maxValueMeanCSD);
			break;
		case kMaxCumCSD:
			thisLine += "Max Cumulative CSD (CSD2)";
			sprintf(theTitleStr, "Max normalized size distribution data");
			break;
		case kLogCSD:
			thisLine += "Log CSD (CSD3)";
			sprintf(theTitleStr, "Logarithmic size distribution data");
			break;
		case kNNCSD:
			thisLine += "Nearest-Neighbor CSD (CSD5)";
			sprintf(theTitleStr, "Nearest-neighbor size distribution data");
			break;
		case kRegCSD:
			thisLine += "Regular CSD (CSD4)";
			sprintf(theTitleStr, "Regular size distribution data, delta L = %3.3f cm", prefs->deltaLRegCSD);
			break;
	}
	
	if (shaveIteration != -1) {
		sprintf(theStr, "; Shave iteration #%d", shaveIteration);
		thisLine += theStr;
	}
	
	saveFile.putOneLine(thisLine.c_str());
	saveFile.putOneLine(&(theXls->GetIntComment()));
	if (inCSDKind == kNNCSD) {
		sprintf(theStr, "Number of valid crystals:\t%hi", stats->numNNRad);
		saveFile.putOneLine(theStr);
		sprintf(theStr, "Mean nearest neighbor size:\t%f", stats->meanNNRad);
		saveFile.putOneLine(theStr);
	} else {
		sprintf(theStr, "Number of positive crystals:\t%hi", stats->totalPosXls);
		saveFile.putOneLine(theStr);
	}
	saveFile.putOneLine(theTitleStr);
	
	switch (inCSDKind) {
		case kMeanCumCSD:
		case kMaxCumCSD:
			saveFile.putOneLine("bin\tcum\tnoncum");
			break;
		case kLogCSD:
		case kNNCSD:
		case kRegCSD:
			saveFile.putOneLine("bin\tpercent");
			break;
	}
	
	numBins = inCSD.GetCount();
	cumPct = 0.0;
	for (i=0; i <= numBins-1; i++) {
		cumPct += inCSD[i];
		switch (inCSDKind) {
			case kMeanCumCSD:
			case kMaxCumCSD:
				sprintf(theStr, "%10.5f\t%10.5f\t%10.5f",  (1.0 * i / numBins), cumPct, inCSD[i]);
				saveFile.putOneLine(theStr);
				break;
			case kLogCSD:
				sprintf(theStr, "%10.5f\t%10.5f",  (1.0 * i / numBins), inCSD[i]);
				saveFile.putOneLine(theStr);
				break;
			case kNNCSD:
			case kRegCSD:
				sprintf(theStr, "%10.5f\t%10.5f", (i * binSize), inCSD[i]);
				saveFile.putOneLine(theStr);
				break;
		}
	}
}

// ---------------------------------------------------------------------------------
//		• MakeCSDFilePath
// ---------------------------------------------------------------------------------
const char *
Calculator::MakeCSDFilePath(BoundingBox *inBBox, short inCSDKind)
{
	static std::string outPath;
	CrystalArray *theXls = inBBox->GetXls();
	stringFile *inputFile = theXls->GetFile();
	std::string oldPath = [[(NSString *)inputFile->getPath() stringByDeletingPathExtension] UTF8String];
	std::string oldName = [[[(NSString *)inputFile->getPath() stringByDeletingPathExtension] lastPathComponent] UTF8String];

	short maxRootLength = 255 - 13;	// 255 is the maximum filename length for OSX (10.4), and 13 is the longest suffix
	if (oldName.length() > maxRootLength) {
		// then quietly truncate it
		short extralength = oldName.length() - maxRootLength;
		oldPath.resize(oldPath.length() - extralength);
	}

	outPath = oldPath;

	switch (inCSDKind) {
		case kMeanCumCSD:
			outPath.append(".MeanCSD");
		break;
		case kMaxCumCSD:
			outPath.append(".MaxCSD");
		break;
		case kLogCSD:
			outPath.append(".LogCSD");
		break;
		case kNNCSD:
			outPath.append(".NNCSD");
		break;
		case kRegCSD:
			outPath.append(".RegCSD");
		break;
		case kReduce:
			outPath.append(".Reduce3D");
		break;
	}
	return outPath.c_str();
}

// ---------------------------------------------------------------------------
//		• DoReduce3DFile
// ---------------------------------------------------------------------------
void
Calculator::DoReduce3DFile(Stats *stats, BoundingBox *inBBox, HoleSet *inHoles, short shaveIteration)
{
	CrystalArray *theXls = inBBox->GetXls();
	stringFile *inputFile = theXls->GetFile();
	short	i;
	Crystal	*thisXl;
	std::string thisLine;
	
	const char * csdPath = MakeCSDFilePath(inBBox, kReduce);
	
	stringFile saveFile(true, csdPath);
	
	thisLine = inputFile->getName();
	saveFile.putOneLine(thisLine.c_str());

	// write file type line
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFDictionaryRef bundleInfoDict = CFBundleGetInfoDictionary( mainBundle );
    CFStringRef mVersionString;
    // If we succeeded, look for our property.
    if ( bundleInfoDict != NULL ) {
        mVersionString = (CFStringRef) CFDictionaryGetValue( bundleInfoDict, CFSTR("CFBundleVersion") );
    }
	
	thisLine = "Reduce3D v";
	char tempStr[kStdStringSize];
	std::string tempSTLStr;
	bool result = CFStringGetCString (mVersionString, tempStr, (CFIndex) kStdStringSize, kCFStringEncodingUTF8);
	(void) result;	// evade compiler error for unused variable
	thisLine += tempStr;
	saveFile.putOneLine(thisLine.c_str());
	
	// write incoming comment line
	saveFile.putOneLine(&(theXls->GetIntComment()));

	tempSTLStr = "Input Header:\t";
	tempSTLStr += theXls->GetIntVersionStr();
	if (theXls->inputHasExtendedVolume()) {
		tempSTLStr += " (treated input data as extended radii)";
	} else {
		tempSTLStr += " (treated input data as actual radii, and extended them for impingement)";
	}
	saveFile.putOneLine(&tempSTLStr);
	
	// write our processing params
	saveFile.putOneLine("============ Begin Processing Parameters ============");
	WritePrefsSettings(&saveFile);

	// write divider for easy reading
	saveFile.putOneLine("============ End Processing Parameters ============");
	
	// write holes file reference
	thisLine = "Holes File: ";
	if (inHoles != nil) {
		stringFile *holesFile = inHoles->GetFile();
		thisLine += holesFile->getName();
	} else {
		thisLine += "none";
	}
	saveFile.putOneLine(thisLine.c_str());

	saveFile.putOneLine("============ Begin Single-scale Statistics ============");
// write number of crystals
	sprintf(tempStr, "Number of crystals:\t%d", stats->numCrystals);
	saveFile.putOneLine(tempStr);
	
// write total area or volume
	sprintf(tempStr, "Total Volume (cubic cm):\t%.7E", stats->BBVolume); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Surface Area (sq cm):\t%.7E", stats->surfaceArea); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Surface to Volume Ratio:\t%.7E", stats->surfaceToVolRatio); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Crystal Volume Fraction:\t%.7E", stats->xlVolFraction); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Crystal density:\t%.7E", stats->xlDensity); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Crystal intensity:\t%.7E", stats->intensity); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Crystal Intensity Sqd:\t%.7E", stats->intensitySqd); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Crystal Volume Fraction (MC Method):\t%.7E", stats->volFrxn); saveFile.putOneLine(tempStr);

	sprintf(tempStr, "Observability criterion 1 setting:\t%f", stats->observabilityCrit1value);
	sprintf(tempStr, "Observability criterion 2 setting:\t%f", stats->observabilityCrit2value);
	sprintf(tempStr, "Observability criterion 1 rejects:\t%d", stats->observabilityCrit1rejects);
	sprintf(tempStr, "Observability criterion 2 rejects:\t%d", stats->observabilityCrit2rejects);
	
// write min, max, mean and esd for radius
	sprintf(tempStr, "Minimum radius:\t%.7E", stats->minR); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Maximum radius:\t%.7E", stats->maxR); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Mean radius:\t%.7E", stats->meanR); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Std deviation:\t%.7E", stats->esdRadius); saveFile.putOneLine(tempStr);

	sprintf(tempStr, "Skewness:\t%.7E", stats->skewness); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Excess Kurtosis:\t%.7E", stats->kurtosis); saveFile.putOneLine(tempStr);

// write min, max, mean and esd for extended volume
	sprintf(tempStr, "Minimum ext volume:\t%.7E", stats->minExtV); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Maximum ext volume:\t%.7E", stats->maxExtV); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Mean ext volume:\t%.7E\t%s\t%.7E\t%s\t%.7E", 
			stats->meanExtV,
			(prefs->outputSigmas ? "Env Mean:" : "Env Min:"),
			(prefs->outputSigmas ? stats->MeanEVEnvMean : stats->MeanEVEnvMin),
			(prefs->outputSigmas ? "Env SD:" : "Env Max:"),
			(prefs->outputSigmas ? stats->MeanEVEnvStdDev : stats->MeanEVEnvMax));
	saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Std deviation:\t%.7E", stats->esdExtVol); saveFile.putOneLine(tempStr);

// write min, max, mean and esd for actual volume
	sprintf(tempStr, "Minimum act volume:\t%.7E", stats->minActV); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Maximum act volume:\t%.7E", stats->maxActV); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Mean act volume:\t%.7E", stats->meanActV); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Std deviation:\t%.7E", stats->esdActVol); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Num engulfed crystals:\t%d", stats->numEngulfed); saveFile.putOneLine(tempStr);

// write min, max, mean and esd for nearest-neighbor distance
	sprintf(tempStr, "Minimum separation:\t%.7E\t%s\t%.7E\t%s\t%.7E", 
			stats->minSep,
			(prefs->outputSigmas ? "Env Mean:" : "Env Min:"),
			(prefs->outputSigmas ? stats->MinSepEnvMean : stats->MinSepEnvMin),
			(prefs->outputSigmas ? "Env SD:" : "Env Max:"),
			(prefs->outputSigmas ? stats->MinSepEnvStdDev : stats->MinSepEnvMax));
	saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Maximum separation:\t%.7E", stats->maxSep); saveFile.putOneLine(tempStr);

	sprintf(tempStr, "Mean separation:\t%.7E\t%s\t%.7E\t%s\t%.7E", 
			stats->meanSep,
			(prefs->outputSigmas ? "Env Mean:" : "Env Min:"),
			(prefs->outputSigmas ? stats->MeanSepEnvMean : stats->MeanSepEnvMin),
			(prefs->outputSigmas ? "Env SD:" : "Env Max:"),
			(prefs->outputSigmas ? stats->MeanSepEnvStdDev : stats->MeanSepEnvMax));
	saveFile.putOneLine(tempStr);

	sprintf(tempStr, "Std deviation:\t%.7E", stats->esdSep); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Num valid crystals:\t%d", stats->numSep); saveFile.putOneLine(tempStr);

// write ordering index
	sprintf(tempStr, "Ordering index:\t%.7E\t%s\t%.7E\t%s\t%.7E", 
			stats->orderingIndex,
			(prefs->outputSigmas ? "Env Mean:" : "Env Min:"),
			(prefs->outputSigmas ? stats->OIEnvMean : stats->OIEnvMin),
			(prefs->outputSigmas ? "Env SD:" : "Env Max:"),
			(prefs->outputSigmas ? stats->OIEnvStdDev : stats->OIEnvMax));
	saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Ordering index t-test:\t%.7E", stats->orderingTTest); saveFile.putOneLine(tempStr);

// write "isolation index" stats
	sprintf(tempStr, "Mean NN radius:\t%.7E\t%s\t%.7E\t%s\t%.7E", 
			stats->meanNNRad,
			(prefs->outputSigmas ? "Env Mean:" : "Env Min:"),
			(prefs->outputSigmas ? stats->MeanNNEnvMean : stats->MeanNNEnvMin),
			(prefs->outputSigmas ? "Env SD:" : "Env Max:"),
			(prefs->outputSigmas ? stats->MeanNNEnvStdDev : stats->MeanNNEnvMax));
	saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Esd NN radius:\t%.7E", stats->esdNNRad); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Num valid pairs:\t%d", stats->numNNRad); saveFile.putOneLine(tempStr);

	sprintf(tempStr, "Isolation index:\t%.7E\t%s\t%.7E\t%s\t%.7E", 
			stats->isolationIndex,
			(prefs->outputSigmas ? "Env Mean:" : "Env Min:"),
			(prefs->outputSigmas ? stats->IIEnvMean : stats->IIEnvMin),
			(prefs->outputSigmas ? "Env SD:" : "Env Max:"),
			(prefs->outputSigmas ? stats->IIEnvStdDev : stats->IIEnvMax));
	saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Isolation index t-test:\t%.7E", stats->isolationTTest); saveFile.putOneLine(tempStr);

// write quadrat stat
	saveFile.putOneLine("Quadrat stat mean:\tnot calculated");
	saveFile.putOneLine("Quadrat stat esd:\tnot calculated");

	sprintf(tempStr, "Spherical quad stat mean:\t%.7E\t%s\t%.7E\t%s\t%.7E", 
			stats->circQuadratStatMean,
			(prefs->outputSigmas ? "Env Mean:" : "Env Min:"),
			(prefs->outputSigmas ? stats->CQSMEnvMean : stats->CQSMEnvMin),
			(prefs->outputSigmas ? "Env SD:" : "Env Max:"),
			(prefs->outputSigmas ? stats->CQSMEnvStdDev : stats->CQSMEnvMax));
	saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Spherical quad stat esd:\t%.7E", stats->circQuadratStatEsd); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Quadrat stat d.o.f:\t%d", stats->quadratStatDOF); saveFile.putOneLine(tempStr);

// write random point stat
	sprintf(tempStr, "Random point stat mean:\t%.7E\t%s\t%.7E\t%s\t%.7E", 
			stats->RPStatMean,
			(prefs->outputSigmas ? "Env Mean:" : "Env Min:"),
			(prefs->outputSigmas ? stats->RPSMEnvMean : stats->RPSMEnvMin),
			(prefs->outputSigmas ? "Env SD:" : "Env Max:"),
			(prefs->outputSigmas ? stats->RPSMEnvStdDev : stats->RPSMEnvMax));
	saveFile.putOneLine(tempStr);
	sprintf(tempStr, "Random point std dev:\t%.7E", stats->RPStatEsd); saveFile.putOneLine(tempStr);

// write Avrami test parameters
// fix
if ((stats->fileType != kDiffSimulation) || prefs->doImpingement) {
		sprintf(tempStr, "Avrami act vol:\t%.7E", stats->actualVol); saveFile.putOneLine(tempStr);
		sprintf(tempStr, "Avrami ext vol:\t%.7E", stats->extendedVol); saveFile.putOneLine(tempStr);
		sprintf(tempStr, "Avrami ext vol rand:\t%.7E", stats->extendedVolRand); saveFile.putOneLine(tempStr);
		sprintf(tempStr, "Avrami ratio:\t%.7E\t%s\t%.7E\t%s\t%.7E", 
			stats->avramiRatio,
			(prefs->outputSigmas ? "Env Mean:" : "Env Min:"),
			(prefs->outputSigmas ? stats->AREnvMean : stats->AREnvMin),
			(prefs->outputSigmas ? "Env SD:" : "Env Max:"),
			(prefs->outputSigmas ? stats->AREnvStdDev : stats->AREnvMax));
		saveFile.putOneLine(tempStr);
	} else {
		saveFile.putOneLine("Avrami ext vol:\tnot calculated");
		saveFile.putOneLine("Avrami act vol:\tnot calculated");
		saveFile.putOneLine("Avrami ext vol rand:\tnot calculated");
		saveFile.putOneLine("Avrami ratio:\tnot calculated");
	}

// write new stats, deciles
	sprintf(tempStr, "CSD shape stat 1 (skew):\t%.7E", stats->myStat1); saveFile.putOneLine(tempStr);
	sprintf(tempStr, "CSD shape stat 2 (flat):\t%.7E", stats->myStat2); saveFile.putOneLine(tempStr);
	saveFile.putOneLine("Decile#\tDecile value");
	for (i = 1; i <= 9; i++) {
		sprintf(tempStr, "%d\t%.7E", i, stats->deciles[i]);
		saveFile.putOneLine(tempStr);
	}
	saveFile.putOneLine("============ End Single-scale Statistics ============");
	

	if (prefs->doLMcfPcf) {
		// write out correlation function results and stuff
		saveFile.putOneLine("============ Begin Correlation Functions ============");
		sprintf(tempStr, "NumXlsUsed in L/PCF/MCF:\t%d", (prefs->doLMcfPcf ? stats->numXlsForL : -1));
		saveFile.putOneLine(tempStr);
		sprintf(tempStr, "Mean R of Xls used in L/PCF/MCF:\t%f", (prefs->doLMcfPcf ? stats->newMeanR : -1.0));
		saveFile.putOneLine(tempStr);
		
		if (prefs->outputSigmas) {
			saveFile.putOneLine("hDistance(cm)\tL value\tL Mean\tL SD\tLgd value\tLgd Mean\tLgd SD\tPCF\tPCF Mean\tPCF SD\tPCFgd\tPCFgd Mean\tPCFgd SD\tMCF\tMCF Mean\tMCF SD\tMCF3\tMCF3 Mean\tMCF3 SD\tMCFgd\tMCFgd Mean\tMCFgd SD\tMCF3gd\tMCF3gd Mean\tMCF3gd SD");
		} else {
			saveFile.putOneLine("hDistance(cm)\tL value\tL Min\tL Max\tLgd value\tLgd Min\tLgd Max\tPCF\tPCF Min\tPCF Max\tPCFgd\tPCFgd Min\tPCFgd Max\tMCF\tMCF Min\tMCF Max\tMCF3\tMCF3 Min\tMCF3 Max\tMCFgd\tMCFgd Min\tMCFgd Max\tMCF3gd\tMCF3gd Min\tMCF3gd Max");
		}
		for (i = 1; i <= stats->numLPoints; i++) {
			if (prefs->numEnvelopeRuns == 0) {
				sprintf(tempStr, "%.7E\t%.7E\t\t\t%.7E\t\t\t%.7E\t\t\t%.7E\t\t\t%.7E\t\t\t%.7E\t\t\t%.7E\t\t\t%.7E\t\t\t",
				stats->hDistances[i],
				stats->LValues[i],
				stats->LgdValues[i],
				stats->PCF[i],
				stats->PCFgd[i],
				stats->MCF[i],
				stats->MCFgd[i],
				stats->MCF3[i],
				stats->MCF3gd[i]);
				saveFile.putOneLine(tempStr);
			} else {
				if (prefs->outputSigmas) {
					sprintf(tempStr, "%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t",
							stats->hDistances[i],
							FlaggedCFVal(true, stats->LValues[i]),
							FlaggedCFEnvVal(true, stats->LValuesEnvMean[i]),
							FlaggedCFEnvVal(true, stats->LValuesEnvStdDev[i]),
							FlaggedCFVal(true, stats->LgdValues[i]),
							FlaggedCFEnvVal(true, stats->LgdValuesEnvMean[i]),
							FlaggedCFEnvVal(true, stats->LgdValuesEnvStdDev[i]),
							FlaggedCFVal(true, stats->PCF[i]),
							FlaggedCFEnvVal(true, stats->PCFValuesEnvMean[i]),
							FlaggedCFEnvVal(true, stats->PCFValuesEnvStdDev[i]),
							FlaggedCFVal(true, stats->PCFgd[i]),
							FlaggedCFEnvVal(true, stats->PCFgdValuesEnvMean[i]),
							FlaggedCFEnvVal(true, stats->PCFgdValuesEnvStdDev[i]),
							FlaggedCFVal(true, stats->MCF[i]),
							FlaggedCFEnvVal(true, stats->MCFValuesEnvMean[i]),
							FlaggedCFEnvVal(true, stats->MCFValuesEnvStdDev[i]),
							FlaggedCFVal(true, stats->MCFgd[i]),
							FlaggedCFEnvVal(true, stats->MCFgdValuesEnvMean[i]),
							FlaggedCFEnvVal(true, stats->MCFgdValuesEnvStdDev[i]),
							FlaggedCFVal(true, stats->MCF3[i]),
							FlaggedCFEnvVal(true, stats->MCF3ValuesEnvMean[i]),
							FlaggedCFEnvVal(true, stats->MCF3ValuesEnvStdDev[i]),
							FlaggedCFVal(true, stats->MCF3gd[i]),
							FlaggedCFEnvVal(true, stats->MCF3gdValuesEnvMean[i]),
							FlaggedCFEnvVal(true, stats->MCF3gdValuesEnvStdDev[i]));
				} else {
					sprintf(tempStr, "%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t",
							stats->hDistances[i],
							FlaggedCFVal(false, stats->LValues[i]),
							FlaggedCFEnvVal(false, stats->LValuesEnvMin[i]),
							FlaggedCFEnvVal(false, stats->LValuesEnvMax[i]),
							FlaggedCFVal(false, stats->LgdValues[i]),
							FlaggedCFEnvVal(false, stats->LgdValuesEnvMin[i]),
							FlaggedCFEnvVal(false, stats->LgdValuesEnvMax[i]),
							FlaggedCFVal(false, stats->PCF[i]),
							FlaggedCFEnvVal(false, stats->PCFValuesEnvMin[i]),
							FlaggedCFEnvVal(false, stats->PCFValuesEnvMax[i]),
							FlaggedCFVal(false, stats->PCFgd[i]),
							FlaggedCFEnvVal(false, stats->PCFgdValuesEnvMin[i]),
							FlaggedCFEnvVal(false, stats->PCFgdValuesEnvMax[i]),
							FlaggedCFVal(false, stats->MCF[i]),
							FlaggedCFEnvVal(false, stats->MCFValuesEnvMin[i]),
							FlaggedCFEnvVal(false, stats->MCFValuesEnvMax[i]),
							FlaggedCFVal(false, stats->MCFgd[i]),
							FlaggedCFEnvVal(false, stats->MCFgdValuesEnvMin[i]),
							FlaggedCFEnvVal(false, stats->MCFgdValuesEnvMax[i]),
							FlaggedCFVal(false, stats->MCF3[i]),
							FlaggedCFEnvVal(false, stats->MCF3ValuesEnvMin[i]),
							FlaggedCFEnvVal(false, stats->MCF3ValuesEnvMax[i]),
							FlaggedCFVal(false, stats->MCF3gd[i]),
							FlaggedCFEnvVal(false, stats->MCF3gdValuesEnvMin[i]),
							FlaggedCFEnvVal(false, stats->MCF3gdValuesEnvMax[i]));
				}
				saveFile.putOneLine(tempStr);
			}
		}
	}
	if (prefs->outputCrystals) {
		saveFile.putOneLine("============ Begin Crystal Dataset ============");
		int numXls = theXls->GetNumXls();
		// write labels for expanded raw data file
		saveFile.putOneLine("n\tx\ty\tz\tr\tvact\tvext\ts\tid\tnear\tdist\tvpoly\tedgeToEdge");
		// write data
		for (i=0; i <= numXls - 1; i++) {
			thisXl = theXls->GetItemPtr(i);
			sprintf(tempStr, "%d\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%.7E\t%d\t%ld\t%d\t%.7E\t%.7E\t%.7E",
					i,
					thisXl->ctr.x,
					thisXl->ctr.y,
					thisXl->ctr.z,
					thisXl->r,
					thisXl->actV,
					thisXl->extV, 
					thisXl->ctrSlice,
					thisXl->ctrID,
					thisXl->neighbor,
					thisXl->ctcDist,
					thisXl->polyV, 
					thisXl->diffV);
			saveFile.putOneLine(tempStr);
		}
		saveFile.putOneLine("============ End Crystal Dataset ============");
	}
}

// ---------------------------------------------------------------------------
//		• WritePrefsSettings
// ---------------------------------------------------------------------------
/* The NSUserDefaults has a bunch of irrelevant settings in addition to those
	we care about.  We'll use the keys (but not the objects) from our factory
	defaults dictionary to get the important keys instead. */
void
Calculator::WritePrefsSettings(stringFile *saveFile)
{
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSDictionary *appDefaults = (NSDictionary *)[(AppController *)controller getDefaultValues];
	NSEnumerator *enumerator = [appDefaults keyEnumerator];

	id key;
	
	while ((key = [enumerator nextObject])) {
		NSObject *thisObj = [defaults objectForKey:key];
		saveFile->putOneLine((CFStringRef) [NSString stringWithFormat:@"%@: %@", key, thisObj]);
	}
}

// ---------------------------------------------------------------------------
//		• GoodCFVal
// ---------------------------------------------------------------------------
bool
Calculator::GoodCFVal(bool inoutputSigmas, double inVal)
{
	if (prefs->outputSigmas) {
		if (inVal == 0)	// neither the mean nor the standard deviation will be exactly zero
						// if the data were good
			return false;
	} else {
		if (inVal == -(DBL_MAX-1) || inVal == DBL_MAX || isnan(inVal))
			return false;
	}
	return true;
}


// ---------------------------------------------------------------------------
//		• FlaggedCFVal
// ---------------------------------------------------------------------------
double
Calculator::FlaggedCFVal(bool inoutputSigmas, double inVal)
{
	if (inVal == -(DBL_MAX-1) || inVal == DBL_MAX || isnan(inVal)) {
		return NAN;
	}
	return inVal;
}

// ---------------------------------------------------------------------------
//		• FlaggedCFEnvVal
// ---------------------------------------------------------------------------
double
Calculator::FlaggedCFEnvVal(bool inoutputSigmas, double inVal)
{
	if (inoutputSigmas) {
		if (inVal == 0)	// neither the mean nor the standard deviation will be exactly zero
			// if the data were good
			return NAN;
	} else {
		if (inVal == -(DBL_MAX-1) || inVal == DBL_MAX || isnan(inVal))
			return NAN;
	}
	return inVal;
}

