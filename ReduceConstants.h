// ===========================================================================
//	Reduce 3D Constants.h				
// ===========================================================================

#define	popup_RectPrism 0
#define	popup_Cylinder	1
#define	popup_Sides	2
#define kUserCanceledErr 101
#define kProcessingError 102

const short		menuitem_Smaller		= 0;
const short		menuitem_Larger			= 1;

// Bounding Box Constants
			// this is as a fraction of the rough averaged dimension; it is compared
			// to a dot product
//const double	kDiscardPointsThreshold	= 0.001;	// smaller is more conservative
const double	kCoplanarThreshold		= 0.000001;	// this is a theta value
const bool	kFinished				= true;
const bool	kUnfinished				= false;
const short		kPerspective			= 1;		/* 10 is strong perspective view */
const float	kScale					= 6000;	/* scaling for 3DBB plot */
//const float	kScale					= 6000/kPerspective;	/* scaling for 3DBB plot */
const short		kPicSize				= 300;		/* approximate desired pic size, in pixels, for the BB */

enum Models {kNone, kBBox, kXls, kHoles};

// Misc constants
const short		kStdStringSize			= 500;
const short		kDefaultNumThermoIncrements = 50;
const long		kNumTriesToPlaceXl = 10000;
const short		kNumSimMakerRestarts = 6;
const short		kNumOctantPoints = 25;	// number of points to use (*16) for BoundingBox::PrimitiveInBox
const short		kRoughMCReductionFactor = 10;
const float		kTimeIntervalBetweenProgressSpins = 0.05;
const float		kSleepTimeForProgressUpdate = 0.01;
const long		myOptionKey = 0x00000004;
const long		myShiftKey = 0x00000001;

// Statistics constants
const short		kWeightFrequency		= 1;		// pre-calculates every kWeightFrequency-th value
enum boxTypes {kCubeBox, kSidesBox, kRPBox, kCylBox};
enum quadTypes {kCube, kSphere};
enum runTypes {kSingle, kBatch};
enum openPanelTypes {kPrimaryInput, kHolesInput};
enum CSDTypes {kMeanCumCSD, kMaxCumCSD, kLogCSD, kNNCSD, kRegCSD, kRedStats, kReduce, kIntegrate, kEnvelope};
enum fileTypes {kUndefined = 0, kReal = 0x01, kDiffSimulation = 0x02 | 0x04, kIntSimulation = 0x02 | 0x08, kHFSimulation = 0x02 | 0x10};
#define isSim(A) ((A) & 0x02)

const short		kMaxNumBins				= 500;	// max number of bins for the quadrat test
const short		kNumBinsNNCSD			= 50;
const short		kNumBinsRegCSD			= 50;
enum		primitiveTypes {kRectPrism = 0, kCylinder = 1, kSides = 2};
enum		criteriaRelations {kSmaller = 0, kLarger = 1};
enum		directions {kFromMax = 0, kSymmetric = 1, kFromMin = 2};
enum		xyzCoordinates {kX = 0, kY = 1, kZ = 2};
enum		boundsTypes {kBoundsNone = 0, kBoundsRP = 1, kBoundsCyl = 2};

const short kArrayIncrement = 10;

// Macros
#define REDUCELOG(A) if (prefs->verbose) mLogFile->putOneLine((A))
