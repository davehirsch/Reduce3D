#import "AppController.h"
#import "ReduceConstants.h"
#import "ProgressController.h"
#import "Mediator.h"

@implementation AppController

//-------------------------------------------------------------
// init
//
// Initializes this object, sets up the connection to the thread in which
//	the calculation is done.
//-------------------------------------------------------------
- (id)init
{
    NSPort *port1;
    NSPort *port2;
    NSArray *portArray;
	
	self = [super init];
    if (self != nil) {
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		NSDictionary *appDefaults = [self getDefaultValues];
		[defaults registerDefaults:appDefaults];
		holesFile = nil;
		theProgressCtl = nil;
		mFilesAreFromAE = false;

		// Set up a Distributed Objects system for communicating with the Calculation
        port1 = [NSPort port];
        port2 = [NSPort port];
		
		connectionToMediator = [[NSConnection alloc] initWithReceivePort:port1 sendPort:port2];
		[connectionToMediator setRootObject:self];
		
        // Initialise the calculator to nil.  Note that there's a concurrency
        // hole in this implementation.  calc is nil when we leave this method,
        // and stays nil until Mediator's connectWithPorts: runs in its thread
        // and sends us a setServer: message to set it.  But we leave this routine without
        // waiting for that return message.  If the new thread is unexpectedly
        // slowed down, it might be possible for someone to call our action methods
        // (eg RunAnalysis:) before transferServer has been set up.  Of course,
        // this is *extremely* unlikely (because our action methods are only invoked
        // as a result of user action) but it's worth noting.  
		mediator = nil;

		// Ports switched here.
        portArray = [NSArray arrayWithObjects:port2, port1, nil];
        [NSThread detachNewThreadSelector:@selector(connectWithPorts:)
								 toTarget:[Mediator class]
							   withObject:portArray];
		[NSApp setDelegate:self];
	}
	return self;
}

//-------------------------------------------------------------
// dealloc
//
// Standard dealloc
//-------------------------------------------------------------
- (void)dealloc
{	
	[mFilesToOpen release];
	[theProgressCtl release];
	[mediator release];
	[super dealloc];
}

//-------------------------------------------------------------
// awakeFromNib
//
// Displays the settings window and adds us as an observer so we can notice when
//	the user hits cancel on the progress window.
//-------------------------------------------------------------
- (void)awakeFromNib
{
	[self showSettingsWindow];
	[self adjustShaveUISettings:[NSNull null]];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(userCanceledRun:)
												 name:@"UserCanceled" object:nil];
}

//=========================================//
#pragma mark     MANAGING THE FILES TO OPEN
//=========================================//

//-------------------------------------------------------------
// application:openFiles:
//
// Called as a result of the Open Apple Event, which delivers an array of pathnames
//
// Note that in the previous (CodeWarrior) version of Reduce, the Reduce output 
//	files had a resource in their resource fork that contained all the settings of 
//	the settings window used in the file's creation.  That is not currently implemented, 
//	since I don't want to deal with resource forks or, alternately, bundles.  One 
//	possibility would be to append an XML dictionary to the output file, or to (optionally) 
//	save a separate file that contains the processing parameters, also perhaps in XML format.
//-------------------------------------------------------------
- (void)application:(NSApplication *)sender openFiles:(NSArray *)filenames
{
	// Look for files dropped on the icon - in an "Open" Apple Event
		// Files that Reduce saves may(?) have a Settings Dictionary that we can parse to use as the current user defaults
		// (This is not implemented now, but was in the previous version of Reduce)

	NSLog(@"Got these files:%@", filenames);
	// record the list of files to open
	mFilesToOpen = filenames;
	[mFilesToOpen retain];	// hang onto this for later
	mFilesAreFromAE = true;
	
	if ([mFilesToOpen count] > 1)
		mRunType = kBatch;
		// DMH: we really ought to check each file to be sure its valid, but given the 
		// file extension restrictions on apple events, it's likely okay.  Also, the
		// method that reads the data will have check in it.
	
	[self showSettingsWindow];	// the user still needs to set/check the run settings
}

