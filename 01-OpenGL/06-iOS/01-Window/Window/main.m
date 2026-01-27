//
//  main.m
//  Window
//
//  Created by user947254 on 1/18/26.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int main(int argc, char *argv[]) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  NSString *appDelegateClassName = NSStringFromClass([AppDelegate class]);
  int ret = UIApplicationMain(argc, argv, nil, appDelegateClassName);

  [pool release];

  return (ret);
}
