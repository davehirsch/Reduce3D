// ===========================================================================
//  ProgressController.m
//  Reduce3D
//
//  Created by David Hirsch on 11/25/07.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================


#import "ProgressController.h"
#import "ReduceConstants.h"

@implementation ProgressController

- (id) init
{
	self = [super initWithWindowNibName:@"Progress"];
	mNumIncrements = kDefaultNumThermoIncrements;
	mLastTickValue = 0;
	mStartTime = nil;
	return self;
}

- (void)windowDidLoad
{
	NSLog(@"Progress Nib is loaded.");
}

// Not used at this time
- (IBAction)pause:(id)sender
{
	if ([[pauseButton title] isEqualToString:@"Pause"]) {
		NSLog(@"Pause calculation.");
		[timeLeft setStringValue:@"[Paused]"];
		[pauseButton setTitle:@"Resume"];
		[stopButton setHidden:FALSE];
		int result = [NSApp runModalForWindow:[self window]];
		NSLog(@"Modal result:%i", result);
	} else {
		NSLog(@"Resume calculation.");
		[timeLeft setStringValue:@"[Paused]"];
		[pauseButton setTitle:@"Pause"];
		[stopButton setHidden:TRUE];
		[NSApp stopModalWithCode:1001];
	}
}

- (IBAction)stop:(id)sender
{
	NSLog(@"Stop calculation.");
	NSAlert *alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"Stop the Calculation"];
	[alert addButtonWithTitle:@"Continue"];
	[alert setMessageText:@"Stop the run?"];
	[alert setInformativeText:@"Are you sure? You'll have to start over."];
	[alert setAlertStyle:NSWarningAlertStyle];
	if ([alert runModal] == NSAlertFirstButtonReturn) {
		// OK clicked, notify listeners of user's cancellation
		[[NSNotificationCenter defaultCenter] postNotificationName:@"UserCanceled" object:self];
	}
	[alert release];
}

- (void)showMyWindow
{
	[self showWindow:nil];
}

- (void)hideWindow
{
	[[self window] orderOut:self];
}

- (void)setEnvelopeMessage:(NSString*) inMessage
{
	NSLog(@"Current Envelope String is:%@; Setting the Message to %@", [message stringValue], inMessage);
	[envelopeText setStringValue:inMessage];
}

- (void)setShaveMessage:(NSString*) inMessage
{
	NSLog(@"Current Shave String is:%@; Setting the Message to %@", [message stringValue], inMessage);
	[shaveText setStringValue:inMessage];
}


- (void)setProgMessage:(NSString*) inMessage
{
	NSLog(@"Current String is:%@; Setting the Message to %@", [message stringValue], inMessage);
	[message setStringValue:inMessage];
}

- (void)setProgMessageWithCString:(char *) inMessage
{
	[self setProgMessage:[NSString stringWithCString:inMessage encoding:NSUTF8StringEncoding]];
}

- (void)setProgTitle:(NSString *) inStr
{
	NSLog(@"Current Title is:%@; Setting the Title to %@", [[self window] title], inStr);
	[[self window]	setTitle:inStr];
}

- (void)setProgTitleWithCString:(char *) inStr
{
	[self setProgTitle:[NSString stringWithCString:inStr encoding:NSUTF8StringEncoding]];
}

- (void)setProgMinValue:(double) inVal
{
	[progBar setMinValue:inVal];
	mLastTickValue = inVal;
}

- (void)setProgMaxValue:(double) inVal
{
	[progBar setMaxValue:inVal];
}

- (void)setIndeterminate:(bool)inVal
{
	[progBar setIndeterminate:inVal];
}

- (void)setIncrements:(short)inVal
{
	mNumIncrements = inVal;
}

- (void)setProgMin:(double)inMin max:(double)inMax cur:(double)inCur
{
	if (inMin == 0 && inMax == 0) {
		[progBar setIndeterminate:TRUE];
		[timeLeftPrompt setHidden:TRUE];
		[timeLeft setHidden:TRUE];
		if (mStartTime) {
			[mStartTime release];
			mStartTime = nil;
		}
//		[progBar startAnimation:self];	// Oddly, this seems to kill the barber pole
	} else {
		[progBar setIndeterminate:FALSE];
		[timeLeftPrompt setHidden:FALSE];
		[timeLeft setHidden:FALSE];
		[self setProgMinValue:inMin];
		[self setProgMaxValue:inMax];
		[timeLeft setStringValue:@"unknown"];
		mLastTickValue = inMin;
		[progBar setDoubleValue:inCur];
		if (mStartTime) {
			[mStartTime release];
			mStartTime = nil;
		}
	}
	[[self window] displayIfNeeded];
	[NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow: kSleepTimeForProgressUpdate]];
}

- (void)thisMuchCompleted:(double)inCur
{
	double width = [progBar maxValue] - [progBar minValue];
	double incSize = width / mNumIncrements;
	if (mStartTime == nil) {
		// set base time for Rate
		mStartTime = [NSDate date];
		[mStartTime retain];
	}
	if (inCur > (mLastTickValue + incSize)) {
		[progBar setDoubleValue:inCur];		// happens mNumIncrements times
		mLastTickValue += incSize;
		// update time estimate text
		double fractionCompleted = (inCur - [progBar minValue]) / width;
		double timeSoFar = [[NSDate date] timeIntervalSinceDate:mStartTime]; // in seconds
		double totalTime = timeSoFar / fractionCompleted;
		double secsToFinish = totalTime - timeSoFar;	// in seconds
		short	timeToFinish_hrs = secsToFinish / 3600.0;
		short	timeToFinish_mins = fmod((double_t)(secsToFinish), (double_t)(3600.0)) / 60;
		short	timeToFinish_secs = secsToFinish - 60*timeToFinish_mins - 3600*timeToFinish_hrs;
		[timeLeft setStringValue:[NSString stringWithFormat:@"%02hi:%02hi:%02hi", timeToFinish_hrs, timeToFinish_mins, timeToFinish_secs]];
		[[self window] displayIfNeeded];
		[NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow: kSleepTimeForProgressUpdate]];
	}
}

- (void)spinPattern
{
/*	if (mStartTime == nil) {
		// set base time for Rate
		mStartTime = [NSDate date];
		[mStartTime retain];
	} else if ([[NSDate date] timeIntervalSinceDate:mStartTime] > kTimeIntervalBetweenProgressSpins) {
		[progBar animate:self];
		[[self window] displayIfNeeded];
		[NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow: kSleepTimeForProgressUpdate]];
		[mStartTime release];
		mStartTime = [NSDate date];
		[mStartTime retain];
	}
*/
}

- (bool)isIndeterminate
{
	return [progBar isIndeterminate];
}
@end