//-------------------------------------------------------------
// chooseFilesToProcess
//
// This creates and runs an open panel as a sheet, allowing the user to choose
//	one or more input files.  It sets up openPanelDidEnd as a delegate (callback).
//	processing will resume there.
//-------------------------------------------------------------
- (void)chooseFilesToProcess
{
	short *context = (short *)malloc(sizeof(short));
	*context = kPrimaryInput;
	NSArray *fileTypes = [NSArray arrayWithObjects:@"txt", @"text", @"Int", NSFileTypeForHFSTypeCode( 'TEXT' ), nil];
	
	NSOpenPanel *oPanel = [NSOpenPanel openPanel];
	
	[oPanel setAllowsMultipleSelection:YES];
	[oPanel setTitle:@"Choose Integrate Files"];
	[oPanel setMessage:@"Choose an integrate file containing crystals to be analysed."];
	[oPanel setDelegate:self];
	[oPanel beginSheetForDirectory:nil
							  file:nil
							 types:fileTypes 
					modalForWindow:mainWindow 
					 modalDelegate:self
					didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
					   contextInfo:(void *)context];	
}

//-------------------------------------------------------------
// loadHolesFile:
//
// Asks the user to locate a holes file, an integrate file to be interpreted as
//	a set of subvolumes to be excluded from further analysis. This creates and 
//	runs an open panel as a sheet, allowing the user to choose
//	one holes file.  It sets up openPanelDidEnd as a delegate (callback).
//	processing will resume there.
//-------------------------------------------------------------
- (IBAction)loadHolesFile:(id)sender
{
	short *context = (short *)malloc(sizeof(short));
	*context = kHolesInput;
	
	NSArray *fileTypes = [NSArray arrayWithObjects:@"txt", @"text", @"Int", NSFileTypeForHFSTypeCode( 'TEXT' ), nil];
	
	NSOpenPanel *oPanel = [NSOpenPanel openPanel];
	
	[oPanel setAllowsMultipleSelection:NO];
	[oPanel setTitle:@"Choose Holes File"];
	[oPanel setMessage:@"Choose an integrate file containing regions to be omitted from calculation."];
	[oPanel setDelegate:self];
	[oPanel beginSheetForDirectory:nil
							  file:nil
							 types:fileTypes 
					modalForWindow:mainWindow 
					 modalDelegate:self
					didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
					   contextInfo:(void *)context];	
}

//-------------------------------------------------------------
// openPanelDidEnd:returnCode:contextInfo:
//
// This is called when the choose files panel completes.  Further processing
//	occurs depending on whether we were looking for input files or a holes file.
//-------------------------------------------------------------
- (void)openPanelDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode  contextInfo:(void  *)contextInfo
{
	short *context = (short *)contextInfo;
	if (returnCode == NSOKButton) {
		if (*context == kPrimaryInput) {
			mFilesToOpen = [panel filenames];
			[mFilesToOpen retain];
			[self runAnalysis];
		} else if (*context == kHolesInput) {
			holesFile = [[panel filenames] objectAtIndex:0];
			[holesText setStringValue:holesFile];
		}
	}
	free(context);	// context was malloced in one of the two panel setup methods. Both end up here eventually.
}

//=========================================//
#pragma mark     EMBARKATION METHODS
//=========================================//

//-------------------------------------------------------------
// beginAnalysis:
//
// This is called when the user clicks the Begin Analysis button.  Depending on whether 
//	we've already received files through an AppleEvent, we either choose input file(s)
//	or we start the calculation.  If we need to choose files, then the calculation
//	will be initiated in the modal delegate (openPanelDidEnd).
//-------------------------------------------------------------
- (IBAction)beginAnalysis:(id)sender
{	
	// If no files were dropped, then ask the user to choose one or more files to process
	if (!mFilesAreFromAE) {
		[self chooseFilesToProcess];	
	} else {
		[self runAnalysis];
	}
}
	
