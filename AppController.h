/* AppController */

#import <Cocoa/Cocoa.h>
#import "ReduceTypes.h"

@class ProgressController;
@class Mediator;

@interface AppController : NSObject
{
    IBOutlet NSWindow *mainWindow;
	IBOutlet NSTextField *holesText;
	IBOutlet NSTextField *shaveText;
	IBOutlet NSButton *beginButton;
	IBOutlet NSButton *shaveButton;
	IBOutlet NSPopUpButton *xyzButton;
	IBOutlet NSTextField *maxAspectRatioTextField;
	IBOutlet NSTextField *maxAspectRatioLabel;
	NSString *holesFile;
	ProgressController *theProgressCtl;
    NSConnection *connectionToMediator;
	Mediator *mediator;	// This will actually be a proxy for the Mediator running on a different thread.
	bool userHasCanceled;
	bool mFilesAreFromAE;
	NSArray *mFilesToOpen;
	short mRunType;
}

- (id)init;
- (void)dealloc;
- (void)awakeFromNib;
- (void)showSettingsWindow;
- (void)chooseFilesToProcess;
- (IBAction)restoreDefaults:(id)sender;
- (IBAction)beginAnalysis:(id)sender;
- (IBAction)loadHolesFile:(id)sender;
- (IBAction)removeHolesFile:(id)sender;
- (IBAction)adjustShaveUISettings:(id)sender;
- (PrefStruct *)getPrefStruct;
- (id)getDefaultValues;
- (void) runAnalysis;
- (void)setMediatorDOServer:(id)serverObject;
- (oneway void)setProgMessage:(NSString *) inMessage;
- (oneway void)setProgTitle:(NSString *) inMessage;
- (oneway void)setShaveMessage:(NSString *) inEnvMessage;
- (oneway void)setEnvMessage:(NSString *) inEnvMessage;
- (oneway void)setupProgress:(NSString *) inMainMessage
				  shaveMessage:(NSString *) inShaveMessage
				  envMessage:(NSString *) inEnvMessage
					   title:(NSString *) inTitle
				  increments:(short) inInc
						 min:(double) inMin
						 max:(double) inMax
						 cur:(double) inCur
			   indeterminate:(bool) inInd;	// this is what the DO Mediator calls to setup the progress window
- (oneway void)progress:(double) inCur;	// this is what the DO Mediator calls to report its progress
- (short)postError:(NSString *) inMessage
 			 title:(NSString *) inTitle
		   buttons:(NSArray *) inButtons
	 defaultButton:(short) inDefault
		 dismissIn:(float) inDismiss;
- (oneway void)runEnded:(bool)goodRun;
- (bool)shouldStopCalculating;
- (NSArray *)getInputFiles;
- (NSString *)getHolesFile;

// NSApplication delegate methods
- (void)application:(NSApplication *)sender openFiles:(NSArray *)filenames;

// NSOpenPanel delegate method
- (void)openPanelDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode  contextInfo:(void  *)contextInfo;

@end
