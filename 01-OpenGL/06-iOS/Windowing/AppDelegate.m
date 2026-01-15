#import "AppDelegate.h"

@implementation AppDelegate
{
    UIWindow *window;
    ViewController *viewController;
    MyView *myView;
}

- (BOOL)application:(UIApplication *)application
didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // code
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    window = [[UIWindow alloc] initWithFrame:screenRect]];
    viewController = [[ViewController alloc] init];
    myView = [[MyView alloc] init];
    window.rootViewController = viewController;
    viewController.view = myView;
    [window makeKeyAndVisible];
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // code
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // code
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // code
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // code
}

- (void)dealloc
{
    // code
    [super dealloc];
}

@end