//-------------------------------------------------------------
// runAnalysis
//
// Sets up for the actual analysis, which happens in the Mediator object.
//-------------------------------------------------------------
- (void) runAnalysis
{
	// Check for a holes file - batch mode collision
	if (holesFile != nil && [mFilesToOpen count] > 1) {
		[self postError:@"You cannot have a holes file selected during batch processing."
				  title:nil
				buttons:nil
		  defaultButton:-1
			  dismissIn:0];
	} else {
			
		userHasCanceled = false;
		NSLog(@"Running an analysis now.");
		// hide settings window
		[mainWindow orderOut:self];
		
		// create progress window
		theProgressCtl = [ProgressController new];
		[theProgressCtl loadWindow]; 

		// Tell the Mediator object to run the analysis.  It can call back to this object to get NSUserDefaults settings
		[mediator runAnalysis:self];
	}
}

//=========================================//
#pragma mark     INTER-THREAD DISCUSSION METHODS
//=========================================//

//-------------------------------------------------------------
// setCalcDOServer:
//
// This is called by the Mediator object on the spawned thread to connect us to
//	the Mediator object (well actually a proxy for that object due to the nature
//	of the Distributed Objects system).
//-------------------------------------------------------------
- (void)setMediatorDOServer:(id)serverObject
{
    // The following line is an interesting optimisation.  We tell our proxy
    // to the Mediator object to about the methods that we're going to
    // send to the proxy.  This optimises Distributed Object's delivery of
    // messages.  [Normally when DO encounters a new method, it must first
    // conduct a transaction with the remote end to find the types for the
    // arguments of that message.  It then bundles up the method and its
    // parameters and sends it.  It also caches the response so that following
    // invokations of that method only take one transaction.  By setting
    // a protocol for the proxy, you let DO know about the messages in
    // advance, and avoid it ever having to do two transactions.]
    [serverObject setProtocolForProxy:@protocol(MediatorServerInterface)];
	
    // We now retain the server object.  Mediator's connectWithPorts:
    // method is about to release it, so we must make sure that someone
    // has a reference!  This also means that we're the *only* person with
    // a reference.
    [serverObject retain];
	
	/*	// Now record the remote server object (actually its proxy) in our instance
		// variable.  Note that "(id <MediatorServerInterface>)" is a form
		// of type casting.  It says that serverObject conforms to the
		// MediatorServerInterface protocol.
		*/
    mediator = (id <MediatorServerInterface>)serverObject;
	
	// Now that we have a valid Mediator object, we can enable the Begin Analysis button.
	[beginButton setEnabled:TRUE];
}

//-------------------------------------------------------------
// runEnded
//
// The Mediator calls this to notify us that it is finished, either due to completion or cancellation.
//-------------------------------------------------------------
- (oneway void)runEnded:(bool)goodRun
{
	if (userHasCanceled || !goodRun) {
		[theProgressCtl close];
		[self showSettingsWindow];
	} else {
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		[self setupProgress:@"Calculations complete!"
			   shaveMessage:@" "
				 envMessage:@" "
					  title:@"Reduce3D Done"
				 increments:-1
						min:0
						max:1
						cur:1
			  indeterminate:false]; 
		if ([defaults boolForKey:@"playSoundWhenDone"]) {
			NSSound *chord = [NSSound soundNamed:@"ChordProgression.aiff"];
			[chord play];
			for (;[chord isPlaying];);	// wait until chord completes;
		}
		[NSApp terminate:self];	// Quit; we're done
	}
}

