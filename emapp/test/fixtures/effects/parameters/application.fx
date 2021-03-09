/* system */
float2 viewport_size : VIEWPORTPIXELSIZE;
float time : TIME;
float elapsed_time : ELAPSEDTIME;
float time_sync_in_edit_mode : TIME < bool SyncInEditMode=true; >;
float elapsed_time_sync_in_edit_mode : ELAPSEDTIME < bool SyncInEditMode=true; >;
float2 mouse_position : MOUSEPOSITION;
float4 left_mouse_down : LEFTMOUSEDOWN;
float4 middle_mouse_down : MIDDLEMOUSEDOWN;
float4 right_mouse_down : RIGHTMOUSEDOWN;

#include "shaders.fx"
