//
//  stringFile.mm
//  Reduce3D
//
//  Created by David Hirsch on 11/29/07.
//  Copyright 2007 David Hirsch. All rights reserved.
//
//  This is intended to be a replacement for the PowerPlant-based myLFileStream.
//	Although it will make extensive use of cocoa, the class itself is C++ so as to
//	easily interface with the existing Reduce code.
//	It will use an array of NSStrings (one per line) for its storage and write that to a file upon command,
//	or read from a file into that array on command.  This will be cool.
#import "stringFile.h"
#import <Cocoa/Cocoa.h>

stringFile::stringFile()
{
	mLines = nil;
	path = nil;
	mCurLine = 0;
}

stringFile::stringFile(bool inForOutput, CFStringRef inPath)
{
	mForOutput = inForOutput;
	mLines = nil;
	setPath(inPath);
	if (!inForOutput) {
		mCurLine = 0;
		readFromFile();
	}
}

stringFile::stringFile(bool inForOutput, const char *inPath)
{
	CFStringRef temp = CFStringCreateWithCString(NULL, inPath, kCFStringEncodingUTF8);
	mForOutput = inForOutput;
	mLines = nil;
	setPath(temp);
	if (!inForOutput) {
		mCurLine = 0;
		readFromFile();
	}
}

stringFile::~stringFile()
{
	if (mForOutput)
		writeToFile();
	if (mLines)
		CFRelease(mLines);
	if (path)
		CFRelease(path);
}

#pragma mark File manipulation methods

void
stringFile::setIsForOutput(bool inForOutput)
{
	mForOutput = inForOutput;
}

void
stringFile::setPath(CFStringRef inPath)
{
	path = CFStringCreateCopy(NULL, inPath);
}

void
stringFile::setPath(char *inPath)
{
	path = CFStringCreateWithCString(NULL, inPath, kCFStringEncodingUTF8);
}

CFStringRef
stringFile::getPath()
{
	return path;
}

std::string
stringFile::getName()
{
	NSString *nsPath = (NSString *)path;
	NSString *name = [nsPath lastPathComponent];
	static std::string outName = [name UTF8String];
	return outName;
}

void 
stringFile::copyPath(stringFile *inStringFile)
{
	path = CFStringCreateCopy(NULL, inStringFile->getPath());
}

#pragma mark Text I/O methods

bool
stringFile::syncWithFile()
{
	if (mForOutput)
		return writeToFile();
	else
		return readFromFile();
}

bool
stringFile::readFromFile()
{
	NSString *buffer =  [NSString stringWithContentsOfFile:(NSString *)path
													encoding:NSUTF8StringEncoding
													   error:NULL];
	if (buffer == nil) {
		return false;
	} else {
		unsigned length = [buffer length];
		unsigned paraStart = 0, paraEnd = 0, contentsEnd = 0;
		NSRange currentRange;
		NSLock *theLock = [NSLock new];
		[theLock lock];
		NSMutableArray *mutArray = [NSMutableArray array];
		while (paraEnd < length) {
			[buffer getParagraphStart:&paraStart 
									end:&paraEnd
							contentsEnd:&contentsEnd 
							   forRange:NSMakeRange(paraEnd, 0)];
			currentRange = NSMakeRange(paraStart, contentsEnd - paraStart);
			[mutArray addObject:[buffer substringWithRange:currentRange]];
		}
		if (mLines != nil) [(NSArray *)mLines release];
		mLines = (CFArrayRef) [NSArray arrayWithArray:mutArray];
		[(NSArray *)mLines retain];	// don't let this be autoreleased
		[theLock unlock];
		[theLock release];
		return true;
	}
}

bool
stringFile::writeToFile()
{
	NSLock *theLock = [NSLock new];
	[theLock lock];
	return [[(NSArray *)mLines componentsJoinedByString:@"\n"] writeToFile: (NSString *)path
					atomically:YES
					  encoding:NSUTF8StringEncoding
						 error:NULL];
	[theLock unlock];
	[theLock release];
}

std::string
stringFile::getOneLine()
{
	if (mCurLine >= [(NSArray *)mLines count]) {
		UnexpectedEOF err;
		err.filename = getName();
		throw(err);
	}
	NSString *thisLine = [(NSArray *)mLines objectAtIndex:mCurLine];
	mCurLine++;
	std::string outString = [thisLine UTF8String];
	return outString;
}

std::string
stringFile::peekLine()
{
	if (mCurLine >= [(NSArray *)mLines count]) {
		UnexpectedEOF err;
		err.filename = getName();
		throw(err);
	}
	NSString *thisLine = [(NSArray *)mLines objectAtIndex:mCurLine];
	std::string outString = [thisLine UTF8String];
	return outString;
}

void
stringFile::putOneLine(std::string *inStr)
{
	NSString *tempStr = [NSString stringWithUTF8String:inStr->c_str()];
	this->putOneLine((CFStringRef) tempStr);
}

void	
stringFile::putOneLine(const char *inStr)
{
	NSString *tempStr = [NSString stringWithUTF8String:inStr];
	this->putOneLine((CFStringRef) tempStr);
}

void	
stringFile::putOneLine(CFStringRef inStr)
{
	// temporarily make a Mutable copy
	NSMutableArray *mutArray = nil;
	NSLock *theLock = [NSLock new];
	[theLock lock];
	if ((NSArray *) mLines == nil) {
		mutArray = [NSMutableArray array];
	} else {
		mutArray = [NSMutableArray arrayWithArray:(NSArray *)mLines];
	}
	[mutArray addObject:(NSString *)inStr];
	if (mLines != nil) [(NSArray *)mLines release];
	mLines = (CFArrayRef) [NSArray arrayWithArray:mutArray];
	[(NSArray *)mLines retain];
	[theLock unlock];
	[theLock release];
}