//-------------------------------------------------------------
// userCanceledRun:
//
// Upon hitting Cancel in the Progress Window, a notification gets posted, and this method gets called as a result.
// We set our userHasCanceled flag, so that the Mediator can query it in the shouldStopCalculating method
//-------------------------------------------------------------
- (void) userCanceledRun:(NSNotification *)notification
{
	NSLog(@"userCanceledRun now.");
	userHasCanceled = true;
}

//-------------------------------------------------------------
// shouldStopCalculating
//
// The Mediator calls this periodically to check if the user has canceled or not.
// Note that this function blocks until the return message is created and sent, which entails significant overhead.
// Therefore, calling this should happen infrequently (but not so rarely that the cancel button is useless.
//-------------------------------------------------------------
- (bool)shouldStopCalculating
{
	return userHasCanceled;
}


//-------------------------------------------------------------
// getInputFiles
//
// The Mediator calls this to get the array of input files.
// Note that this function blocks until the return message is created and sent, which entails significant overhead.
//-------------------------------------------------------------
- (NSArray *)getInputFiles
{
	return mFilesToOpen;
}

//-------------------------------------------------------------
// getHolesFile
//
// The Mediator calls this to get the path to the holes file (nil if non selected).
// Note that this function blocks until the return message is created and sent, which entails significant overhead.
//-------------------------------------------------------------
- (NSString *)getHolesFile
{
	return holesFile;
}

