//
//  SceneDelegate.m
//  Window2
//
//  Created by Akash Musale on 1/18/26.
//

#import "SceneDelegate.h"
#import "GLESView.h"


@implementation SceneDelegate
{
    GLESView *view;
}


- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions
{
    // code
    view  = (GLESView *)[[[self window]rootViewController]view];
    if([view isKindOfClass:[GLESView class]])
    {
        [view startDisplayLink];
    }
}

- (void)sceneWillResignActive:(UIScene *)scene
{
    // code
    if([view isKindOfClass:[GLESView class]])
    {
        [view stopDisplayLink];
    }
    
}

- (void)sceneDidEnterBackground:(UIScene *)scene
{
    // code
}

- (void)sceneWillEnterForeground:(UIScene *)scene
{
    // code
}

- (void)sceneDidBecomeActive:(UIScene *)scene
{
    // code
    if([view isKindOfClass:[GLESView class]])
    {
        [view startDisplayLink];
    }
}

- (void)sceneDidDisconnect:(UIScene *)scene
{
    // code
    if([view isKindOfClass:[GLESView class]])
    {
        [view stopDisplayLink];
    }
}

-(void)dealloc
{
    [super dealloc];
}


@end
