//
//  ProgressController.h
//  Reduce3D
//
//  Created by David Hirsch on 11/25/07.
//  Copyright 2007 David Hirsch. All rights reserved.
//

#import <Cocoa/Cocoa.h>

enum PauseButtonStates {kResume, kPause};
const short		kDefaultNumThermoIncrements = 50;	// number of increments in progress bar

@interface ProgressController : NSWindowController {
	IBOutlet NSButton		*pauseButton;
	IBOutlet NSButton		*stopButton;
	IBOutlet NSTextField	*shaveText;
	IBOutlet NSTextField	*envelopeText;
	IBOutlet NSTextField	*message;
	IBOutlet NSTextField	*timeLeft;
	IBOutlet NSTextField	*timeLeftPrompt;
	IBOutlet NSProgressIndicator	*progBar;
	short	mNumIncrements;
	double	mLastTickValue;
	NSDate *mStartTime;
	bool mRateBasetimeSet;
}

- (IBAction)pause:(id)sender;
- (IBAction)stop:(id)sender;
- (void)showMyWindow;
- (void)hideWindow;
- (void)setEnvelopeMessage:(NSString *) inMessage;
- (void)setShaveMessage:(NSString*) inMessage;
- (void)setProgMessage:(NSString *) inMessage;
- (void)setProgMessageWithCString:(char *) inMessage;
- (void)setProgTitle:(NSString *) inStr;
- (void)setProgTitleWithCString:(char *) inStr;
- (void)setProgMinValue:(double) inVal;
- (void)setProgMaxValue:(double) inVal;
- (void)setProgMin:(double)inMin max:(double)inMax cur:(double)inCur;
- (void)thisMuchCompleted:(double) inCur;
- (void)spinPattern;
- (void)setIncrements:(short)inVal;
- (void)setIndeterminate:(bool)inVal;
- (bool)isIndeterminate;

@end