//-------------------------------------------------------------
// getPrefStruct
//
// The Mediator calls this to get the set of preferences.  
// Note that this function blocks until the return message is created and sent, which entails significant overhead.
//-------------------------------------------------------------
- (PrefStruct *)getPrefStruct
{
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	static PrefStruct thePrefs;
	thePrefs.includeMeanCSD = [defaults boolForKey:@"includeMeanCSD"];
	thePrefs.numClassesMeanCSD = [defaults integerForKey:@"numClassesMeanCSD"];
	thePrefs.maxValueMeanCSD = [defaults floatForKey:@"maxValueMeanCSD"];
	thePrefs.includeLogCSD = [defaults boolForKey:@"includeLogCSD"];
	thePrefs.numClassesLogCSD = [defaults integerForKey:@"numClassesLogCSD"];
	thePrefs.includeMaxCSD = [defaults boolForKey:@"includeMaxCSD"];
	thePrefs.numClassesMaxCSD = [defaults integerForKey:@"numClassesMaxCSD"];
	thePrefs.includeRegCSD = [defaults boolForKey:@"includeRegCSD"];
	thePrefs.deltaLRegCSD = [defaults floatForKey:@"deltaLRegCSD"];

	thePrefs.doQuadrat = [defaults boolForKey:@"doQuadrat"];
	thePrefs.quadratNumPlacings = [defaults integerForKey:@"quadratNumPlacings"];
	thePrefs.quadratNumReps = [defaults integerForKey:@"quadratNumReps"];
	thePrefs.doImpingement = [defaults boolForKey:@"doImpingement"];
	thePrefs.impingementMeanErr = [defaults floatForKey:@"impingementMeanErr"];
	thePrefs.impingementMaxErr = [defaults floatForKey:@"impingementMaxErr"];
	thePrefs.doOthers = [defaults boolForKey:@"doOthers"];
	thePrefs.inflateBBox = [defaults boolForKey:@"inflateBBox"];
	thePrefs.discardNegs = [defaults boolForKey:@"discardNegs"];
	thePrefs.useRaeburn = [defaults boolForKey:@"useRaeburn"];
	thePrefs.useVolume = [defaults boolForKey:@"useVolume"];
	thePrefs.doRandomPt = [defaults boolForKey:@"doRandomPt"];
	thePrefs.RPNumPlacings = [defaults integerForKey:@"RPNumPlacings"];
	thePrefs.RPNumReps = [defaults integerForKey:@"RPNumReps"];
	thePrefs.doLMcfPcf = [defaults boolForKey:@"doLMcfPcf"];
	thePrefs.numNNDist = [defaults integerForKey:@"numNNDist"];
	thePrefs.overlap = [defaults integerForKey:@"overlap"];
	thePrefs.numCFOffsetVolPts = [defaults integerForKey:@"numCFOffsetVolPts"];
	thePrefs.EpanecnikovCVal = [defaults floatForKey:@"EpanecnikovCVal"];
	thePrefs.sampleShape = [defaults integerForKey:@"sampleShape"];
	thePrefs.exscribedPrimitive = [defaults boolForKey:@"exscribedPrimitive"];
	thePrefs.shrinkExscribedPrimitive = [defaults floatForKey:@"shrinkExscribedPrimitive"];

	thePrefs.numEnvelopeRuns = [defaults integerForKey:@"numEnvelopeRuns"];
	thePrefs.matchVF = [defaults boolForKey:@"matchVF"];
	thePrefs.VFPercent = [defaults floatForKey:@"VFPercent"];
	thePrefs.matchPDF = [defaults boolForKey:@"matchPDF"];
	thePrefs.makeDCEnv = [defaults boolForKey:@"makeDCEnv"];
	thePrefs.observabilityFilter = [defaults boolForKey:@"observabilityFilter"];
	thePrefs.crit1Factor = [defaults floatForKey:@"crit1Factor"];
	thePrefs.crit2Factor = [defaults floatForKey:@"crit2Factor"];

	thePrefs.outputReduce = [defaults boolForKey:@"outputReduce"];
	thePrefs.outputCrystals = [defaults boolForKey:@"outputCrystals"];
	thePrefs.outputSigmas = [defaults boolForKey:@"outputSigmas"];
	thePrefs.confidence = [defaults integerForKey:@"confidence"];

	thePrefs.seed = [defaults integerForKey:@"seed"];
	thePrefs.MCReps = [defaults integerForKey:@"MCReps"];
	thePrefs.tidyUp = [defaults boolForKey:@"tidyUp"];
	thePrefs.verbose = [defaults boolForKey:@"verbose"];	
	thePrefs.playSoundWhenDone = [defaults boolForKey:@"playSoundWhenDone"];	

	thePrefs.doShave = [defaults boolForKey:@"doShave"];
	thePrefs.keepAspectRatios = [defaults boolForKey:@"keepAspectRatios"];
	thePrefs.shaveXYZ = [defaults integerForKey:@"shaveXYZ"];
	thePrefs.shaveIncrement = [defaults floatForKey:@"shaveIncrement"];
	thePrefs.direction = [defaults integerForKey:@"direction"];
	thePrefs.minPercent = [defaults floatForKey:@"minPercent"];
	thePrefs.minPopulation = [defaults integerForKey:@"minPopulation"];
	thePrefs.maxAspectRatio = [defaults floatForKey:@"maxAspectRatio"];
	thePrefs.shaveSmLg = [defaults integerForKey:@"shaveSmLg"];
	thePrefs.ShaveSave = [defaults boolForKey:@"ShaveSave"];

	return &(thePrefs);
}


//-------------------------------------------------------------
// setProgMessage:
//
// This is what the DO Mediator calls to set the main message of the progress window.  
// This just does some error checking and sends the request on to the actual ProgressController
//-------------------------------------------------------------
- (oneway void)setProgMessage:(NSString *) inMessage
{
	if (inMessage != nil)
		[theProgressCtl setProgMessage:inMessage]; 
}

//-------------------------------------------------------------
// setProgTitle:
//
// This is what the DO Mediator calls to set the title of the progress window.  
// This just does some error checking and sends the request on to the actual ProgressController
//-------------------------------------------------------------
- (oneway void)setProgTitle:(NSString *) inMessage
{
	if (inMessage != nil)
		[theProgressCtl setProgTitle:inMessage]; 
}

