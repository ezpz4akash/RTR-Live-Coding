//
//  MyView.h
//  Window2
//
//  Created by Akash Musale on 1/18/26.
//

#import <UIKit/UIKit.h>


@interface GLESView : UIView<UIGestureRecognizerDelegate>
-(void)startDisplayLink;
-(void)stopDisplayLink;
@end

