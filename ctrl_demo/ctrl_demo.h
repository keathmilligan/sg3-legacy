/* ctrl_demo.h - SG3 demo
 * Copyright 2012 Keath Milligan
 */


#ifndef CTRL_DEMO_H_
#define CTRL_DEMO_H_

int demo_init();
void demo_reshape_viewport(int width, int height);
void demo_render();
void demo_cleanup();
void demo_up(int state);
void demo_down(int state);
void demo_right(int state);
void demo_left(int state);
void demo_roll_right(int state);
void demo_roll_left(int state);
void demo_throttle_up(int state);
void demo_throttle_down(int state);

#endif /* CTRL_DEMO_H_ */