//-------------------------------------------------------------
// setEnvMessage:
//
// This is what the DO Mediator calls to set the Envelope message of the progress window.  
// This just does some error checking and sends the request on to the actual ProgressController
//-------------------------------------------------------------
- (oneway void)setEnvMessage:(NSString *) inEnvMessage
{
	if (inEnvMessage != nil)
		[theProgressCtl setEnvelopeMessage:inEnvMessage]; 
}

//-------------------------------------------------------------
// setShaveMessage:
//
// This is what the DO Mediator calls to set the Shave message of the progress window.  
// This just does some error checking and sends the request on to the actual ProgressController
//-------------------------------------------------------------
- (oneway void)setShaveMessage:(NSString *) inEnvMessage
{
	if (inEnvMessage != nil)
		[theProgressCtl setShaveMessage:inEnvMessage]; 
}

//-------------------------------------------------------------
// setupProgress:
//
// This is what the DO Mediator calls to setup the progress window.  
// This just does some error checking and sends the request on to the actual ProgressController
//-------------------------------------------------------------
- (oneway void)setupProgress:(NSString *) inMainMessage
				shaveMessage:(NSString *) inShaveMessage
				  envMessage:(NSString *) inEnvMessage
					   title:(NSString *) inTitle
				  increments:(short) inInc
						 min:(double) inMin
						 max:(double) inMax
						 cur:(double) inCur
			   indeterminate:(bool) inInd
{
	NSLog(@"setting up progress window.");
	if (inMainMessage != nil)
		[theProgressCtl setProgMessage:inMainMessage]; 
	if (inShaveMessage != nil)
		[theProgressCtl setShaveMessage:inShaveMessage]; 
	if (inEnvMessage != nil)
		[theProgressCtl setEnvelopeMessage:inEnvMessage]; 
	if (inTitle != nil)
		[theProgressCtl setProgTitle:inTitle]; 
	if (inInc != -1)
		[theProgressCtl setIncrements:inInc];
	if (inMin != -1)
		[theProgressCtl setProgMin:inMin max:inMax cur:inCur];
	[theProgressCtl setIndeterminate:inInd];

	[theProgressCtl showWindow:nil];
}

//-------------------------------------------------------------
// progress:
//
// this is what the Distributed Objects Mediator calls to report its progress
//-------------------------------------------------------------
- (oneway void)progress:(double) inCur
{
	if ([theProgressCtl isIndeterminate]) {
		[theProgressCtl spinPattern];
	} else {
		[theProgressCtl thisMuchCompleted:inCur];
	}
}

//-------------------------------------------------------------
// postError:
//
// this is what the Distributed Objects Mediator calls to report an Error.
// This funtion blocks until response is supplied, but the caller can
// have the window auto-dismiss and the default option auto-selected. (not yet implemented)
// The buttons
//-------------------------------------------------------------
- (short)postError:(NSString *) inMessage
 			 title:(NSString *) inTitle
		   buttons:(NSArray *) inButtons
	 defaultButton:(short) inDefault
		 dismissIn:(float) inDismiss
{
	NSAlert *alert = [[NSAlert alloc] init];
	if (inButtons) {
		NSMutableArray *btnArray = [NSMutableArray array];
		for (short i = 0; i <= [inButtons count] - 1; i++) {
			[btnArray addObject: [alert addButtonWithTitle:[inButtons objectAtIndex:i]]];
		}
		if (inDefault != -1)
			[[btnArray objectAtIndex:inDefault] setKeyEquivalent:@"\r"];
	} else {
		[alert addButtonWithTitle:@"OK"];
		[alert addButtonWithTitle:@"Cancel"];
	}
	if (inTitle) {
		[alert setMessageText:inTitle];
	} else {
		[alert setMessageText:@"Reduce3D Error"];
	}
	if (inMessage) {
		[alert setInformativeText:inMessage];
	} else {
		[alert setInformativeText:@""];
	}
	[alert setAlertStyle:NSWarningAlertStyle];
	short response = [alert runModal];
	[alert release];
	return response;
}

//=========================================//
#pragma mark     DEFAULTS METHODS
//=========================================//

