// ===========================================================================
//	EnvelopeData.h
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================

#pragma once
#import <stddef.h>

#define ThrowIfNil_(var); if (var == NULL) throw("memory error");


enum CFTypes {LKind, LgdKind, PCFKind, PCFgdKind, MCFKind, MCF3Kind, MCFgdKind, MCF3gdKind, OIKind, IIKind, QSMKind, CQSMKind, RPSMKind,
				ARKind, MinSepKind, MeanSepKind, MNNRKind, MEVKind};

class OneSet {	// holds the data for the CF's measured from one simulation
	public:
		double	*L;
		double	*Lgd;
		double	*PCF;
		double	*PCFgd;
		double	*MCF;
		double	*MCF3;
		double	*MCFgd;
		double	*MCF3gd;
		double		orderingIndex;
		double		isolationIndex;
		double		quadratStatMean;
		double		circQuadratStatMean;
		double		RPStatMean;
		double		avramiRatio;
		double		minSep;
		double		meanSep;
		double		meanNNRad;
		double		meanExtV;
};

class EnvelopeData
{
	public:
	// Public Enums and Constants
		
	// Public Functions
		//Default Constructor
							EnvelopeData(short inEnvelopes, short inHPoints, bool inDoSigmas);
		//Destructor
							~EnvelopeData();
		double				*GetArray(short inEnvNum, short inKind);
		inline OneSet		*GetSet(short inEnvNum) {return &(mRuns[inEnvNum]);};
		void				Clear();
		void				ClearFirstCFs();
		void				GetMeanAndSD(short inHNum, short inKind,
										 double &outMean, double &outSD);
		void				GetMinAndMax(short inHNum, short inKind, short inConfidence,
										 double &outMean, double &outSD);
		inline double		GetVal(short EnvNum, short inKind, short inHNum);

	private:
		// Member Variables and Classes
		OneSet		*mRuns;
		short		mNumEnv;
		short		mNumH;
		bool		mDoSigmas;
	
};
