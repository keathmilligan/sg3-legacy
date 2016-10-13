//
//  ViewController.m
//  sg3ios
//
//  Created by Keath Milligan on 3/13/13.
//  Copyright (c) 2013 Keath Milligan. All rights reserved.
//

#import "ViewController.h"
#import "DemoView.h"

void _log_std_output(const char *msg)
{
    NSLog(@"%s", msg);
}

void _log_err_output(const char *msg)
{
    NSLog(@"ERROR: %s", msg);
}


@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	
    CGRect bounds = [[self view] bounds];
    NSLog(@"bounds: %@", NSStringFromCGRect(bounds));
    DemoView *demo = [[DemoView alloc] initWithFrame:bounds];
    self.view = demo;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void) startAnimation
{
    [(DemoView*)self.view startAnimation];
}

- (void) stopAnimation
{
    [(DemoView*)self.view stopAnimation];
}

@end