//-------------------------------------------------------------
// restoreDefaults:
//
// Called by the button click, this method takes the factory defaults Dictionary supplied by
// getDefaultValues and applies each setting to the current user defaults.  Because the
// items in the window are bound to the defaults, this causes them to be updated visually as well.
//-------------------------------------------------------------
- (IBAction)restoreDefaults:(id)sender
{
	NSLog(@"Restoring defaults now.");

	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSDictionary *appDefaults = (NSDictionary *)[self getDefaultValues];
	NSEnumerator *enumerator = [appDefaults keyEnumerator];
	id key;
	
	while ((key = [enumerator nextObject])) {
		/* code that uses the returned key */
		NSObject *thisObj = [appDefaults objectForKey:key];
		NSLog([NSString stringWithFormat:@"Found key %@ and Object %@", key, thisObj]);
		if ([thisObj isKindOfClass:[NSString class]]) {
			NSString *thisStr = (NSString *) thisObj;
			if ([thisStr isEqualToString:@"NO"] || [thisStr isEqualToString:@"N"] || [thisStr isEqualToString:@"0"]) {
				[defaults setBool:NO forKey:key];
			} else {
				[defaults setBool:YES forKey:key];
			}
		} else if ([thisObj isKindOfClass:[NSNumber class]]) {
			NSNumber *thisNum = (NSNumber *) thisObj;
			if ([thisNum intValue] == [thisNum floatValue]) {
				// then the number is an integer
				[defaults setInteger:[thisNum intValue] forKey:key];
			} else {
				// then the number is a float
				[defaults setFloat:[thisNum floatValue] forKey:key];
			}
		} else {
			NSLog(@"Not a bool or a number, probably some kind of object");
		}
	}
}


//-------------------------------------------------------------
// restoreDefaults:
//
// This method is called every time a setting is changed that might affect the shaving settings.
// This method examines the current settings and enables items as appropriate.
//-------------------------------------------------------------
- (IBAction)adjustShaveUISettings:(id)sender
{
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSObject *thisObj = [defaults objectForKey:@"sampleShape"];
	NSNumber *thisNum = (NSNumber *) thisObj;
	short value = [thisNum intValue];

	BOOL inflate = [defaults boolForKey:@"inflateBBox"];

	// if Shape = Sides then disable Shave panel and post notice on panel
	// if Shape = RP/Cyl then enable Shave panel and remove notice
	switch (value) {
		case popup_RectPrism:
		case popup_Cylinder: {
			if (inflate) {
				[defaults setBool:NO forKey:@"doShave"];
				[shaveButton setEnabled:NO];
				[shaveText setStringValue:@"Shaving is incompatible with inflation (Stats Panel)."];
			} else {
//				[defaults setBool:YES forKey:@"doShave"];	don't actually activate shaving
				[shaveButton setEnabled:YES];
				[shaveText setStringValue:@" "];
			}
		} break;
		case popup_Sides: {
			[shaveButton setEnabled:NO];
			[defaults setBool:NO forKey:@"doShave"];
			[shaveText setStringValue:@"Shaving is not possible with a Sides box (Stats Panel)."];
		} break;
	}
	if ([defaults boolForKey:@"keepAspectRatios"]) {
		[xyzButton setEnabled:NO];
		[maxAspectRatioTextField setEnabled:NO];
		[maxAspectRatioLabel setEnabled:NO];
	} else {
		[xyzButton setEnabled:YES];
		[maxAspectRatioTextField setEnabled:YES];
		[maxAspectRatioLabel setEnabled:YES];
	}
}

