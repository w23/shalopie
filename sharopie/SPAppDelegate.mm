//
//  SPAppDelegate.m
//  sharopie
//
//  Created by Ivan Avdeev on 06.07.13.
//  Copyright (c) 2013 Ivan Avdeev. All rights reserved.
//

#import "SPAppDelegate.h"
#include <kapusha/core.h>
#include "../src/viewport/Viewport.h"
#include <kapusha/fontain/coretext/Face.h>

@implementation SPAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  // Insert code here to initialize your application
  [KPView class];
  
  KP_LOG_OPEN("/tmp/Kapusha.log");
  [self.window setAcceptsMouseMovedEvents:YES];
  [self.viewport setViewport:new sharopie::Viewport(
                                                    new kapusha::fontain::coretext::Face("Courier New", kapusha::vec2i(42)))];
}

@end
