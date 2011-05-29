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
	bool		tidyUp;
	bool		useVolume;
	bool		matchVF;
	bool		makeDCEnv;
	bool		verbose;
	bool		playSoundWhenDone;
	bool		matchPDF;
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
	long		seed;
	long		MCReps;
	float		maxValueMeanCSD;
	float		deltaLRegCSD;
	float		impingementMeanErr;
	float		impingementMaxErr;
	float		EpanecnikovCVal;
	float		crit1Factor;
	float		crit2Factor;
	float		shaveIncrement;
	float		minPercent;
	float		maxAspectRatio;
	float		VFPercent;
} PrefStruct;

typedef struct {
	short	halfSines;
	short	start;
	float	minProb;
} nuclProb1D;

typedef struct {
	nuclProb1D x;
	nuclProb1D y;
	nuclProb1D z;
} nuclProbType;

typedef struct DistArrayElem {
	short	index;
	float	dist;
} DistArrayElem;
