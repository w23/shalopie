//
//  SPAppDelegate.h
//  sharopie
//
//  Created by Ivan Avdeev on 06.07.13.
//  Copyright (c) 2013 Ivan Avdeev. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <kapusha/sys/osx/KPView.h>

@interface SPAppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
@property (weak) IBOutlet KPView *viewport;

@end
