// ===========================================================================
//	ReduceConstants.h				
//  Reduce3D
//
//  Created by David Hirsch on 12/1/02.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================

#define	popup_Sides	0
#define	popup_RectPrism 1
#define	popup_Cylinder	2
#define kUserCanceledErr 101
#define kProcessingError 102

const short		menuitem_Smaller		= 0;
const short		menuitem_Larger			= 1;

// Bounding Box Constants
			// this is as a fraction of the rough averaged dimension; it is compared
			// to a dot product
//const double	kDiscardPointsThreshold	= 0.001;	// smaller is more conservative
const double	kCoplanarThreshold		= 0.0001;	// this is a theta value
const bool	kFinished				= true;
const bool	kUnfinished				= false;
const short		kPerspective			= 1;		/* 10 is strong perspective view */
const double	kScale					= 6000;	/* scaling for 3DBB plot */
//const double	kScale					= 6000/kPerspective;	/* scaling for 3DBB plot */
const short		kPicSize				= 300;		/* approximate desired pic size, in pixels, for the BB */

enum Models {kNone, kBBox, kXls, kHoles};

// Misc constants
const short		kStdStringSize			= 500;
const short		kNumberOfProgressCalls = 10;	// how many times in a loop the progress call should be made.  Can't 
												// offload this to the progressController itself, because calls to it must 
												// cross thread boundaries and thus are themselves slow
const long		kNumTriesToPlaceXl = 10000;
const short		kNumSimMakerRestarts = 6;
const short		kNumOctantPoints = 25;	// number of points to use (*16) for BoundingBox::PrimitiveInBox
const short		kRoughMCReductionFactor = 10;
const double		kTimeIntervalBetweenProgressSpins = 0.05;
const double		kSleepTimeForProgressUpdate = 0.01;
const long		myOptionKey = 0x00000004;
const long		myShiftKey = 0x00000001;

// Statistics constants
const short		kWeightFrequency		= 1;		// pre-calculates every kWeightFrequency-th value
enum boxTypes {kCubeBox, kSidesBox, kRPBox, kCylBox};
enum quadTypes {kCube, kSphere};
enum runTypes {kSingle, kBatch};
enum openPanelTypes {kPrimaryInput, kHolesInput};
enum CSDTypes {kMeanCumCSD, kMaxCumCSD, kLogCSD, kNNCSD, kRegCSD, kRedStats, kReduce, kIntegrate, kEnvelope};
enum fileTypes {kUndefined = 0, kReal = 0x20, kDiffSimulation = 0x02 | 0x04, kIntSimulation = 0x02 | 0x08, kHFSimulation = 0x02 | 0x10, kNewCrystallizeSimulation = 0x02 | 0x20};
#define isSim(A) ((A) & 0x02)
#define hasExtendedVolume(A) ((A) & 0x20)

const short		kMaxNumBins				= 500;	// max number of bins for the quadrat test
const short		kNumBinsNNCSD			= 50;
const short		kNumBinsRegCSD			= 50;
enum		primitiveTypes {kSides = 0, kRectPrism = 1, kCylinder = 2};
enum obvservabilityMethods {kSetDirectly = 0, kSetFromData = 1};
enum		criteriaRelations {kSmaller = 0, kLarger = 1};
enum		directions {kFromMax = 0, kSymmetric = 1, kFromMin = 2};
enum		xyzCoordinates {kX = 0, kY = 1, kZ = 2};
enum		boundsTypes {kBoundsNone = 0, kBoundsRP = 1, kBoundsCyl = 2};

const short kArrayIncrement = 10;

// Macros
#define REDUCELOG(A) if (prefs->verbose) mLogFile->putOneLine((A))
