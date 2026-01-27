//
//  ViewController.m
//  Window2
//
//  Created by user947254 on 1/18/26.
//

#import "ViewController.h"
#import "GLESView.h"

@implementation ViewController
{
    GLESView *view;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
}

-(void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
}

-(void)dealloc
{
    [view release];
    [super dealloc];
}

@end
