//
//  AppDelegate.m
//  Window
//
//  Created by user947254 on 1/18/26.
//

#import "AppDelegate.h"
#import "MyView.h"
#import "ViewController.h"

@implementation AppDelegate {
  UIWindow *window;
  ViewController *viewController;
  MyView *view;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
  // Override point for customization after application launch.
  CGRect winRect = [[UIScreen mainScreen] bounds];

  window = [[UIWindow alloc] initWithFrame:winRect];
  viewController = [[ViewController alloc] init];

  [window setRootViewController:viewController];

  view = [[MyView alloc] initWithFrame:winRect];

  [viewController setView:view];
  [view release];

  [window makeKeyAndVisible];

  return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
  // code
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
  // code
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
  // code
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
  // code
}

- (void)applicationWillTerminate:(UIApplication *)application {
  // code
}

- (void)dealloc {
  // code
  [view release];
  [viewController release];
  [window release];

  [super dealloc];
}

@end
