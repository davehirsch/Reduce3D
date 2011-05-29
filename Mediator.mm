// ===========================================================================
//	Mediator.mm
//  Reduce3D
//
//  Created by David Hirsch on 12/1/07.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================


#import "Mediator.h"
#import "AppController.h"
#import "stringFile.h"
#import "ReduceTypes.h"
#import "Calculator.h"
#include <vector>

@implementation Mediator

// Most of the Distributed Objects code required for communication between Mediator and AppController
//	is altered from Apple's Sample Code "TrivialThreads" program

+ (void)connectWithPorts:(NSArray *)portArray
{
    NSAutoreleasePool *pool;
    NSConnection *connectionToController;
    Mediator *mediatorObject;
	
    // First we must create an autorelease pool.  Methods that we invoke
    // are going to expect to be able to do [object autorelease], but this
    // doesn't work unless we explicitly create an autorelease pool because
    // we're running in a new thread, and threads don't start off with a
    // default autorelease pool.
    
    pool = [[NSAutoreleasePool alloc] init];
	
    // Now we connect back to the main thread using the ports given in our
    // portArray argument.  Note the subtle difference in that we use
    // connectWithReceivePort here, whereas Controller's init method uses
    // initWithReceivePort.
    
    connectionToController = [NSConnection connectionWithReceivePort:[portArray objectAtIndex:0]
															sendPort:[portArray objectAtIndex:1]];
	
    // Now create a server object.  In this example, we only have one server
    // object to handle all requests.
    
    mediatorObject = [[self alloc] init];
	
    // Now get the proxy of the root object at the other end of the connection
    // (which was set to be the controller object in Controller's init method)
    // and send it the setServer message so that it knows about our server object.
    // Note that there is nothing magic about the method name "setServer"; it's
    // merely the name I decided to use when designing these two objects.
    
    [ ((AppController *)[connectionToController rootProxy]) setMediatorDOServer:mediatorObject];
	
    // We now release our mediatorObject, so the AppController has the only reference
    // to it.
    
    [mediatorObject release];
	
    // Now we enter our run loop.  The run loop waits looking for events and
    // executes them.  In the case of a non-application thread, the source
    // of events is the NSConnections installed in the loop.  In our case, this
    // in the NSConnection we created earlier in this routine.  When a DO
    // message arrives on our receive port, the run loop unpackages the message
    // and executes the corresponding Objective-C method.
	
    [[NSRunLoop currentRunLoop] run];
	
    // Clean up.
	[[connectionToController receivePort] invalidate];
    [[connectionToController sendPort] invalidate];
	[connectionToController release];
    [pool release];
	
    //TIMC 3/17/98 -- commented this line out.  If [NSThread exit] is called, the thread terminates
    //immediately.  Unfortunately, this means that the code in NSThread that releases the parameters
    //is not executed, so you'd leak the parameters.  That code runs immediately after this function
    //completes.
    //[NSThread exit];
	
    return;
}


- (id)init
{
    self = [super init];
    if (self != nil) {
		// do init stuff here
		calc = nil;
	}
    return (self);
}

//-------------------------------------------------------------
// dealloc
//
// Standard dealloc
//-------------------------------------------------------------
- (void)dealloc
{	
	[super dealloc];
}

//-------------------------------------------------------------
// runAnalysis
//
// Repackages the inputFiles from an NSArray of NSStrings to a std::vector of std::strings, and repackages the
// user defaults into a struct to send to the part of the code that does the work, which is in C++ for legacy reasons.
// It then calls runAnalysis on the Calculator object to do the calculations.
//-------------------------------------------------------------
- (oneway void)runAnalysis:(AppController *)controller;
{
	try {
		PrefStruct *prefs = [controller getPrefStruct];
		if (calc != nil) delete calc;
		calc = new Calculator(self, controller, prefs);
		std::vector<std::string> inputFiles;
		NSArray *inNSStrFiles = [controller getInputFiles];
		NSString *holesFilePath = [controller getHolesFile];
		std::string holesFile;
		if (holesFilePath != nil) {
			holesFile = [holesFilePath UTF8String];
		} else {
			holesFile = "";
		}
		short numFiles = [inNSStrFiles count];
		for (short i=0; i < numFiles; i++) {
			std::string thisPath = [[inNSStrFiles objectAtIndex:i] UTF8String];
			inputFiles.push_back(thisPath);
		}
		bool result = calc->runAnalysis(inputFiles, holesFile);
		[controller runEnded: result];
	} catch (...) {
		// Most errors should be caught by the Calculator object.  Those that
		// get here are from outside its operation.  For now, we'll report them
		// and die.
		[controller postError:@"There was an unidentified error in Reduce3D.  A better programmer than Dave would have given a more descriptive error message here."
						title:@"Reduce 3D Error"
					  buttons:nil
				defaultButton:-1
					dismissIn:-1];
		[controller runEnded: false];
	}
}

@end
