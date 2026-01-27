//
//  MyView.m
//  Window2
//
//  Created by Akash Musale on 1/18/26.
//

#import "MyView.h"

@implementation MyView
{
    NSString *text;
}

-(id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if(self)
    {
        [self setBackgroundColor:[UIColor blackColor]];
        
        //event handling
    
        text = @"Hello World!";

        //single tap
        UITapGestureRecognizer *singleTapGestureRecognizer =
               [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onSingleTap:)];
        [singleTapGestureRecognizer setNumberOfTapsRequired:1];
        [singleTapGestureRecognizer setNumberOfTouchesRequired:1];
        [singleTapGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:singleTapGestureRecognizer];

        //double tap
        UITapGestureRecognizer *doubleTapGestureRecongnizer =
               [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onDoubleTap:)];
        [doubleTapGestureRecongnizer setNumberOfTapsRequired:2];
        [doubleTapGestureRecongnizer setNumberOfTouchesRequired:1];
        [doubleTapGestureRecongnizer setDelegate:self];
        [self addGestureRecognizer:doubleTapGestureRecongnizer];
        
        //tell single tap recognizer to fail where there is double tap
        [singleTapGestureRecognizer requireGestureRecognizerToFail:doubleTapGestureRecongnizer];
        
        //swipe
        UISwipeGestureRecognizer *swipeGestureReconginzer =
               [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipe:)];
        [swipeGestureReconginzer setDelegate:self];
        [self addGestureRecognizer:swipeGestureReconginzer];

        //longpress
        UILongPressGestureRecognizer *longPressGestureRecognizer =
               [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(onLongPress:)];
        [longPressGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:longPressGestureRecognizer];
        
        
    }
    
    return (self);
}


-(void)drawRect:(CGRect)rect
{
    
    // Drawing code
    UIColor *backgroundColor = [UIColor blackColor];
    [backgroundColor set];
    UIRectFill(rect);

    NSDictionary *dictionaryForTextAttributes = [NSDictionary
          dictionaryWithObjectsAndKeys:[UIFont fontWithName:@"Helvetica" size:32], NSFontAttributeName,
                                       [UIColor greenColor], NSForegroundColorAttributeName, nil];

    CGSize textSize = [text sizeWithAttributes:dictionaryForTextAttributes];

    CGPoint point;
    point.x = (rect.size.width / 2) - (textSize.width / 2);
    point.y = (rect.size.height / 2) - (textSize.height / 2);

    [text drawAtPoint:point withAttributes:dictionaryForTextAttributes];
}

-(BOOL)becomeFirstResponder
{
    //code
    return (YES);
}

-(void)touchesBegan:(UITouch *)touches withEvent:(UIEvent *)event
{
    //code
}

- (void)onSingleTap:(UITapGestureRecognizer *)gestureRecognizer {
    // code
    text = @"Single Tap";
    [self setNeedsDisplay];
}

- (void)onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer {
    // code
    text = @"Double Tap";
    [self setNeedsDisplay];
}

- (void)onSwipe:(UITapGestureRecognizer *)gestureRecognizer {
    // code
    [self release];
    exit(0);
}

- (void)onLongPress:(UITapGestureRecognizer *)gestureRecognizer {
    // code
    text = @"Long Press";
    [self setNeedsDisplay];
}

-(void)dealloc
{
    [super dealloc];
}

@end
