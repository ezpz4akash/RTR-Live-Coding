#import <Foundation/Foundation.h> //like stdio.h
#import <Cocoa/Cocoa.h> // like Windows.h

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface MyView : NSView
@end

int main(int argc, char** argv)
{
    //code 
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];
    
    NSApp = [NSApplication sharedApplication]; //NSApp process created
    
    [NSApp setDelegate:[[AppDelegate alloc]init]];

    [NSApp run];

    [pool release];

    return (0);
}

@implementation AppDelegate
{
    @private
    NSWindow *window;
    MyView *view;
}

// equivalent to WM_CREATE
-(void)applicationDidFinishLaunching:(NSNotification *)notification
{
    //code
    NSRect winRect = NSMakeRect(0.0, 0.0, 800.0, 600.0);
    window = [[NSWindow alloc]initWithContentRect:winRect
                                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                                backing:NSBackingStoreBuffered
                                defer:NO];

    [window setTitle:@"VVA : macOS WIndow"];
    [window center];

    view = [[MyView alloc]initWithFrame:winRect];

    [window setContentView:view];
    [window setDelegate:self];
    [window makeKeyAndOrderFront:self];  // setfocus, WS_TOP
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    //code

}

-(void)windowWillClose:(NSNotification *)notification
{
    //code
    [NSApp terminate:self];
}

-(void)dealloc
{
    //code 
    [view release];
    [window release];
    [super dealloc];
}

@end

@implementation MyView
{
    @private
    NSString *text;
}

-(id)initWithFrame:(NSRect)frame
{
    //code
    self = [super initWithFrame:frame];

    if (self)
    {
        [[self window]setContentView:self];
        
        text = @"Hello MAC World!!";
        
        [self setWantsLayer:YES];

        NSColor *backgColor = [NSColor blackColor];
        struct CGColor *bgColor = [backgColor CGColor];
        [[self layer]setBackgroundColor:bgColor];
    }

    return(self);    
}

-(void)drawRect:(NSRect)dirtyRect
{
    //code 
    NSFont *textFont = [NSFont fontWithName:@"Helvetica" size:32];
    NSColor *textColor = [NSColor colorWithDeviceRed:0.0 green:1.0 blue:0.0 alpha:1.0];
    NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:textFont, NSFontAttributeName,
                                                                        textColor, NSForegroundColorAttributeName,
                                                                        nil];
    NSSize textSize = [text sizeWithAttributes:dictionary];
    NSPoint point;
    point.x = (dirtyRect.size.width  / 2) - (textSize.width /2);
    point.y = (dirtyRect.size.height / 2) - (textSize.height / 2) + 12;
    [text drawAtPoint:point withAttributes:dictionary];
}

-(BOOL)acceptsFirstResponder
{
    //code
    [[self window]makeFirstResponder:self];

    return(YES);
}

-(void)keyDown:(NSEvent *)event
{
    //code

    int key = (int)[[event characters]characterAtIndex:0];

    switch(key)
    {
        case 27:
            [self release];
            [NSApp terminate:self];
            break;

        case 'F':
        case 'f':
            text = @"'F' or 'f' Key Is Pressed";
            [[self window]toggleFullScreen:self];
            break;

        default:
            break;

    }
}

-(void)mouseDown:(NSEvent *)event
{
    //code 
    text = @"Left Mouse Button is Clicked";
    [self setNeedsDisplay:YES];
}

-(void)dealloc
{
    //code
    [super dealloc];
}

@end