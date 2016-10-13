/* demo.h - SG3 demo
 * Copyright 2012 Keath Milligan
 */


#ifndef DEMO_H_
#define DEMO_H_

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
void demo_reset();
void demo_throttle_up();
void demo_throttle_down();
void demo_throttle_reset();

#endif /* DEMO_H_ */
