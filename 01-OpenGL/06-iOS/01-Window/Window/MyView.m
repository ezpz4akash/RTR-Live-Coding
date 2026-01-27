//
//  MyView.m
//  Window
//
//  Created by user947254 on 1/18/26.
//

#import "MyView.h"

@implementation MyView {
 @private
  NSString *centralText;
}

- (id)initWithFrame:(CGRect)frame {
  // code
  self = [super initWithFrame:frame];

  if (self) {
    centralText = @"Hello World!";

    UITapGestureRecognizer *singleTapGestureRecognizer =
        [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onSingleTap:)];
    [singleTapGestureRecognizer setNumberOfTapsRequired:1];
    [singleTapGestureRecognizer setNumberOfTouchesRequired:1];
    [singleTapGestureRecognizer setDelegate:self];
    [self addGestureRecognizer:singleTapGestureRecognizer];

    UITapGestureRecognizer *doubleTapGestureRecongnizer =
        [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onDoubleTap:)];
    [doubleTapGestureRecongnizer setNumberOfTapsRequired:2];
    [doubleTapGestureRecongnizer setNumberOfTouchesRequired:1];
    [doubleTapGestureRecongnizer setDelegate:self];
    [self addGestureRecognizer:doubleTapGestureRecongnizer];

    // this will separate two single taps from double tap
    [singleTapGestureRecognizer requireGestureRecognizerToFail:doubleTapGestureRecongnizer];

    UISwipeGestureRecognizer *swipeGestureReconginzer =
        [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipe:)];
    [swipeGestureReconginzer setDelegate:self];
    [self addGestureRecognizer:swipeGestureReconginzer];

    UILongPressGestureRecognizer *longPressGestureRecognizer =
        [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(onLongPress:)];
    [longPressGestureRecognizer setDelegate:self];
    [self addGestureRecognizer:longPressGestureRecognizer];
  }

  return (self);
}

- (void)drawRect:(CGRect)rect {
  // Drawing code
  UIColor *backgroundColor = [UIColor blackColor];
  [backgroundColor set];
  UIRectFill(rect);

  NSDictionary *dictionaryForTextAttributes = [NSDictionary
      dictionaryWithObjectsAndKeys:[UIFont fontWithName:@"Helvetica" size:32], NSFontAttributeName,
                                   [UIColor greenColor], NSForegroundColorAttributeName, nil];

  CGSize textSize = [centralText sizeWithAttributes:dictionaryForTextAttributes];

  CGPoint point;
  point.x = (rect.size.width / 2) - (textSize.width / 2);
  point.y = (rect.size.height / 2) - (textSize.height / 2);

  [centralText drawAtPoint:point withAttributes:dictionaryForTextAttributes];
}

- (void)onSingleTap:(UITapGestureRecognizer *)gestureRecognizer {
  // code
  centralText = @"Single Tap";
  [self setNeedsDisplay];
}

- (void)onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer  {
  // code
  centralText = @"Double Tap";
  [self setNeedsDisplay];
}

- (void)onSwipe:(UISwipeGestureRecognizer *)gestureRecognizer  {
  // code
  centralText = @"Swipe";
  [self setNeedsDisplay];
}

- (void)onLongPress:(UIGestureRecognizer *)gestureRecognizer  {
  // code
  centralText = @"Long Press";
  [self setNeedsDisplay];
}	

- (void)dealloc {
  // code
  [super dealloc];
}

@end
