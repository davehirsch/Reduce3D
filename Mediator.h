//
//  Mediator.h
//  Reduce3D
//
//  Created by David Hirsch on 11/27/07.
//  Copyright 2007 David Hirsch. All rights reserved.
//

#import <Cocoa/Cocoa.h>

class stringFile;
class Calculator;

@class AppController;

// MediatorServerInterface is a formal protocol that defines the methods which
// the Mediator object responds to.
@protocol MediatorServerInterface

- (oneway void)runAnalysis:(AppController *)controller;

@end

@interface Mediator : NSObject <MediatorServerInterface>
{
	AppController *app;
	NSAutoreleasePool *pool;
	bool mCalcCanceled;
	Calculator *calc;
}

+ (void)connectWithPorts:(NSArray *)portArray;
    // This method is invoked on a new thread by the
    // Controller's init method.  The portArray contains
    // two NSPort objects that are the ports of the connection
    // to which the new thread should connect to.  The thread
    // connects to these ports, then creates a Mediator object
    // and calls the controller's setServer method to let it know
    // about the server object.  It then goes into a run loop
    // handling the incoming Distributed Object requests (ie
    // any methods in MediatorServerInterface).

- (id)init;
- (void)dealloc;

@end
