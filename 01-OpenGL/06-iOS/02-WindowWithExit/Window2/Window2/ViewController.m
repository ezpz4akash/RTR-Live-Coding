//
//  ViewController.m
//  Window2
//
//  Created by Akash Musale on 1/18/26.
//

#import "ViewController.h"
#import "MyView.h"

@implementation ViewController
{
    MyView *view;
}

-(void)loadView
{
    //code
    view = [[MyView alloc]initWithFrame:(CGRectZero)];
    [self setView:view];
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