//-------------------------------------------------------------
// getDefaultValues
//
// This method returns a Dictionary of the factory default settings to use for 
// registering and for resetting defaults in restoreDefaults.
//-------------------------------------------------------------
- (id)getDefaultValues
{
	NSDictionary *appDefaults = [NSDictionary dictionaryWithObjectsAndKeys:
		@"NO",	@"includeMeanCSD",
		[NSNumber numberWithInt:63],	@"numClassesMeanCSD",
		[NSNumber numberWithFloat:3.00],	@"maxValueMeanCSD",
		@"NO",	@"includeLogCSD",
		[NSNumber numberWithInt:63],	@"numClassesLogCSD",
		@"NO",	@"includeMaxCSD",
		[NSNumber numberWithInt:63],	@"numClassesMaxCSD",
		@"NO",	@"includeRegCSD",
		[NSNumber numberWithFloat:0.10],	@"deltaLRegCSD",
		
		@"YES",	@"doQuadrat",
		[NSNumber numberWithInt:1000],	@"quadratNumPlacings",
		[NSNumber numberWithInt:10],	@"quadratNumReps",
		@"YES",	@"doImpingement",
		[NSNumber numberWithFloat:0.10],	@"impingementMeanErr",
		[NSNumber numberWithFloat:20.00],	@"impingementMaxErr",
		@"YES",	@"doOthers",
		@"NO",	@"inflateBBox",
		@"YES",	@"discardNegs",
		@"NO",	@"useRaeburn",
		@"NO",	@"useVolume",
		@"YES",	@"doRandomPt",
		[NSNumber numberWithInt:1000],	@"RPNumPlacings",
		[NSNumber numberWithInt:10],	@"RPNumReps",
		@"YES",	@"doLMcfPcf",
		[NSNumber numberWithInt:6],	@"numNNDist",
		[NSNumber numberWithInt:50],	@"overlap",
		[NSNumber numberWithInt:500],	@"numCFOffsetVolPts",
		[NSNumber numberWithFloat:0.1],	@"EpanecnikovCVal",
		[NSNumber numberWithInt:kRectPrism],	@"sampleShape",
		@"YES",	@"exscribedPrimitive",
		[NSNumber numberWithInt:0.01],	@"shrinkExscribedPrimitive",
		
		[NSNumber numberWithInt:100],	@"numEnvelopeRuns",
		@"NO",	@"matchVF",
		[NSNumber numberWithFloat:10.0],	@"VFPercent",
		@"NO",	@"matchPDF",
		@"NO",	@"makeDCEnv",
		@"YES",	@"observabilityFilter",
		[NSNumber numberWithFloat:0.85],	@"crit1Factor",
		[NSNumber numberWithFloat:3.0],	@"crit2Factor",
		
		@"YES",	@"outputReduce",
		@"NO",	@"outputCrystals",
		@"NO",	@"outputSigmas",
		[NSNumber numberWithInt:96],	@"confidence",
		
		[NSNumber numberWithInt:1234567],	@"seed",
		[NSNumber numberWithInt:5000],	@"MCReps",
		@"NO",	@"tidyUp",
		@"NO",	@"verbose",
		@"YES",	@"playSoundWhenDone",
		
		@"NO",	@"doShave",
		@"YES",	@"keepAspectRatios",
		[NSNumber numberWithInt:kX],	@"shaveXYZ",
		[NSNumber numberWithFloat:10.0],	@"shaveIncrement",
		[NSNumber numberWithInt:kSymmetric],	@"direction",
		[NSNumber numberWithFloat:10.0],	@"minPercent",
		[NSNumber numberWithInt:200],	@"minPopulation",
		[NSNumber numberWithFloat:1000.0],	@"maxAspectRatio",
		[NSNumber numberWithInt:kSmaller],	@"shaveSmLg",
		@"NO",	@"ShaveSave",
		nil];

	return appDefaults;
}

//=========================================//
#pragma mark     UTILITY METHODS
//=========================================//
- (void)showSettingsWindow
{
	[mainWindow makeKeyAndOrderFront:self];	// show main Window
}

- (IBAction)removeHolesFile:(id)sender
{
	holesFile = nil;
	[holesText setStringValue:@"None selected."];
}


@end
