#include "../include/vector.h"
#include <math.h>

float forearm_x, forearm_y, forearm_z;
float hand_x, hand_y, hand_z;

float forearm[3] = {forearm_x, forearm_y, forearm_z};
float hand[3] = {hand_x, hand_y, hand_z};

float wrist_deg = vec_angle_deg_a(forearm, hand);

if (isnan(wrist_deg))
{
    usart_print("Wrist angle: undefined (one or both vectors are near zero)\r\n");
}
else
{
    usart_print("Wrist angle: ");
    usart_print_float(wrist_deg, 2);
    usart_print(" degrees\r\n");
}
