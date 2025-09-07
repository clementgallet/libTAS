/*
 * This file generated automatically from xinput.xml by c_client.py.
 * Edit at your peril.
 */

/**
 * @defgroup XCB_Input_API XCB Input API
 * @brief Input XCB Protocol Implementation.
 * @{
 **/

#ifndef __XINPUT_H
#define __XINPUT_H

// #include "xcb.h"
// #include "xfixes.h"

// #ifdef __cplusplus
// extern "C" {
// #endif

#define XCB_INPUT_MAJOR_VERSION 2
#define XCB_INPUT_MINOR_VERSION 4

extern xcb_extension_t xcb_input_id;

typedef uint32_t xcb_input_event_class_t;

/**
 * @brief xcb_input_event_class_iterator_t
 **/
typedef struct xcb_input_event_class_iterator_t {
    xcb_input_event_class_t *data;
    int                      rem;
    int                      index;
} xcb_input_event_class_iterator_t;

typedef uint8_t xcb_input_key_code_t;

/**
 * @brief xcb_input_key_code_iterator_t
 **/
typedef struct xcb_input_key_code_iterator_t {
    xcb_input_key_code_t *data;
    int                   rem;
    int                   index;
} xcb_input_key_code_iterator_t;

typedef uint16_t xcb_input_device_id_t;

/**
 * @brief xcb_input_device_id_iterator_t
 **/
typedef struct xcb_input_device_id_iterator_t {
    xcb_input_device_id_t *data;
    int                    rem;
    int                    index;
} xcb_input_device_id_iterator_t;

typedef int32_t xcb_input_fp1616_t;

/**
 * @brief xcb_input_fp1616_iterator_t
 **/
typedef struct xcb_input_fp1616_iterator_t {
    xcb_input_fp1616_t *data;
    int                 rem;
    int                 index;
} xcb_input_fp1616_iterator_t;

/**
 * @brief xcb_input_fp3232_t
 **/
typedef struct xcb_input_fp3232_t {
    int32_t  integral;
    uint32_t frac;
} xcb_input_fp3232_t;

/**
 * @brief xcb_input_fp3232_iterator_t
 **/
typedef struct xcb_input_fp3232_iterator_t {
    xcb_input_fp3232_t *data;
    int                 rem;
    int                 index;
} xcb_input_fp3232_iterator_t;

/**
 * @brief xcb_input_get_extension_version_cookie_t
 **/
typedef struct xcb_input_get_extension_version_cookie_t {
    unsigned int sequence;
} xcb_input_get_extension_version_cookie_t;

/** Opcode for xcb_input_get_extension_version. */
#define XCB_INPUT_GET_EXTENSION_VERSION 1

/**
 * @brief xcb_input_get_extension_version_request_t
 **/
typedef struct xcb_input_get_extension_version_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint16_t name_len;
    uint8_t  pad0[2];
} xcb_input_get_extension_version_request_t;

/**
 * @brief xcb_input_get_extension_version_reply_t
 **/
typedef struct xcb_input_get_extension_version_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint16_t server_major;
    uint16_t server_minor;
    uint8_t  present;
    uint8_t  pad0[19];
} xcb_input_get_extension_version_reply_t;

typedef enum xcb_input_device_use_t {
    XCB_INPUT_DEVICE_USE_IS_X_POINTER = 0,
    XCB_INPUT_DEVICE_USE_IS_X_KEYBOARD = 1,
    XCB_INPUT_DEVICE_USE_IS_X_EXTENSION_DEVICE = 2,
    XCB_INPUT_DEVICE_USE_IS_X_EXTENSION_KEYBOARD = 3,
    XCB_INPUT_DEVICE_USE_IS_X_EXTENSION_POINTER = 4
} xcb_input_device_use_t;

typedef enum xcb_input_input_class_t {
    XCB_INPUT_INPUT_CLASS_KEY = 0,
    XCB_INPUT_INPUT_CLASS_BUTTON = 1,
    XCB_INPUT_INPUT_CLASS_VALUATOR = 2,
    XCB_INPUT_INPUT_CLASS_FEEDBACK = 3,
    XCB_INPUT_INPUT_CLASS_PROXIMITY = 4,
    XCB_INPUT_INPUT_CLASS_FOCUS = 5,
    XCB_INPUT_INPUT_CLASS_OTHER = 6
} xcb_input_input_class_t;

typedef enum xcb_input_valuator_mode_t {
    XCB_INPUT_VALUATOR_MODE_RELATIVE = 0,
    XCB_INPUT_VALUATOR_MODE_ABSOLUTE = 1
} xcb_input_valuator_mode_t;

/**
 * @brief xcb_input_device_info_t
 **/
typedef struct xcb_input_device_info_t {
    xcb_atom_t device_type;
    uint8_t    device_id;
    uint8_t    num_class_info;
    uint8_t    device_use;
    uint8_t    pad0;
} xcb_input_device_info_t;

/**
 * @brief xcb_input_device_info_iterator_t
 **/
typedef struct xcb_input_device_info_iterator_t {
    xcb_input_device_info_t *data;
    int                      rem;
    int                      index;
} xcb_input_device_info_iterator_t;

/**
 * @brief xcb_input_key_info_t
 **/
typedef struct xcb_input_key_info_t {
    uint8_t              class_id;
    uint8_t              len;
    xcb_input_key_code_t min_keycode;
    xcb_input_key_code_t max_keycode;
    uint16_t             num_keys;
    uint8_t              pad0[2];
} xcb_input_key_info_t;

/**
 * @brief xcb_input_key_info_iterator_t
 **/
typedef struct xcb_input_key_info_iterator_t {
    xcb_input_key_info_t *data;
    int                   rem;
    int                   index;
} xcb_input_key_info_iterator_t;

/**
 * @brief xcb_input_button_info_t
 **/
typedef struct xcb_input_button_info_t {
    uint8_t  class_id;
    uint8_t  len;
    uint16_t num_buttons;
} xcb_input_button_info_t;

/**
 * @brief xcb_input_button_info_iterator_t
 **/
typedef struct xcb_input_button_info_iterator_t {
    xcb_input_button_info_t *data;
    int                      rem;
    int                      index;
} xcb_input_button_info_iterator_t;

/**
 * @brief xcb_input_axis_info_t
 **/
typedef struct xcb_input_axis_info_t {
    uint32_t resolution;
    int32_t  minimum;
    int32_t  maximum;
} xcb_input_axis_info_t;

/**
 * @brief xcb_input_axis_info_iterator_t
 **/
typedef struct xcb_input_axis_info_iterator_t {
    xcb_input_axis_info_t *data;
    int                    rem;
    int                    index;
} xcb_input_axis_info_iterator_t;

/**
 * @brief xcb_input_valuator_info_t
 **/
typedef struct xcb_input_valuator_info_t {
    uint8_t  class_id;
    uint8_t  len;
    uint8_t  axes_len;
    uint8_t  mode;
    uint32_t motion_size;
} xcb_input_valuator_info_t;

/**
 * @brief xcb_input_valuator_info_iterator_t
 **/
typedef struct xcb_input_valuator_info_iterator_t {
    xcb_input_valuator_info_t *data;
    int                        rem;
    int                        index;
} xcb_input_valuator_info_iterator_t;

/**
 * @brief xcb_input_input_info_info_t
 **/
typedef struct xcb_input_input_info_info_t {
    struct {
        xcb_input_key_code_t   min_keycode;
        xcb_input_key_code_t   max_keycode;
        uint16_t               num_keys;
        uint8_t                pad0[2];
    } key;
    struct {
        uint16_t               num_buttons;
    } button;
    struct {
        uint8_t                axes_len;
        uint8_t                mode;
        uint32_t               motion_size;
        xcb_input_axis_info_t *axes;
    } valuator;
} xcb_input_input_info_info_t;

/**
 * @brief xcb_input_input_info_t
 **/
typedef struct xcb_input_input_info_t {
    uint8_t class_id;
    uint8_t len;
} xcb_input_input_info_t;

void *
xcb_input_input_info_info (const xcb_input_input_info_t *R);

/**
 * @brief xcb_input_input_info_iterator_t
 **/
typedef struct xcb_input_input_info_iterator_t {
    xcb_input_input_info_t *data;
    int                     rem;
    int                     index;
} xcb_input_input_info_iterator_t;

/**
 * @brief xcb_input_device_name_t
 **/
typedef struct xcb_input_device_name_t {
    uint8_t len;
} xcb_input_device_name_t;

/**
 * @brief xcb_input_device_name_iterator_t
 **/
typedef struct xcb_input_device_name_iterator_t {
    xcb_input_device_name_t *data;
    int                      rem;
    int                      index;
} xcb_input_device_name_iterator_t;

/**
 * @brief xcb_input_list_input_devices_cookie_t
 **/
typedef struct xcb_input_list_input_devices_cookie_t {
    unsigned int sequence;
} xcb_input_list_input_devices_cookie_t;

/** Opcode for xcb_input_list_input_devices. */
#define XCB_INPUT_LIST_INPUT_DEVICES 2

/**
 * @brief xcb_input_list_input_devices_request_t
 **/
typedef struct xcb_input_list_input_devices_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
} xcb_input_list_input_devices_request_t;

/**
 * @brief xcb_input_list_input_devices_reply_t
 **/
typedef struct xcb_input_list_input_devices_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  devices_len;
    uint8_t  pad0[23];
} xcb_input_list_input_devices_reply_t;

typedef uint8_t xcb_input_event_type_base_t;

/**
 * @brief xcb_input_event_type_base_iterator_t
 **/
typedef struct xcb_input_event_type_base_iterator_t {
    xcb_input_event_type_base_t *data;
    int                          rem;
    int                          index;
} xcb_input_event_type_base_iterator_t;

/**
 * @brief xcb_input_input_class_info_t
 **/
typedef struct xcb_input_input_class_info_t {
    uint8_t                     class_id;
    xcb_input_event_type_base_t event_type_base;
} xcb_input_input_class_info_t;

/**
 * @brief xcb_input_input_class_info_iterator_t
 **/
typedef struct xcb_input_input_class_info_iterator_t {
    xcb_input_input_class_info_t *data;
    int                           rem;
    int                           index;
} xcb_input_input_class_info_iterator_t;

/**
 * @brief xcb_input_open_device_cookie_t
 **/
typedef struct xcb_input_open_device_cookie_t {
    unsigned int sequence;
} xcb_input_open_device_cookie_t;

/** Opcode for xcb_input_open_device. */
#define XCB_INPUT_OPEN_DEVICE 3

/**
 * @brief xcb_input_open_device_request_t
 **/
typedef struct xcb_input_open_device_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  pad0[3];
} xcb_input_open_device_request_t;

/**
 * @brief xcb_input_open_device_reply_t
 **/
typedef struct xcb_input_open_device_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  num_classes;
    uint8_t  pad0[23];
} xcb_input_open_device_reply_t;

/** Opcode for xcb_input_close_device. */
#define XCB_INPUT_CLOSE_DEVICE 4

/**
 * @brief xcb_input_close_device_request_t
 **/
typedef struct xcb_input_close_device_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  pad0[3];
} xcb_input_close_device_request_t;

/**
 * @brief xcb_input_set_device_mode_cookie_t
 **/
typedef struct xcb_input_set_device_mode_cookie_t {
    unsigned int sequence;
} xcb_input_set_device_mode_cookie_t;

/** Opcode for xcb_input_set_device_mode. */
#define XCB_INPUT_SET_DEVICE_MODE 5

/**
 * @brief xcb_input_set_device_mode_request_t
 **/
typedef struct xcb_input_set_device_mode_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  mode;
    uint8_t  pad0[2];
} xcb_input_set_device_mode_request_t;

/**
 * @brief xcb_input_set_device_mode_reply_t
 **/
typedef struct xcb_input_set_device_mode_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  status;
    uint8_t  pad0[23];
} xcb_input_set_device_mode_reply_t;

/** Opcode for xcb_input_select_extension_event. */
#define XCB_INPUT_SELECT_EXTENSION_EVENT 6

/**
 * @brief xcb_input_select_extension_event_request_t
 **/
typedef struct xcb_input_select_extension_event_request_t {
    uint8_t      major_opcode;
    uint8_t      minor_opcode;
    uint16_t     length;
    xcb_window_t window;
    uint16_t     num_classes;
    uint8_t      pad0[2];
} xcb_input_select_extension_event_request_t;

/**
 * @brief xcb_input_get_selected_extension_events_cookie_t
 **/
typedef struct xcb_input_get_selected_extension_events_cookie_t {
    unsigned int sequence;
} xcb_input_get_selected_extension_events_cookie_t;

/** Opcode for xcb_input_get_selected_extension_events. */
#define XCB_INPUT_GET_SELECTED_EXTENSION_EVENTS 7

/**
 * @brief xcb_input_get_selected_extension_events_request_t
 **/
typedef struct xcb_input_get_selected_extension_events_request_t {
    uint8_t      major_opcode;
    uint8_t      minor_opcode;
    uint16_t     length;
    xcb_window_t window;
} xcb_input_get_selected_extension_events_request_t;

/**
 * @brief xcb_input_get_selected_extension_events_reply_t
 **/
typedef struct xcb_input_get_selected_extension_events_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint16_t num_this_classes;
    uint16_t num_all_classes;
    uint8_t  pad0[20];
} xcb_input_get_selected_extension_events_reply_t;

typedef enum xcb_input_propagate_mode_t {
    XCB_INPUT_PROPAGATE_MODE_ADD_TO_LIST = 0,
    XCB_INPUT_PROPAGATE_MODE_DELETE_FROM_LIST = 1
} xcb_input_propagate_mode_t;

/** Opcode for xcb_input_change_device_dont_propagate_list. */
#define XCB_INPUT_CHANGE_DEVICE_DONT_PROPAGATE_LIST 8

/**
 * @brief xcb_input_change_device_dont_propagate_list_request_t
 **/
typedef struct xcb_input_change_device_dont_propagate_list_request_t {
    uint8_t      major_opcode;
    uint8_t      minor_opcode;
    uint16_t     length;
    xcb_window_t window;
    uint16_t     num_classes;
    uint8_t      mode;
    uint8_t      pad0;
} xcb_input_change_device_dont_propagate_list_request_t;

/**
 * @brief xcb_input_get_device_dont_propagate_list_cookie_t
 **/
typedef struct xcb_input_get_device_dont_propagate_list_cookie_t {
    unsigned int sequence;
} xcb_input_get_device_dont_propagate_list_cookie_t;

/** Opcode for xcb_input_get_device_dont_propagate_list. */
#define XCB_INPUT_GET_DEVICE_DONT_PROPAGATE_LIST 9

/**
 * @brief xcb_input_get_device_dont_propagate_list_request_t
 **/
typedef struct xcb_input_get_device_dont_propagate_list_request_t {
    uint8_t      major_opcode;
    uint8_t      minor_opcode;
    uint16_t     length;
    xcb_window_t window;
} xcb_input_get_device_dont_propagate_list_request_t;

/**
 * @brief xcb_input_get_device_dont_propagate_list_reply_t
 **/
typedef struct xcb_input_get_device_dont_propagate_list_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint16_t num_classes;
    uint8_t  pad0[22];
} xcb_input_get_device_dont_propagate_list_reply_t;

/**
 * @brief xcb_input_device_time_coord_t
 **/
typedef struct xcb_input_device_time_coord_t {
    xcb_timestamp_t time;
} xcb_input_device_time_coord_t;

/**
 * @brief xcb_input_device_time_coord_iterator_t
 **/
typedef struct xcb_input_device_time_coord_iterator_t {
    xcb_input_device_time_coord_t *data;
    int                            rem;
    int                            index;
    uint8_t                        num_axes; /**<  */
} xcb_input_device_time_coord_iterator_t;

/**
 * @brief xcb_input_get_device_motion_events_cookie_t
 **/
typedef struct xcb_input_get_device_motion_events_cookie_t {
    unsigned int sequence;
} xcb_input_get_device_motion_events_cookie_t;

/** Opcode for xcb_input_get_device_motion_events. */
#define XCB_INPUT_GET_DEVICE_MOTION_EVENTS 10

/**
 * @brief xcb_input_get_device_motion_events_request_t
 **/
typedef struct xcb_input_get_device_motion_events_request_t {
    uint8_t         major_opcode;
    uint8_t         minor_opcode;
    uint16_t        length;
    xcb_timestamp_t start;
    xcb_timestamp_t stop;
    uint8_t         device_id;
    uint8_t         pad0[3];
} xcb_input_get_device_motion_events_request_t;

/**
 * @brief xcb_input_get_device_motion_events_reply_t
 **/
typedef struct xcb_input_get_device_motion_events_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint32_t num_events;
    uint8_t  num_axes;
    uint8_t  device_mode;
    uint8_t  pad0[18];
} xcb_input_get_device_motion_events_reply_t;

/**
 * @brief xcb_input_change_keyboard_device_cookie_t
 **/
typedef struct xcb_input_change_keyboard_device_cookie_t {
    unsigned int sequence;
} xcb_input_change_keyboard_device_cookie_t;

/** Opcode for xcb_input_change_keyboard_device. */
#define XCB_INPUT_CHANGE_KEYBOARD_DEVICE 11

/**
 * @brief xcb_input_change_keyboard_device_request_t
 **/
typedef struct xcb_input_change_keyboard_device_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  pad0[3];
} xcb_input_change_keyboard_device_request_t;

/**
 * @brief xcb_input_change_keyboard_device_reply_t
 **/
typedef struct xcb_input_change_keyboard_device_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  status;
    uint8_t  pad0[23];
} xcb_input_change_keyboard_device_reply_t;

/**
 * @brief xcb_input_change_pointer_device_cookie_t
 **/
typedef struct xcb_input_change_pointer_device_cookie_t {
    unsigned int sequence;
} xcb_input_change_pointer_device_cookie_t;

/** Opcode for xcb_input_change_pointer_device. */
#define XCB_INPUT_CHANGE_POINTER_DEVICE 12

/**
 * @brief xcb_input_change_pointer_device_request_t
 **/
typedef struct xcb_input_change_pointer_device_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  x_axis;
    uint8_t  y_axis;
    uint8_t  device_id;
    uint8_t  pad0;
} xcb_input_change_pointer_device_request_t;

/**
 * @brief xcb_input_change_pointer_device_reply_t
 **/
typedef struct xcb_input_change_pointer_device_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  status;
    uint8_t  pad0[23];
} xcb_input_change_pointer_device_reply_t;

/**
 * @brief xcb_input_grab_device_cookie_t
 **/
typedef struct xcb_input_grab_device_cookie_t {
    unsigned int sequence;
} xcb_input_grab_device_cookie_t;

/** Opcode for xcb_input_grab_device. */
#define XCB_INPUT_GRAB_DEVICE 13

/**
 * @brief xcb_input_grab_device_request_t
 **/
typedef struct xcb_input_grab_device_request_t {
    uint8_t         major_opcode;
    uint8_t         minor_opcode;
    uint16_t        length;
    xcb_window_t    grab_window;
    xcb_timestamp_t time;
    uint16_t        num_classes;
    uint8_t         this_device_mode;
    uint8_t         other_device_mode;
    uint8_t         owner_events;
    uint8_t         device_id;
    uint8_t         pad0[2];
} xcb_input_grab_device_request_t;

/**
 * @brief xcb_input_grab_device_reply_t
 **/
typedef struct xcb_input_grab_device_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  status;
    uint8_t  pad0[23];
} xcb_input_grab_device_reply_t;

/** Opcode for xcb_input_ungrab_device. */
#define XCB_INPUT_UNGRAB_DEVICE 14

/**
 * @brief xcb_input_ungrab_device_request_t
 **/
typedef struct xcb_input_ungrab_device_request_t {
    uint8_t         major_opcode;
    uint8_t         minor_opcode;
    uint16_t        length;
    xcb_timestamp_t time;
    uint8_t         device_id;
    uint8_t         pad0[3];
} xcb_input_ungrab_device_request_t;

typedef enum xcb_input_modifier_device_t {
    XCB_INPUT_MODIFIER_DEVICE_USE_X_KEYBOARD = 255
} xcb_input_modifier_device_t;

/** Opcode for xcb_input_grab_device_key. */
#define XCB_INPUT_GRAB_DEVICE_KEY 15

/**
 * @brief xcb_input_grab_device_key_request_t
 **/
typedef struct xcb_input_grab_device_key_request_t {
    uint8_t      major_opcode;
    uint8_t      minor_opcode;
    uint16_t     length;
    xcb_window_t grab_window;
    uint16_t     num_classes;
    uint16_t     modifiers;
    uint8_t      modifier_device;
    uint8_t      grabbed_device;
    uint8_t      key;
    uint8_t      this_device_mode;
    uint8_t      other_device_mode;
    uint8_t      owner_events;
    uint8_t      pad0[2];
} xcb_input_grab_device_key_request_t;

/** Opcode for xcb_input_ungrab_device_key. */
#define XCB_INPUT_UNGRAB_DEVICE_KEY 16

/**
 * @brief xcb_input_ungrab_device_key_request_t
 **/
typedef struct xcb_input_ungrab_device_key_request_t {
    uint8_t      major_opcode;
    uint8_t      minor_opcode;
    uint16_t     length;
    xcb_window_t grabWindow;
    uint16_t     modifiers;
    uint8_t      modifier_device;
    uint8_t      key;
    uint8_t      grabbed_device;
} xcb_input_ungrab_device_key_request_t;

/** Opcode for xcb_input_grab_device_button. */
#define XCB_INPUT_GRAB_DEVICE_BUTTON 17

/**
 * @brief xcb_input_grab_device_button_request_t
 **/
typedef struct xcb_input_grab_device_button_request_t {
    uint8_t      major_opcode;
    uint8_t      minor_opcode;
    uint16_t     length;
    xcb_window_t grab_window;
    uint8_t      grabbed_device;
    uint8_t      modifier_device;
    uint16_t     num_classes;
    uint16_t     modifiers;
    uint8_t      this_device_mode;
    uint8_t      other_device_mode;
    uint8_t      button;
    uint8_t      owner_events;
    uint8_t      pad0[2];
} xcb_input_grab_device_button_request_t;

/** Opcode for xcb_input_ungrab_device_button. */
#define XCB_INPUT_UNGRAB_DEVICE_BUTTON 18

/**
 * @brief xcb_input_ungrab_device_button_request_t
 **/
typedef struct xcb_input_ungrab_device_button_request_t {
    uint8_t      major_opcode;
    uint8_t      minor_opcode;
    uint16_t     length;
    xcb_window_t grab_window;
    uint16_t     modifiers;
    uint8_t      modifier_device;
    uint8_t      button;
    uint8_t      grabbed_device;
    uint8_t      pad0[3];
} xcb_input_ungrab_device_button_request_t;

typedef enum xcb_input_device_input_mode_t {
    XCB_INPUT_DEVICE_INPUT_MODE_ASYNC_THIS_DEVICE = 0,
    XCB_INPUT_DEVICE_INPUT_MODE_SYNC_THIS_DEVICE = 1,
    XCB_INPUT_DEVICE_INPUT_MODE_REPLAY_THIS_DEVICE = 2,
    XCB_INPUT_DEVICE_INPUT_MODE_ASYNC_OTHER_DEVICES = 3,
    XCB_INPUT_DEVICE_INPUT_MODE_ASYNC_ALL = 4,
    XCB_INPUT_DEVICE_INPUT_MODE_SYNC_ALL = 5
} xcb_input_device_input_mode_t;

/** Opcode for xcb_input_allow_device_events. */
#define XCB_INPUT_ALLOW_DEVICE_EVENTS 19

/**
 * @brief xcb_input_allow_device_events_request_t
 **/
typedef struct xcb_input_allow_device_events_request_t {
    uint8_t         major_opcode;
    uint8_t         minor_opcode;
    uint16_t        length;
    xcb_timestamp_t time;
    uint8_t         mode;
    uint8_t         device_id;
    uint8_t         pad0[2];
} xcb_input_allow_device_events_request_t;

/**
 * @brief xcb_input_get_device_focus_cookie_t
 **/
typedef struct xcb_input_get_device_focus_cookie_t {
    unsigned int sequence;
} xcb_input_get_device_focus_cookie_t;

/** Opcode for xcb_input_get_device_focus. */
#define XCB_INPUT_GET_DEVICE_FOCUS 20

/**
 * @brief xcb_input_get_device_focus_request_t
 **/
typedef struct xcb_input_get_device_focus_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  pad0[3];
} xcb_input_get_device_focus_request_t;

/**
 * @brief xcb_input_get_device_focus_reply_t
 **/
typedef struct xcb_input_get_device_focus_reply_t {
    uint8_t         response_type;
    uint8_t         xi_reply_type;
    uint16_t        sequence;
    uint32_t        length;
    xcb_window_t    focus;
    xcb_timestamp_t time;
    uint8_t         revert_to;
    uint8_t         pad0[15];
} xcb_input_get_device_focus_reply_t;

/** Opcode for xcb_input_set_device_focus. */
#define XCB_INPUT_SET_DEVICE_FOCUS 21

/**
 * @brief xcb_input_set_device_focus_request_t
 **/
typedef struct xcb_input_set_device_focus_request_t {
    uint8_t         major_opcode;
    uint8_t         minor_opcode;
    uint16_t        length;
    xcb_window_t    focus;
    xcb_timestamp_t time;
    uint8_t         revert_to;
    uint8_t         device_id;
    uint8_t         pad0[2];
} xcb_input_set_device_focus_request_t;

typedef enum xcb_input_feedback_class_t {
    XCB_INPUT_FEEDBACK_CLASS_KEYBOARD = 0,
    XCB_INPUT_FEEDBACK_CLASS_POINTER = 1,
    XCB_INPUT_FEEDBACK_CLASS_STRING = 2,
    XCB_INPUT_FEEDBACK_CLASS_INTEGER = 3,
    XCB_INPUT_FEEDBACK_CLASS_LED = 4,
    XCB_INPUT_FEEDBACK_CLASS_BELL = 5
} xcb_input_feedback_class_t;

/**
 * @brief xcb_input_kbd_feedback_state_t
 **/
typedef struct xcb_input_kbd_feedback_state_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
    uint16_t pitch;
    uint16_t duration;
    uint32_t led_mask;
    uint32_t led_values;
    uint8_t  global_auto_repeat;
    uint8_t  click;
    uint8_t  percent;
    uint8_t  pad0;
    uint8_t  auto_repeats[32];
} xcb_input_kbd_feedback_state_t;

/**
 * @brief xcb_input_kbd_feedback_state_iterator_t
 **/
typedef struct xcb_input_kbd_feedback_state_iterator_t {
    xcb_input_kbd_feedback_state_t *data;
    int                             rem;
    int                             index;
} xcb_input_kbd_feedback_state_iterator_t;

/**
 * @brief xcb_input_ptr_feedback_state_t
 **/
typedef struct xcb_input_ptr_feedback_state_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
    uint8_t  pad0[2];
    uint16_t accel_num;
    uint16_t accel_denom;
    uint16_t threshold;
} xcb_input_ptr_feedback_state_t;

/**
 * @brief xcb_input_ptr_feedback_state_iterator_t
 **/
typedef struct xcb_input_ptr_feedback_state_iterator_t {
    xcb_input_ptr_feedback_state_t *data;
    int                             rem;
    int                             index;
} xcb_input_ptr_feedback_state_iterator_t;

/**
 * @brief xcb_input_integer_feedback_state_t
 **/
typedef struct xcb_input_integer_feedback_state_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
    uint32_t resolution;
    int32_t  min_value;
    int32_t  max_value;
} xcb_input_integer_feedback_state_t;

/**
 * @brief xcb_input_integer_feedback_state_iterator_t
 **/
typedef struct xcb_input_integer_feedback_state_iterator_t {
    xcb_input_integer_feedback_state_t *data;
    int                                 rem;
    int                                 index;
} xcb_input_integer_feedback_state_iterator_t;

/**
 * @brief xcb_input_string_feedback_state_t
 **/
typedef struct xcb_input_string_feedback_state_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
    uint16_t max_symbols;
    uint16_t num_keysyms;
} xcb_input_string_feedback_state_t;

/**
 * @brief xcb_input_string_feedback_state_iterator_t
 **/
typedef struct xcb_input_string_feedback_state_iterator_t {
    xcb_input_string_feedback_state_t *data;
    int                                rem;
    int                                index;
} xcb_input_string_feedback_state_iterator_t;

/**
 * @brief xcb_input_bell_feedback_state_t
 **/
typedef struct xcb_input_bell_feedback_state_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
    uint8_t  percent;
    uint8_t  pad0[3];
    uint16_t pitch;
    uint16_t duration;
} xcb_input_bell_feedback_state_t;

/**
 * @brief xcb_input_bell_feedback_state_iterator_t
 **/
typedef struct xcb_input_bell_feedback_state_iterator_t {
    xcb_input_bell_feedback_state_t *data;
    int                              rem;
    int                              index;
} xcb_input_bell_feedback_state_iterator_t;

/**
 * @brief xcb_input_led_feedback_state_t
 **/
typedef struct xcb_input_led_feedback_state_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
    uint32_t led_mask;
    uint32_t led_values;
} xcb_input_led_feedback_state_t;

/**
 * @brief xcb_input_led_feedback_state_iterator_t
 **/
typedef struct xcb_input_led_feedback_state_iterator_t {
    xcb_input_led_feedback_state_t *data;
    int                             rem;
    int                             index;
} xcb_input_led_feedback_state_iterator_t;

/**
 * @brief xcb_input_feedback_state_data_t
 **/
typedef struct xcb_input_feedback_state_data_t {
    struct {
        uint16_t      pitch;
        uint16_t      duration;
        uint32_t      led_mask;
        uint32_t      led_values;
        uint8_t       global_auto_repeat;
        uint8_t       click;
        uint8_t       percent;
        uint8_t       pad0;
        uint8_t       auto_repeats[32];
    } keyboard;
    struct {
        uint8_t       pad1[2];
        uint16_t      accel_num;
        uint16_t      accel_denom;
        uint16_t      threshold;
    } pointer;
    struct {
        uint16_t      max_symbols;
        uint16_t      num_keysyms;
        xcb_keysym_t *keysyms;
    } string;
    struct {
        uint32_t      resolution;
        int32_t       min_value;
        int32_t       max_value;
    } integer;
    struct {
        uint32_t      led_mask;
        uint32_t      led_values;
    } led;
    struct {
        uint8_t       percent;
        uint8_t       pad2[3];
        uint16_t      pitch;
        uint16_t      duration;
    } bell;
} xcb_input_feedback_state_data_t;

/**
 * @brief xcb_input_feedback_state_t
 **/
typedef struct xcb_input_feedback_state_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
} xcb_input_feedback_state_t;

void *
xcb_input_feedback_state_data (const xcb_input_feedback_state_t *R);

/**
 * @brief xcb_input_feedback_state_iterator_t
 **/
typedef struct xcb_input_feedback_state_iterator_t {
    xcb_input_feedback_state_t *data;
    int                         rem;
    int                         index;
} xcb_input_feedback_state_iterator_t;

/**
 * @brief xcb_input_get_feedback_control_cookie_t
 **/
typedef struct xcb_input_get_feedback_control_cookie_t {
    unsigned int sequence;
} xcb_input_get_feedback_control_cookie_t;

/** Opcode for xcb_input_get_feedback_control. */
#define XCB_INPUT_GET_FEEDBACK_CONTROL 22

/**
 * @brief xcb_input_get_feedback_control_request_t
 **/
typedef struct xcb_input_get_feedback_control_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  pad0[3];
} xcb_input_get_feedback_control_request_t;

/**
 * @brief xcb_input_get_feedback_control_reply_t
 **/
typedef struct xcb_input_get_feedback_control_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint16_t num_feedbacks;
    uint8_t  pad0[22];
} xcb_input_get_feedback_control_reply_t;

/**
 * @brief xcb_input_kbd_feedback_ctl_t
 **/
typedef struct xcb_input_kbd_feedback_ctl_t {
    uint8_t              class_id;
    uint8_t              feedback_id;
    uint16_t             len;
    xcb_input_key_code_t key;
    uint8_t              auto_repeat_mode;
    int8_t               key_click_percent;
    int8_t               bell_percent;
    int16_t              bell_pitch;
    int16_t              bell_duration;
    uint32_t             led_mask;
    uint32_t             led_values;
} xcb_input_kbd_feedback_ctl_t;

/**
 * @brief xcb_input_kbd_feedback_ctl_iterator_t
 **/
typedef struct xcb_input_kbd_feedback_ctl_iterator_t {
    xcb_input_kbd_feedback_ctl_t *data;
    int                           rem;
    int                           index;
} xcb_input_kbd_feedback_ctl_iterator_t;

/**
 * @brief xcb_input_ptr_feedback_ctl_t
 **/
typedef struct xcb_input_ptr_feedback_ctl_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
    uint8_t  pad0[2];
    int16_t  num;
    int16_t  denom;
    int16_t  threshold;
} xcb_input_ptr_feedback_ctl_t;

/**
 * @brief xcb_input_ptr_feedback_ctl_iterator_t
 **/
typedef struct xcb_input_ptr_feedback_ctl_iterator_t {
    xcb_input_ptr_feedback_ctl_t *data;
    int                           rem;
    int                           index;
} xcb_input_ptr_feedback_ctl_iterator_t;

/**
 * @brief xcb_input_integer_feedback_ctl_t
 **/
typedef struct xcb_input_integer_feedback_ctl_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
    int32_t  int_to_display;
} xcb_input_integer_feedback_ctl_t;

/**
 * @brief xcb_input_integer_feedback_ctl_iterator_t
 **/
typedef struct xcb_input_integer_feedback_ctl_iterator_t {
    xcb_input_integer_feedback_ctl_t *data;
    int                               rem;
    int                               index;
} xcb_input_integer_feedback_ctl_iterator_t;

/**
 * @brief xcb_input_string_feedback_ctl_t
 **/
typedef struct xcb_input_string_feedback_ctl_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
    uint8_t  pad0[2];
    uint16_t num_keysyms;
} xcb_input_string_feedback_ctl_t;

/**
 * @brief xcb_input_string_feedback_ctl_iterator_t
 **/
typedef struct xcb_input_string_feedback_ctl_iterator_t {
    xcb_input_string_feedback_ctl_t *data;
    int                              rem;
    int                              index;
} xcb_input_string_feedback_ctl_iterator_t;

/**
 * @brief xcb_input_bell_feedback_ctl_t
 **/
typedef struct xcb_input_bell_feedback_ctl_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
    int8_t   percent;
    uint8_t  pad0[3];
    int16_t  pitch;
    int16_t  duration;
} xcb_input_bell_feedback_ctl_t;

/**
 * @brief xcb_input_bell_feedback_ctl_iterator_t
 **/
typedef struct xcb_input_bell_feedback_ctl_iterator_t {
    xcb_input_bell_feedback_ctl_t *data;
    int                            rem;
    int                            index;
} xcb_input_bell_feedback_ctl_iterator_t;

/**
 * @brief xcb_input_led_feedback_ctl_t
 **/
typedef struct xcb_input_led_feedback_ctl_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
    uint32_t led_mask;
    uint32_t led_values;
} xcb_input_led_feedback_ctl_t;

/**
 * @brief xcb_input_led_feedback_ctl_iterator_t
 **/
typedef struct xcb_input_led_feedback_ctl_iterator_t {
    xcb_input_led_feedback_ctl_t *data;
    int                           rem;
    int                           index;
} xcb_input_led_feedback_ctl_iterator_t;

/**
 * @brief xcb_input_feedback_ctl_data_t
 **/
typedef struct xcb_input_feedback_ctl_data_t {
    struct {
        xcb_input_key_code_t key;
        uint8_t              auto_repeat_mode;
        int8_t               key_click_percent;
        int8_t               bell_percent;
        int16_t              bell_pitch;
        int16_t              bell_duration;
        uint32_t             led_mask;
        uint32_t             led_values;
    } keyboard;
    struct {
        uint8_t              pad0[2];
        int16_t              num;
        int16_t              denom;
        int16_t              threshold;
    } pointer;
    struct {
        uint8_t              pad1[2];
        uint16_t             num_keysyms;
        xcb_keysym_t        *keysyms;
    } string;
    struct {
        int32_t              int_to_display;
    } integer;
    struct {
        uint32_t             led_mask;
        uint32_t             led_values;
    } led;
    struct {
        int8_t               percent;
        uint8_t              pad2[3];
        int16_t              pitch;
        int16_t              duration;
    } bell;
} xcb_input_feedback_ctl_data_t;

/**
 * @brief xcb_input_feedback_ctl_t
 **/
typedef struct xcb_input_feedback_ctl_t {
    uint8_t  class_id;
    uint8_t  feedback_id;
    uint16_t len;
} xcb_input_feedback_ctl_t;

void *
xcb_input_feedback_ctl_data (const xcb_input_feedback_ctl_t *R);

/**
 * @brief xcb_input_feedback_ctl_iterator_t
 **/
typedef struct xcb_input_feedback_ctl_iterator_t {
    xcb_input_feedback_ctl_t *data;
    int                       rem;
    int                       index;
} xcb_input_feedback_ctl_iterator_t;

typedef enum xcb_input_change_feedback_control_mask_t {
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_KEY_CLICK_PERCENT = 1,
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_PERCENT = 2,
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_PITCH = 4,
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_DURATION = 8,
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_LED = 16,
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_LED_MODE = 32,
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_KEY = 64,
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_AUTO_REPEAT_MODE = 128,
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_STRING = 1,
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_INTEGER = 1,
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_ACCEL_NUM = 1,
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_ACCEL_DENOM = 2,
    XCB_INPUT_CHANGE_FEEDBACK_CONTROL_MASK_THRESHOLD = 4
} xcb_input_change_feedback_control_mask_t;

/** Opcode for xcb_input_change_feedback_control. */
#define XCB_INPUT_CHANGE_FEEDBACK_CONTROL 23

/**
 * @brief xcb_input_change_feedback_control_request_t
 **/
typedef struct xcb_input_change_feedback_control_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint32_t mask;
    uint8_t  device_id;
    uint8_t  feedback_id;
    uint8_t  pad0[2];
} xcb_input_change_feedback_control_request_t;

/**
 * @brief xcb_input_get_device_key_mapping_cookie_t
 **/
typedef struct xcb_input_get_device_key_mapping_cookie_t {
    unsigned int sequence;
} xcb_input_get_device_key_mapping_cookie_t;

/** Opcode for xcb_input_get_device_key_mapping. */
#define XCB_INPUT_GET_DEVICE_KEY_MAPPING 24

/**
 * @brief xcb_input_get_device_key_mapping_request_t
 **/
typedef struct xcb_input_get_device_key_mapping_request_t {
    uint8_t              major_opcode;
    uint8_t              minor_opcode;
    uint16_t             length;
    uint8_t              device_id;
    xcb_input_key_code_t first_keycode;
    uint8_t              count;
    uint8_t              pad0;
} xcb_input_get_device_key_mapping_request_t;

/**
 * @brief xcb_input_get_device_key_mapping_reply_t
 **/
typedef struct xcb_input_get_device_key_mapping_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  keysyms_per_keycode;
    uint8_t  pad0[23];
} xcb_input_get_device_key_mapping_reply_t;

/** Opcode for xcb_input_change_device_key_mapping. */
#define XCB_INPUT_CHANGE_DEVICE_KEY_MAPPING 25

/**
 * @brief xcb_input_change_device_key_mapping_request_t
 **/
typedef struct xcb_input_change_device_key_mapping_request_t {
    uint8_t              major_opcode;
    uint8_t              minor_opcode;
    uint16_t             length;
    uint8_t              device_id;
    xcb_input_key_code_t first_keycode;
    uint8_t              keysyms_per_keycode;
    uint8_t              keycode_count;
} xcb_input_change_device_key_mapping_request_t;

/**
 * @brief xcb_input_get_device_modifier_mapping_cookie_t
 **/
typedef struct xcb_input_get_device_modifier_mapping_cookie_t {
    unsigned int sequence;
} xcb_input_get_device_modifier_mapping_cookie_t;

/** Opcode for xcb_input_get_device_modifier_mapping. */
#define XCB_INPUT_GET_DEVICE_MODIFIER_MAPPING 26

/**
 * @brief xcb_input_get_device_modifier_mapping_request_t
 **/
typedef struct xcb_input_get_device_modifier_mapping_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  pad0[3];
} xcb_input_get_device_modifier_mapping_request_t;

/**
 * @brief xcb_input_get_device_modifier_mapping_reply_t
 **/
typedef struct xcb_input_get_device_modifier_mapping_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  keycodes_per_modifier;
    uint8_t  pad0[23];
} xcb_input_get_device_modifier_mapping_reply_t;

/**
 * @brief xcb_input_set_device_modifier_mapping_cookie_t
 **/
typedef struct xcb_input_set_device_modifier_mapping_cookie_t {
    unsigned int sequence;
} xcb_input_set_device_modifier_mapping_cookie_t;

/** Opcode for xcb_input_set_device_modifier_mapping. */
#define XCB_INPUT_SET_DEVICE_MODIFIER_MAPPING 27

/**
 * @brief xcb_input_set_device_modifier_mapping_request_t
 **/
typedef struct xcb_input_set_device_modifier_mapping_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  keycodes_per_modifier;
    uint8_t  pad0[2];
} xcb_input_set_device_modifier_mapping_request_t;

/**
 * @brief xcb_input_set_device_modifier_mapping_reply_t
 **/
typedef struct xcb_input_set_device_modifier_mapping_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  status;
    uint8_t  pad0[23];
} xcb_input_set_device_modifier_mapping_reply_t;

/**
 * @brief xcb_input_get_device_button_mapping_cookie_t
 **/
typedef struct xcb_input_get_device_button_mapping_cookie_t {
    unsigned int sequence;
} xcb_input_get_device_button_mapping_cookie_t;

/** Opcode for xcb_input_get_device_button_mapping. */
#define XCB_INPUT_GET_DEVICE_BUTTON_MAPPING 28

/**
 * @brief xcb_input_get_device_button_mapping_request_t
 **/
typedef struct xcb_input_get_device_button_mapping_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  pad0[3];
} xcb_input_get_device_button_mapping_request_t;

/**
 * @brief xcb_input_get_device_button_mapping_reply_t
 **/
typedef struct xcb_input_get_device_button_mapping_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  map_size;
    uint8_t  pad0[23];
} xcb_input_get_device_button_mapping_reply_t;

/**
 * @brief xcb_input_set_device_button_mapping_cookie_t
 **/
typedef struct xcb_input_set_device_button_mapping_cookie_t {
    unsigned int sequence;
} xcb_input_set_device_button_mapping_cookie_t;

/** Opcode for xcb_input_set_device_button_mapping. */
#define XCB_INPUT_SET_DEVICE_BUTTON_MAPPING 29

/**
 * @brief xcb_input_set_device_button_mapping_request_t
 **/
typedef struct xcb_input_set_device_button_mapping_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  map_size;
    uint8_t  pad0[2];
} xcb_input_set_device_button_mapping_request_t;

/**
 * @brief xcb_input_set_device_button_mapping_reply_t
 **/
typedef struct xcb_input_set_device_button_mapping_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  status;
    uint8_t  pad0[23];
} xcb_input_set_device_button_mapping_reply_t;

/**
 * @brief xcb_input_key_state_t
 **/
typedef struct xcb_input_key_state_t {
    uint8_t class_id;
    uint8_t len;
    uint8_t num_keys;
    uint8_t pad0;
    uint8_t keys[32];
} xcb_input_key_state_t;

/**
 * @brief xcb_input_key_state_iterator_t
 **/
typedef struct xcb_input_key_state_iterator_t {
    xcb_input_key_state_t *data;
    int                    rem;
    int                    index;
} xcb_input_key_state_iterator_t;

/**
 * @brief xcb_input_button_state_t
 **/
typedef struct xcb_input_button_state_t {
    uint8_t class_id;
    uint8_t len;
    uint8_t num_buttons;
    uint8_t pad0;
    uint8_t buttons[32];
} xcb_input_button_state_t;

/**
 * @brief xcb_input_button_state_iterator_t
 **/
typedef struct xcb_input_button_state_iterator_t {
    xcb_input_button_state_t *data;
    int                       rem;
    int                       index;
} xcb_input_button_state_iterator_t;

typedef enum xcb_input_valuator_state_mode_mask_t {
    XCB_INPUT_VALUATOR_STATE_MODE_MASK_DEVICE_MODE_ABSOLUTE = 1,
    XCB_INPUT_VALUATOR_STATE_MODE_MASK_OUT_OF_PROXIMITY = 2
} xcb_input_valuator_state_mode_mask_t;

/**
 * @brief xcb_input_valuator_state_t
 **/
typedef struct xcb_input_valuator_state_t {
    uint8_t class_id;
    uint8_t len;
    uint8_t num_valuators;
    uint8_t mode;
} xcb_input_valuator_state_t;

/**
 * @brief xcb_input_valuator_state_iterator_t
 **/
typedef struct xcb_input_valuator_state_iterator_t {
    xcb_input_valuator_state_t *data;
    int                         rem;
    int                         index;
} xcb_input_valuator_state_iterator_t;

/**
 * @brief xcb_input_input_state_data_t
 **/
typedef struct xcb_input_input_state_data_t {
    struct {
        uint8_t  num_keys;
        uint8_t  pad0;
        uint8_t  keys[32];
    } key;
    struct {
        uint8_t  num_buttons;
        uint8_t  pad1;
        uint8_t  buttons[32];
    } button;
    struct {
        uint8_t  num_valuators;
        uint8_t  mode;
        int32_t *valuators;
    } valuator;
} xcb_input_input_state_data_t;

/**
 * @brief xcb_input_input_state_t
 **/
typedef struct xcb_input_input_state_t {
    uint8_t class_id;
    uint8_t len;
} xcb_input_input_state_t;

void *
xcb_input_input_state_data (const xcb_input_input_state_t *R);

/**
 * @brief xcb_input_input_state_iterator_t
 **/
typedef struct xcb_input_input_state_iterator_t {
    xcb_input_input_state_t *data;
    int                      rem;
    int                      index;
} xcb_input_input_state_iterator_t;

/**
 * @brief xcb_input_query_device_state_cookie_t
 **/
typedef struct xcb_input_query_device_state_cookie_t {
    unsigned int sequence;
} xcb_input_query_device_state_cookie_t;

/** Opcode for xcb_input_query_device_state. */
#define XCB_INPUT_QUERY_DEVICE_STATE 30

/**
 * @brief xcb_input_query_device_state_request_t
 **/
typedef struct xcb_input_query_device_state_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  pad0[3];
} xcb_input_query_device_state_request_t;

/**
 * @brief xcb_input_query_device_state_reply_t
 **/
typedef struct xcb_input_query_device_state_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  num_classes;
    uint8_t  pad0[23];
} xcb_input_query_device_state_reply_t;

/** Opcode for xcb_input_device_bell. */
#define XCB_INPUT_DEVICE_BELL 32

/**
 * @brief xcb_input_device_bell_request_t
 **/
typedef struct xcb_input_device_bell_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  feedback_id;
    uint8_t  feedback_class;
    int8_t   percent;
} xcb_input_device_bell_request_t;

/**
 * @brief xcb_input_set_device_valuators_cookie_t
 **/
typedef struct xcb_input_set_device_valuators_cookie_t {
    unsigned int sequence;
} xcb_input_set_device_valuators_cookie_t;

/** Opcode for xcb_input_set_device_valuators. */
#define XCB_INPUT_SET_DEVICE_VALUATORS 33

/**
 * @brief xcb_input_set_device_valuators_request_t
 **/
typedef struct xcb_input_set_device_valuators_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  first_valuator;
    uint8_t  num_valuators;
    uint8_t  pad0;
} xcb_input_set_device_valuators_request_t;

/**
 * @brief xcb_input_set_device_valuators_reply_t
 **/
typedef struct xcb_input_set_device_valuators_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  status;
    uint8_t  pad0[23];
} xcb_input_set_device_valuators_reply_t;

typedef enum xcb_input_device_control_t {
    XCB_INPUT_DEVICE_CONTROL_RESOLUTION = 1,
    XCB_INPUT_DEVICE_CONTROL_ABS_CALIB = 2,
    XCB_INPUT_DEVICE_CONTROL_CORE = 3,
    XCB_INPUT_DEVICE_CONTROL_ENABLE = 4,
    XCB_INPUT_DEVICE_CONTROL_ABS_AREA = 5
} xcb_input_device_control_t;

/**
 * @brief xcb_input_device_resolution_state_t
 **/
typedef struct xcb_input_device_resolution_state_t {
    uint16_t control_id;
    uint16_t len;
    uint32_t num_valuators;
} xcb_input_device_resolution_state_t;

/**
 * @brief xcb_input_device_resolution_state_iterator_t
 **/
typedef struct xcb_input_device_resolution_state_iterator_t {
    xcb_input_device_resolution_state_t *data;
    int                                  rem;
    int                                  index;
} xcb_input_device_resolution_state_iterator_t;

/**
 * @brief xcb_input_device_abs_calib_state_t
 **/
typedef struct xcb_input_device_abs_calib_state_t {
    uint16_t control_id;
    uint16_t len;
    int32_t  min_x;
    int32_t  max_x;
    int32_t  min_y;
    int32_t  max_y;
    uint32_t flip_x;
    uint32_t flip_y;
    uint32_t rotation;
    uint32_t button_threshold;
} xcb_input_device_abs_calib_state_t;

/**
 * @brief xcb_input_device_abs_calib_state_iterator_t
 **/
typedef struct xcb_input_device_abs_calib_state_iterator_t {
    xcb_input_device_abs_calib_state_t *data;
    int                                 rem;
    int                                 index;
} xcb_input_device_abs_calib_state_iterator_t;

/**
 * @brief xcb_input_device_abs_area_state_t
 **/
typedef struct xcb_input_device_abs_area_state_t {
    uint16_t control_id;
    uint16_t len;
    uint32_t offset_x;
    uint32_t offset_y;
    uint32_t width;
    uint32_t height;
    uint32_t screen;
    uint32_t following;
} xcb_input_device_abs_area_state_t;

/**
 * @brief xcb_input_device_abs_area_state_iterator_t
 **/
typedef struct xcb_input_device_abs_area_state_iterator_t {
    xcb_input_device_abs_area_state_t *data;
    int                                rem;
    int                                index;
} xcb_input_device_abs_area_state_iterator_t;

/**
 * @brief xcb_input_device_core_state_t
 **/
typedef struct xcb_input_device_core_state_t {
    uint16_t control_id;
    uint16_t len;
    uint8_t  status;
    uint8_t  iscore;
    uint8_t  pad0[2];
} xcb_input_device_core_state_t;

/**
 * @brief xcb_input_device_core_state_iterator_t
 **/
typedef struct xcb_input_device_core_state_iterator_t {
    xcb_input_device_core_state_t *data;
    int                            rem;
    int                            index;
} xcb_input_device_core_state_iterator_t;

/**
 * @brief xcb_input_device_enable_state_t
 **/
typedef struct xcb_input_device_enable_state_t {
    uint16_t control_id;
    uint16_t len;
    uint8_t  enable;
    uint8_t  pad0[3];
} xcb_input_device_enable_state_t;

/**
 * @brief xcb_input_device_enable_state_iterator_t
 **/
typedef struct xcb_input_device_enable_state_iterator_t {
    xcb_input_device_enable_state_t *data;
    int                              rem;
    int                              index;
} xcb_input_device_enable_state_iterator_t;

/**
 * @brief xcb_input_device_state_data_t
 **/
typedef struct xcb_input_device_state_data_t {
    struct {
        uint32_t  num_valuators;
        uint32_t *resolution_values;
        uint32_t *resolution_min;
        uint32_t *resolution_max;
    } resolution;
    struct {
        int32_t   min_x;
        int32_t   max_x;
        int32_t   min_y;
        int32_t   max_y;
        uint32_t  flip_x;
        uint32_t  flip_y;
        uint32_t  rotation;
        uint32_t  button_threshold;
    } abs_calib;
    struct {
        uint8_t   status;
        uint8_t   iscore;
        uint8_t   pad0[2];
    } core;
    struct {
        uint8_t   enable;
        uint8_t   pad1[3];
    } enable;
    struct {
        uint32_t  offset_x;
        uint32_t  offset_y;
        uint32_t  width;
        uint32_t  height;
        uint32_t  screen;
        uint32_t  following;
    } abs_area;
} xcb_input_device_state_data_t;

/**
 * @brief xcb_input_device_state_t
 **/
typedef struct xcb_input_device_state_t {
    uint16_t control_id;
    uint16_t len;
} xcb_input_device_state_t;

void *
xcb_input_device_state_data (const xcb_input_device_state_t *R);

/**
 * @brief xcb_input_device_state_iterator_t
 **/
typedef struct xcb_input_device_state_iterator_t {
    xcb_input_device_state_t *data;
    int                       rem;
    int                       index;
} xcb_input_device_state_iterator_t;

/**
 * @brief xcb_input_get_device_control_cookie_t
 **/
typedef struct xcb_input_get_device_control_cookie_t {
    unsigned int sequence;
} xcb_input_get_device_control_cookie_t;

/** Opcode for xcb_input_get_device_control. */
#define XCB_INPUT_GET_DEVICE_CONTROL 34

/**
 * @brief xcb_input_get_device_control_request_t
 **/
typedef struct xcb_input_get_device_control_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint16_t control_id;
    uint8_t  device_id;
    uint8_t  pad0;
} xcb_input_get_device_control_request_t;

/**
 * @brief xcb_input_get_device_control_reply_t
 **/
typedef struct xcb_input_get_device_control_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  status;
    uint8_t  pad0[23];
} xcb_input_get_device_control_reply_t;

/**
 * @brief xcb_input_device_resolution_ctl_t
 **/
typedef struct xcb_input_device_resolution_ctl_t {
    uint16_t control_id;
    uint16_t len;
    uint8_t  first_valuator;
    uint8_t  num_valuators;
    uint8_t  pad0[2];
} xcb_input_device_resolution_ctl_t;

/**
 * @brief xcb_input_device_resolution_ctl_iterator_t
 **/
typedef struct xcb_input_device_resolution_ctl_iterator_t {
    xcb_input_device_resolution_ctl_t *data;
    int                                rem;
    int                                index;
} xcb_input_device_resolution_ctl_iterator_t;

/**
 * @brief xcb_input_device_abs_calib_ctl_t
 **/
typedef struct xcb_input_device_abs_calib_ctl_t {
    uint16_t control_id;
    uint16_t len;
    int32_t  min_x;
    int32_t  max_x;
    int32_t  min_y;
    int32_t  max_y;
    uint32_t flip_x;
    uint32_t flip_y;
    uint32_t rotation;
    uint32_t button_threshold;
} xcb_input_device_abs_calib_ctl_t;

/**
 * @brief xcb_input_device_abs_calib_ctl_iterator_t
 **/
typedef struct xcb_input_device_abs_calib_ctl_iterator_t {
    xcb_input_device_abs_calib_ctl_t *data;
    int                               rem;
    int                               index;
} xcb_input_device_abs_calib_ctl_iterator_t;

/**
 * @brief xcb_input_device_abs_area_ctrl_t
 **/
typedef struct xcb_input_device_abs_area_ctrl_t {
    uint16_t control_id;
    uint16_t len;
    uint32_t offset_x;
    uint32_t offset_y;
    int32_t  width;
    int32_t  height;
    int32_t  screen;
    uint32_t following;
} xcb_input_device_abs_area_ctrl_t;

/**
 * @brief xcb_input_device_abs_area_ctrl_iterator_t
 **/
typedef struct xcb_input_device_abs_area_ctrl_iterator_t {
    xcb_input_device_abs_area_ctrl_t *data;
    int                               rem;
    int                               index;
} xcb_input_device_abs_area_ctrl_iterator_t;

/**
 * @brief xcb_input_device_core_ctrl_t
 **/
typedef struct xcb_input_device_core_ctrl_t {
    uint16_t control_id;
    uint16_t len;
    uint8_t  status;
    uint8_t  pad0[3];
} xcb_input_device_core_ctrl_t;

/**
 * @brief xcb_input_device_core_ctrl_iterator_t
 **/
typedef struct xcb_input_device_core_ctrl_iterator_t {
    xcb_input_device_core_ctrl_t *data;
    int                           rem;
    int                           index;
} xcb_input_device_core_ctrl_iterator_t;

/**
 * @brief xcb_input_device_enable_ctrl_t
 **/
typedef struct xcb_input_device_enable_ctrl_t {
    uint16_t control_id;
    uint16_t len;
    uint8_t  enable;
    uint8_t  pad0[3];
} xcb_input_device_enable_ctrl_t;

/**
 * @brief xcb_input_device_enable_ctrl_iterator_t
 **/
typedef struct xcb_input_device_enable_ctrl_iterator_t {
    xcb_input_device_enable_ctrl_t *data;
    int                             rem;
    int                             index;
} xcb_input_device_enable_ctrl_iterator_t;

/**
 * @brief xcb_input_device_ctl_data_t
 **/
typedef struct xcb_input_device_ctl_data_t {
    struct {
        uint8_t   first_valuator;
        uint8_t   num_valuators;
        uint8_t   pad0[2];
        uint32_t *resolution_values;
    } resolution;
    struct {
        int32_t   min_x;
        int32_t   max_x;
        int32_t   min_y;
        int32_t   max_y;
        uint32_t  flip_x;
        uint32_t  flip_y;
        uint32_t  rotation;
        uint32_t  button_threshold;
    } abs_calib;
    struct {
        uint8_t   status;
        uint8_t   pad1[3];
    } core;
    struct {
        uint8_t   enable;
        uint8_t   pad2[3];
    } enable;
    struct {
        uint32_t  offset_x;
        uint32_t  offset_y;
        int32_t   width;
        int32_t   height;
        int32_t   screen;
        uint32_t  following;
    } abs_area;
} xcb_input_device_ctl_data_t;

/**
 * @brief xcb_input_device_ctl_t
 **/
typedef struct xcb_input_device_ctl_t {
    uint16_t control_id;
    uint16_t len;
} xcb_input_device_ctl_t;

void *
xcb_input_device_ctl_data (const xcb_input_device_ctl_t *R);

/**
 * @brief xcb_input_device_ctl_iterator_t
 **/
typedef struct xcb_input_device_ctl_iterator_t {
    xcb_input_device_ctl_t *data;
    int                     rem;
    int                     index;
} xcb_input_device_ctl_iterator_t;

/**
 * @brief xcb_input_change_device_control_cookie_t
 **/
typedef struct xcb_input_change_device_control_cookie_t {
    unsigned int sequence;
} xcb_input_change_device_control_cookie_t;

/** Opcode for xcb_input_change_device_control. */
#define XCB_INPUT_CHANGE_DEVICE_CONTROL 35

/**
 * @brief xcb_input_change_device_control_request_t
 **/
typedef struct xcb_input_change_device_control_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint16_t control_id;
    uint8_t  device_id;
    uint8_t  pad0;
} xcb_input_change_device_control_request_t;

/**
 * @brief xcb_input_change_device_control_reply_t
 **/
typedef struct xcb_input_change_device_control_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint8_t  status;
    uint8_t  pad0[23];
} xcb_input_change_device_control_reply_t;

/**
 * @brief xcb_input_list_device_properties_cookie_t
 **/
typedef struct xcb_input_list_device_properties_cookie_t {
    unsigned int sequence;
} xcb_input_list_device_properties_cookie_t;

/** Opcode for xcb_input_list_device_properties. */
#define XCB_INPUT_LIST_DEVICE_PROPERTIES 36

/**
 * @brief xcb_input_list_device_properties_request_t
 **/
typedef struct xcb_input_list_device_properties_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  device_id;
    uint8_t  pad0[3];
} xcb_input_list_device_properties_request_t;

/**
 * @brief xcb_input_list_device_properties_reply_t
 **/
typedef struct xcb_input_list_device_properties_reply_t {
    uint8_t  response_type;
    uint8_t  xi_reply_type;
    uint16_t sequence;
    uint32_t length;
    uint16_t num_atoms;
    uint8_t  pad0[22];
} xcb_input_list_device_properties_reply_t;

typedef enum xcb_input_property_format_t {
    XCB_INPUT_PROPERTY_FORMAT_8_BITS = 8,
    XCB_INPUT_PROPERTY_FORMAT_16_BITS = 16,
    XCB_INPUT_PROPERTY_FORMAT_32_BITS = 32
} xcb_input_property_format_t;

/**
 * @brief xcb_input_change_device_property_items_t
 **/
typedef struct xcb_input_change_device_property_items_t {
    uint8_t  *data8;
    uint16_t *data16;
    uint32_t *data32;
} xcb_input_change_device_property_items_t;

/** Opcode for xcb_input_change_device_property. */
#define XCB_INPUT_CHANGE_DEVICE_PROPERTY 37

/**
 * @brief xcb_input_change_device_property_request_t
 **/
typedef struct xcb_input_change_device_property_request_t {
    uint8_t    major_opcode;
    uint8_t    minor_opcode;
    uint16_t   length;
    xcb_atom_t property;
    xcb_atom_t type;
    uint8_t    device_id;
    uint8_t    format;
    uint8_t    mode;
    uint8_t    pad0;
    uint32_t   num_items;
} xcb_input_change_device_property_request_t;

/** Opcode for xcb_input_delete_device_property. */
#define XCB_INPUT_DELETE_DEVICE_PROPERTY 38

/**
 * @brief xcb_input_delete_device_property_request_t
 **/
typedef struct xcb_input_delete_device_property_request_t {
    uint8_t    major_opcode;
    uint8_t    minor_opcode;
    uint16_t   length;
    xcb_atom_t property;
    uint8_t    device_id;
    uint8_t    pad0[3];
} xcb_input_delete_device_property_request_t;

/**
 * @brief xcb_input_get_device_property_cookie_t
 **/
typedef struct xcb_input_get_device_property_cookie_t {
    unsigned int sequence;
} xcb_input_get_device_property_cookie_t;

/** Opcode for xcb_input_get_device_property. */
#define XCB_INPUT_GET_DEVICE_PROPERTY 39

/**
 * @brief xcb_input_get_device_property_request_t
 **/
typedef struct xcb_input_get_device_property_request_t {
    uint8_t    major_opcode;
    uint8_t    minor_opcode;
    uint16_t   length;
    xcb_atom_t property;
    xcb_atom_t type;
    uint32_t   offset;
    uint32_t   len;
    uint8_t    device_id;
    uint8_t    _delete;
    uint8_t    pad0[2];
} xcb_input_get_device_property_request_t;

/**
 * @brief xcb_input_get_device_property_items_t
 **/
typedef struct xcb_input_get_device_property_items_t {
    uint8_t  *data8;
    uint16_t *data16;
    uint32_t *data32;
} xcb_input_get_device_property_items_t;

/**
 * @brief xcb_input_get_device_property_reply_t
 **/
typedef struct xcb_input_get_device_property_reply_t {
    uint8_t    response_type;
    uint8_t    xi_reply_type;
    uint16_t   sequence;
    uint32_t   length;
    xcb_atom_t type;
    uint32_t   bytes_after;
    uint32_t   num_items;
    uint8_t    format;
    uint8_t    device_id;
    uint8_t    pad0[10];
} xcb_input_get_device_property_reply_t;

typedef enum xcb_input_device_t {
    XCB_INPUT_DEVICE_ALL = 0,
    XCB_INPUT_DEVICE_ALL_MASTER = 1
} xcb_input_device_t;

/**
 * @brief xcb_input_group_info_t
 **/
typedef struct xcb_input_group_info_t {
    uint8_t base;
    uint8_t latched;
    uint8_t locked;
    uint8_t effective;
} xcb_input_group_info_t;

/**
 * @brief xcb_input_group_info_iterator_t
 **/
typedef struct xcb_input_group_info_iterator_t {
    xcb_input_group_info_t *data;
    int                     rem;
    int                     index;
} xcb_input_group_info_iterator_t;

/**
 * @brief xcb_input_modifier_info_t
 **/
typedef struct xcb_input_modifier_info_t {
    uint32_t base;
    uint32_t latched;
    uint32_t locked;
    uint32_t effective;
} xcb_input_modifier_info_t;

/**
 * @brief xcb_input_modifier_info_iterator_t
 **/
typedef struct xcb_input_modifier_info_iterator_t {
    xcb_input_modifier_info_t *data;
    int                        rem;
    int                        index;
} xcb_input_modifier_info_iterator_t;

/**
 * @brief xcb_input_xi_query_pointer_cookie_t
 **/
typedef struct xcb_input_xi_query_pointer_cookie_t {
    unsigned int sequence;
} xcb_input_xi_query_pointer_cookie_t;

/** Opcode for xcb_input_xi_query_pointer. */
#define XCB_INPUT_XI_QUERY_POINTER 40

/**
 * @brief xcb_input_xi_query_pointer_request_t
 **/
typedef struct xcb_input_xi_query_pointer_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_window_t          window;
    xcb_input_device_id_t deviceid;
    uint8_t               pad0[2];
} xcb_input_xi_query_pointer_request_t;

/**
 * @brief xcb_input_xi_query_pointer_reply_t
 **/
typedef struct xcb_input_xi_query_pointer_reply_t {
    uint8_t                   response_type;
    uint8_t                   pad0;
    uint16_t                  sequence;
    uint32_t                  length;
    xcb_window_t              root;
    xcb_window_t              child;
    xcb_input_fp1616_t        root_x;
    xcb_input_fp1616_t        root_y;
    xcb_input_fp1616_t        win_x;
    xcb_input_fp1616_t        win_y;
    uint8_t                   same_screen;
    uint8_t                   pad1;
    uint16_t                  buttons_len;
    xcb_input_modifier_info_t mods;
    xcb_input_group_info_t    group;
} xcb_input_xi_query_pointer_reply_t;

/** Opcode for xcb_input_xi_warp_pointer. */
#define XCB_INPUT_XI_WARP_POINTER 41

/**
 * @brief xcb_input_xi_warp_pointer_request_t
 **/
typedef struct xcb_input_xi_warp_pointer_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_window_t          src_win;
    xcb_window_t          dst_win;
    xcb_input_fp1616_t    src_x;
    xcb_input_fp1616_t    src_y;
    uint16_t              src_width;
    uint16_t              src_height;
    xcb_input_fp1616_t    dst_x;
    xcb_input_fp1616_t    dst_y;
    xcb_input_device_id_t deviceid;
    uint8_t               pad0[2];
} xcb_input_xi_warp_pointer_request_t;

/** Opcode for xcb_input_xi_change_cursor. */
#define XCB_INPUT_XI_CHANGE_CURSOR 42

/**
 * @brief xcb_input_xi_change_cursor_request_t
 **/
typedef struct xcb_input_xi_change_cursor_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_window_t          window;
    xcb_cursor_t          cursor;
    xcb_input_device_id_t deviceid;
    uint8_t               pad0[2];
} xcb_input_xi_change_cursor_request_t;

typedef enum xcb_input_hierarchy_change_type_t {
    XCB_INPUT_HIERARCHY_CHANGE_TYPE_ADD_MASTER = 1,
    XCB_INPUT_HIERARCHY_CHANGE_TYPE_REMOVE_MASTER = 2,
    XCB_INPUT_HIERARCHY_CHANGE_TYPE_ATTACH_SLAVE = 3,
    XCB_INPUT_HIERARCHY_CHANGE_TYPE_DETACH_SLAVE = 4
} xcb_input_hierarchy_change_type_t;

typedef enum xcb_input_change_mode_t {
    XCB_INPUT_CHANGE_MODE_ATTACH = 1,
    XCB_INPUT_CHANGE_MODE_FLOAT = 2
} xcb_input_change_mode_t;

/**
 * @brief xcb_input_add_master_t
 **/
typedef struct xcb_input_add_master_t {
    uint16_t type;
    uint16_t len;
    uint16_t name_len;
    uint8_t  send_core;
    uint8_t  enable;
} xcb_input_add_master_t;

/**
 * @brief xcb_input_add_master_iterator_t
 **/
typedef struct xcb_input_add_master_iterator_t {
    xcb_input_add_master_t *data;
    int                     rem;
    int                     index;
} xcb_input_add_master_iterator_t;

/**
 * @brief xcb_input_remove_master_t
 **/
typedef struct xcb_input_remove_master_t {
    uint16_t              type;
    uint16_t              len;
    xcb_input_device_id_t deviceid;
    uint8_t               return_mode;
    uint8_t               pad0;
    xcb_input_device_id_t return_pointer;
    xcb_input_device_id_t return_keyboard;
} xcb_input_remove_master_t;

/**
 * @brief xcb_input_remove_master_iterator_t
 **/
typedef struct xcb_input_remove_master_iterator_t {
    xcb_input_remove_master_t *data;
    int                        rem;
    int                        index;
} xcb_input_remove_master_iterator_t;

/**
 * @brief xcb_input_attach_slave_t
 **/
typedef struct xcb_input_attach_slave_t {
    uint16_t              type;
    uint16_t              len;
    xcb_input_device_id_t deviceid;
    xcb_input_device_id_t master;
} xcb_input_attach_slave_t;

/**
 * @brief xcb_input_attach_slave_iterator_t
 **/
typedef struct xcb_input_attach_slave_iterator_t {
    xcb_input_attach_slave_t *data;
    int                       rem;
    int                       index;
} xcb_input_attach_slave_iterator_t;

/**
 * @brief xcb_input_detach_slave_t
 **/
typedef struct xcb_input_detach_slave_t {
    uint16_t              type;
    uint16_t              len;
    xcb_input_device_id_t deviceid;
    uint8_t               pad0[2];
} xcb_input_detach_slave_t;

/**
 * @brief xcb_input_detach_slave_iterator_t
 **/
typedef struct xcb_input_detach_slave_iterator_t {
    xcb_input_detach_slave_t *data;
    int                       rem;
    int                       index;
} xcb_input_detach_slave_iterator_t;

/**
 * @brief xcb_input_hierarchy_change_data_t
 **/
typedef struct xcb_input_hierarchy_change_data_t {
    struct {
        uint16_t              name_len;
        uint8_t               send_core;
        uint8_t               enable;
        char                 *name;
    } add_master;
    struct {
        xcb_input_device_id_t deviceid;
        uint8_t               return_mode;
        uint8_t               pad1;
        xcb_input_device_id_t return_pointer;
        xcb_input_device_id_t return_keyboard;
    } remove_master;
    struct {
        xcb_input_device_id_t deviceid;
        xcb_input_device_id_t master;
    } attach_slave;
    struct {
        xcb_input_device_id_t deviceid;
        uint8_t               pad2[2];
    } detach_slave;
} xcb_input_hierarchy_change_data_t;

/**
 * @brief xcb_input_hierarchy_change_t
 **/
typedef struct xcb_input_hierarchy_change_t {
    uint16_t type;
    uint16_t len;
} xcb_input_hierarchy_change_t;

void *
xcb_input_hierarchy_change_data (const xcb_input_hierarchy_change_t *R);

/**
 * @brief xcb_input_hierarchy_change_iterator_t
 **/
typedef struct xcb_input_hierarchy_change_iterator_t {
    xcb_input_hierarchy_change_t *data;
    int                           rem;
    int                           index;
} xcb_input_hierarchy_change_iterator_t;

/** Opcode for xcb_input_xi_change_hierarchy. */
#define XCB_INPUT_XI_CHANGE_HIERARCHY 43

/**
 * @brief xcb_input_xi_change_hierarchy_request_t
 **/
typedef struct xcb_input_xi_change_hierarchy_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint8_t  num_changes;
    uint8_t  pad0[3];
} xcb_input_xi_change_hierarchy_request_t;

/** Opcode for xcb_input_xi_set_client_pointer. */
#define XCB_INPUT_XI_SET_CLIENT_POINTER 44

/**
 * @brief xcb_input_xi_set_client_pointer_request_t
 **/
typedef struct xcb_input_xi_set_client_pointer_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_window_t          window;
    xcb_input_device_id_t deviceid;
    uint8_t               pad0[2];
} xcb_input_xi_set_client_pointer_request_t;

/**
 * @brief xcb_input_xi_get_client_pointer_cookie_t
 **/
typedef struct xcb_input_xi_get_client_pointer_cookie_t {
    unsigned int sequence;
} xcb_input_xi_get_client_pointer_cookie_t;

/** Opcode for xcb_input_xi_get_client_pointer. */
#define XCB_INPUT_XI_GET_CLIENT_POINTER 45

/**
 * @brief xcb_input_xi_get_client_pointer_request_t
 **/
typedef struct xcb_input_xi_get_client_pointer_request_t {
    uint8_t      major_opcode;
    uint8_t      minor_opcode;
    uint16_t     length;
    xcb_window_t window;
} xcb_input_xi_get_client_pointer_request_t;

/**
 * @brief xcb_input_xi_get_client_pointer_reply_t
 **/
typedef struct xcb_input_xi_get_client_pointer_reply_t {
    uint8_t               response_type;
    uint8_t               pad0;
    uint16_t              sequence;
    uint32_t              length;
    uint8_t               set;
    uint8_t               pad1;
    xcb_input_device_id_t deviceid;
    uint8_t               pad2[20];
} xcb_input_xi_get_client_pointer_reply_t;

typedef enum xcb_input_xi_event_mask_t {
    XCB_INPUT_XI_EVENT_MASK_DEVICE_CHANGED = 2,
    XCB_INPUT_XI_EVENT_MASK_KEY_PRESS = 4,
    XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE = 8,
    XCB_INPUT_XI_EVENT_MASK_BUTTON_PRESS = 16,
    XCB_INPUT_XI_EVENT_MASK_BUTTON_RELEASE = 32,
    XCB_INPUT_XI_EVENT_MASK_MOTION = 64,
    XCB_INPUT_XI_EVENT_MASK_ENTER = 128,
    XCB_INPUT_XI_EVENT_MASK_LEAVE = 256,
    XCB_INPUT_XI_EVENT_MASK_FOCUS_IN = 512,
    XCB_INPUT_XI_EVENT_MASK_FOCUS_OUT = 1024,
    XCB_INPUT_XI_EVENT_MASK_HIERARCHY = 2048,
    XCB_INPUT_XI_EVENT_MASK_PROPERTY = 4096,
    XCB_INPUT_XI_EVENT_MASK_RAW_KEY_PRESS = 8192,
    XCB_INPUT_XI_EVENT_MASK_RAW_KEY_RELEASE = 16384,
    XCB_INPUT_XI_EVENT_MASK_RAW_BUTTON_PRESS = 32768,
    XCB_INPUT_XI_EVENT_MASK_RAW_BUTTON_RELEASE = 65536,
    XCB_INPUT_XI_EVENT_MASK_RAW_MOTION = 131072,
    XCB_INPUT_XI_EVENT_MASK_TOUCH_BEGIN = 262144,
    XCB_INPUT_XI_EVENT_MASK_TOUCH_UPDATE = 524288,
    XCB_INPUT_XI_EVENT_MASK_TOUCH_END = 1048576,
    XCB_INPUT_XI_EVENT_MASK_TOUCH_OWNERSHIP = 2097152,
    XCB_INPUT_XI_EVENT_MASK_RAW_TOUCH_BEGIN = 4194304,
    XCB_INPUT_XI_EVENT_MASK_RAW_TOUCH_UPDATE = 8388608,
    XCB_INPUT_XI_EVENT_MASK_RAW_TOUCH_END = 16777216,
    XCB_INPUT_XI_EVENT_MASK_BARRIER_HIT = 33554432,
    XCB_INPUT_XI_EVENT_MASK_BARRIER_LEAVE = 67108864
} xcb_input_xi_event_mask_t;

/**
 * @brief xcb_input_event_mask_t
 **/
typedef struct xcb_input_event_mask_t {
    xcb_input_device_id_t deviceid;
    uint16_t              mask_len;
} xcb_input_event_mask_t;

/**
 * @brief xcb_input_event_mask_iterator_t
 **/
typedef struct xcb_input_event_mask_iterator_t {
    xcb_input_event_mask_t *data;
    int                     rem;
    int                     index;
} xcb_input_event_mask_iterator_t;

/** Opcode for xcb_input_xi_select_events. */
#define XCB_INPUT_XI_SELECT_EVENTS 46

/**
 * @brief xcb_input_xi_select_events_request_t
 **/
typedef struct xcb_input_xi_select_events_request_t {
    uint8_t      major_opcode;
    uint8_t      minor_opcode;
    uint16_t     length;
    xcb_window_t window;
    uint16_t     num_mask;
    uint8_t      pad0[2];
} xcb_input_xi_select_events_request_t;

/**
 * @brief xcb_input_xi_query_version_cookie_t
 **/
typedef struct xcb_input_xi_query_version_cookie_t {
    unsigned int sequence;
} xcb_input_xi_query_version_cookie_t;

/** Opcode for xcb_input_xi_query_version. */
#define XCB_INPUT_XI_QUERY_VERSION 47

/**
 * @brief xcb_input_xi_query_version_request_t
 **/
typedef struct xcb_input_xi_query_version_request_t {
    uint8_t  major_opcode;
    uint8_t  minor_opcode;
    uint16_t length;
    uint16_t major_version;
    uint16_t minor_version;
} xcb_input_xi_query_version_request_t;

/**
 * @brief xcb_input_xi_query_version_reply_t
 **/
typedef struct xcb_input_xi_query_version_reply_t {
    uint8_t  response_type;
    uint8_t  pad0;
    uint16_t sequence;
    uint32_t length;
    uint16_t major_version;
    uint16_t minor_version;
    uint8_t  pad1[20];
} xcb_input_xi_query_version_reply_t;

typedef enum xcb_input_device_class_type_t {
    XCB_INPUT_DEVICE_CLASS_TYPE_KEY = 0,
    XCB_INPUT_DEVICE_CLASS_TYPE_BUTTON = 1,
    XCB_INPUT_DEVICE_CLASS_TYPE_VALUATOR = 2,
    XCB_INPUT_DEVICE_CLASS_TYPE_SCROLL = 3,
    XCB_INPUT_DEVICE_CLASS_TYPE_TOUCH = 8,
    XCB_INPUT_DEVICE_CLASS_TYPE_GESTURE = 9
} xcb_input_device_class_type_t;

typedef enum xcb_input_device_type_t {
    XCB_INPUT_DEVICE_TYPE_MASTER_POINTER = 1,
    XCB_INPUT_DEVICE_TYPE_MASTER_KEYBOARD = 2,
    XCB_INPUT_DEVICE_TYPE_SLAVE_POINTER = 3,
    XCB_INPUT_DEVICE_TYPE_SLAVE_KEYBOARD = 4,
    XCB_INPUT_DEVICE_TYPE_FLOATING_SLAVE = 5
} xcb_input_device_type_t;

typedef enum xcb_input_scroll_flags_t {
    XCB_INPUT_SCROLL_FLAGS_NO_EMULATION = 1,
    XCB_INPUT_SCROLL_FLAGS_PREFERRED = 2
} xcb_input_scroll_flags_t;

typedef enum xcb_input_scroll_type_t {
    XCB_INPUT_SCROLL_TYPE_VERTICAL = 1,
    XCB_INPUT_SCROLL_TYPE_HORIZONTAL = 2
} xcb_input_scroll_type_t;

typedef enum xcb_input_touch_mode_t {
    XCB_INPUT_TOUCH_MODE_DIRECT = 1,
    XCB_INPUT_TOUCH_MODE_DEPENDENT = 2
} xcb_input_touch_mode_t;

/**
 * @brief xcb_input_button_class_t
 **/
typedef struct xcb_input_button_class_t {
    uint16_t              type;
    uint16_t              len;
    xcb_input_device_id_t sourceid;
    uint16_t              num_buttons;
} xcb_input_button_class_t;

/**
 * @brief xcb_input_button_class_iterator_t
 **/
typedef struct xcb_input_button_class_iterator_t {
    xcb_input_button_class_t *data;
    int                       rem;
    int                       index;
} xcb_input_button_class_iterator_t;

/**
 * @brief xcb_input_key_class_t
 **/
typedef struct xcb_input_key_class_t {
    uint16_t              type;
    uint16_t              len;
    xcb_input_device_id_t sourceid;
    uint16_t              num_keys;
} xcb_input_key_class_t;

/**
 * @brief xcb_input_key_class_iterator_t
 **/
typedef struct xcb_input_key_class_iterator_t {
    xcb_input_key_class_t *data;
    int                    rem;
    int                    index;
} xcb_input_key_class_iterator_t;

/**
 * @brief xcb_input_scroll_class_t
 **/
typedef struct xcb_input_scroll_class_t {
    uint16_t              type;
    uint16_t              len;
    xcb_input_device_id_t sourceid;
    uint16_t              number;
    uint16_t              scroll_type;
    uint8_t               pad0[2];
    uint32_t              flags;
    xcb_input_fp3232_t    increment;
} xcb_input_scroll_class_t;

/**
 * @brief xcb_input_scroll_class_iterator_t
 **/
typedef struct xcb_input_scroll_class_iterator_t {
    xcb_input_scroll_class_t *data;
    int                       rem;
    int                       index;
} xcb_input_scroll_class_iterator_t;

/**
 * @brief xcb_input_touch_class_t
 **/
typedef struct xcb_input_touch_class_t {
    uint16_t              type;
    uint16_t              len;
    xcb_input_device_id_t sourceid;
    uint8_t               mode;
    uint8_t               num_touches;
} xcb_input_touch_class_t;

/**
 * @brief xcb_input_touch_class_iterator_t
 **/
typedef struct xcb_input_touch_class_iterator_t {
    xcb_input_touch_class_t *data;
    int                      rem;
    int                      index;
} xcb_input_touch_class_iterator_t;

/**
 * @brief xcb_input_gesture_class_t
 **/
typedef struct xcb_input_gesture_class_t {
    uint16_t              type;
    uint16_t              len;
    xcb_input_device_id_t sourceid;
    uint8_t               num_touches;
    uint8_t               pad0;
} xcb_input_gesture_class_t;

/**
 * @brief xcb_input_gesture_class_iterator_t
 **/
typedef struct xcb_input_gesture_class_iterator_t {
    xcb_input_gesture_class_t *data;
    int                        rem;
    int                        index;
} xcb_input_gesture_class_iterator_t;

/**
 * @brief xcb_input_valuator_class_t
 **/
typedef struct xcb_input_valuator_class_t {
    uint16_t              type;
    uint16_t              len;
    xcb_input_device_id_t sourceid;
    uint16_t              number;
    xcb_atom_t            label;
    xcb_input_fp3232_t    min;
    xcb_input_fp3232_t    max;
    xcb_input_fp3232_t    value;
    uint32_t              resolution;
    uint8_t               mode;
    uint8_t               pad0[3];
} xcb_input_valuator_class_t;

/**
 * @brief xcb_input_valuator_class_iterator_t
 **/
typedef struct xcb_input_valuator_class_iterator_t {
    xcb_input_valuator_class_t *data;
    int                         rem;
    int                         index;
} xcb_input_valuator_class_iterator_t;

/**
 * @brief xcb_input_device_class_data_t
 **/
typedef struct xcb_input_device_class_data_t {
    struct {
        uint16_t           num_keys;
        uint32_t          *keys;
    } key;
    struct {
        uint16_t           num_buttons;
        uint32_t          *state;
        xcb_atom_t        *labels;
    } button;
    struct {
        uint16_t           number;
        xcb_atom_t         label;
        xcb_input_fp3232_t min;
        xcb_input_fp3232_t max;
        xcb_input_fp3232_t value;
        uint32_t           resolution;
        uint8_t            mode;
        uint8_t            pad0[3];
    } valuator;
    struct {
        uint16_t           number;
        uint16_t           scroll_type;
        uint8_t            pad1[2];
        uint32_t           flags;
        xcb_input_fp3232_t increment;
    } scroll;
    struct {
        uint8_t            mode;
        uint8_t            num_touches;
    } touch;
    struct {
        uint8_t            num_touches;
        uint8_t            pad2;
    } gesture;
} xcb_input_device_class_data_t;

/**
 * @brief xcb_input_device_class_t
 **/
typedef struct xcb_input_device_class_t {
    uint16_t              type;
    uint16_t              len;
    xcb_input_device_id_t sourceid;
} xcb_input_device_class_t;

void *
xcb_input_device_class_data (const xcb_input_device_class_t *R);

/**
 * @brief xcb_input_device_class_iterator_t
 **/
typedef struct xcb_input_device_class_iterator_t {
    xcb_input_device_class_t *data;
    int                       rem;
    int                       index;
} xcb_input_device_class_iterator_t;

/**
 * @brief xcb_input_xi_device_info_t
 **/
typedef struct xcb_input_xi_device_info_t {
    xcb_input_device_id_t deviceid;
    uint16_t              type;
    xcb_input_device_id_t attachment;
    uint16_t              num_classes;
    uint16_t              name_len;
    uint8_t               enabled;
    uint8_t               pad0;
} xcb_input_xi_device_info_t;

/**
 * @brief xcb_input_xi_device_info_iterator_t
 **/
typedef struct xcb_input_xi_device_info_iterator_t {
    xcb_input_xi_device_info_t *data;
    int                         rem;
    int                         index;
} xcb_input_xi_device_info_iterator_t;

/**
 * @brief xcb_input_xi_query_device_cookie_t
 **/
typedef struct xcb_input_xi_query_device_cookie_t {
    unsigned int sequence;
} xcb_input_xi_query_device_cookie_t;

/** Opcode for xcb_input_xi_query_device. */
#define XCB_INPUT_XI_QUERY_DEVICE 48

/**
 * @brief xcb_input_xi_query_device_request_t
 **/
typedef struct xcb_input_xi_query_device_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_input_device_id_t deviceid;
    uint8_t               pad0[2];
} xcb_input_xi_query_device_request_t;

/**
 * @brief xcb_input_xi_query_device_reply_t
 **/
typedef struct xcb_input_xi_query_device_reply_t {
    uint8_t  response_type;
    uint8_t  pad0;
    uint16_t sequence;
    uint32_t length;
    uint16_t num_infos;
    uint8_t  pad1[22];
} xcb_input_xi_query_device_reply_t;

/** Opcode for xcb_input_xi_set_focus. */
#define XCB_INPUT_XI_SET_FOCUS 49

/**
 * @brief xcb_input_xi_set_focus_request_t
 **/
typedef struct xcb_input_xi_set_focus_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_window_t          window;
    xcb_timestamp_t       time;
    xcb_input_device_id_t deviceid;
    uint8_t               pad0[2];
} xcb_input_xi_set_focus_request_t;

/**
 * @brief xcb_input_xi_get_focus_cookie_t
 **/
typedef struct xcb_input_xi_get_focus_cookie_t {
    unsigned int sequence;
} xcb_input_xi_get_focus_cookie_t;

/** Opcode for xcb_input_xi_get_focus. */
#define XCB_INPUT_XI_GET_FOCUS 50

/**
 * @brief xcb_input_xi_get_focus_request_t
 **/
typedef struct xcb_input_xi_get_focus_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_input_device_id_t deviceid;
    uint8_t               pad0[2];
} xcb_input_xi_get_focus_request_t;

/**
 * @brief xcb_input_xi_get_focus_reply_t
 **/
typedef struct xcb_input_xi_get_focus_reply_t {
    uint8_t      response_type;
    uint8_t      pad0;
    uint16_t     sequence;
    uint32_t     length;
    xcb_window_t focus;
    uint8_t      pad1[20];
} xcb_input_xi_get_focus_reply_t;

typedef enum xcb_input_grab_owner_t {
    XCB_INPUT_GRAB_OWNER_NO_OWNER = 0,
    XCB_INPUT_GRAB_OWNER_OWNER = 1
} xcb_input_grab_owner_t;

/**
 * @brief xcb_input_xi_grab_device_cookie_t
 **/
typedef struct xcb_input_xi_grab_device_cookie_t {
    unsigned int sequence;
} xcb_input_xi_grab_device_cookie_t;

/** Opcode for xcb_input_xi_grab_device. */
#define XCB_INPUT_XI_GRAB_DEVICE 51

/**
 * @brief xcb_input_xi_grab_device_request_t
 **/
typedef struct xcb_input_xi_grab_device_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_window_t          window;
    xcb_timestamp_t       time;
    xcb_cursor_t          cursor;
    xcb_input_device_id_t deviceid;
    uint8_t               mode;
    uint8_t               paired_device_mode;
    uint8_t               owner_events;
    uint8_t               pad0;
    uint16_t              mask_len;
} xcb_input_xi_grab_device_request_t;

/**
 * @brief xcb_input_xi_grab_device_reply_t
 **/
typedef struct xcb_input_xi_grab_device_reply_t {
    uint8_t  response_type;
    uint8_t  pad0;
    uint16_t sequence;
    uint32_t length;
    uint8_t  status;
    uint8_t  pad1[23];
} xcb_input_xi_grab_device_reply_t;

/** Opcode for xcb_input_xi_ungrab_device. */
#define XCB_INPUT_XI_UNGRAB_DEVICE 52

/**
 * @brief xcb_input_xi_ungrab_device_request_t
 **/
typedef struct xcb_input_xi_ungrab_device_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_timestamp_t       time;
    xcb_input_device_id_t deviceid;
    uint8_t               pad0[2];
} xcb_input_xi_ungrab_device_request_t;

typedef enum xcb_input_event_mode_t {
    XCB_INPUT_EVENT_MODE_ASYNC_DEVICE = 0,
    XCB_INPUT_EVENT_MODE_SYNC_DEVICE = 1,
    XCB_INPUT_EVENT_MODE_REPLAY_DEVICE = 2,
    XCB_INPUT_EVENT_MODE_ASYNC_PAIRED_DEVICE = 3,
    XCB_INPUT_EVENT_MODE_ASYNC_PAIR = 4,
    XCB_INPUT_EVENT_MODE_SYNC_PAIR = 5,
    XCB_INPUT_EVENT_MODE_ACCEPT_TOUCH = 6,
    XCB_INPUT_EVENT_MODE_REJECT_TOUCH = 7
} xcb_input_event_mode_t;

/** Opcode for xcb_input_xi_allow_events. */
#define XCB_INPUT_XI_ALLOW_EVENTS 53

/**
 * @brief xcb_input_xi_allow_events_request_t
 **/
typedef struct xcb_input_xi_allow_events_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_timestamp_t       time;
    xcb_input_device_id_t deviceid;
    uint8_t               event_mode;
    uint8_t               pad0;
    uint32_t              touchid;
    xcb_window_t          grab_window;
} xcb_input_xi_allow_events_request_t;

typedef enum xcb_input_grab_mode_22_t {
    XCB_INPUT_GRAB_MODE_22_SYNC = 0,
    XCB_INPUT_GRAB_MODE_22_ASYNC = 1,
    XCB_INPUT_GRAB_MODE_22_TOUCH = 2
} xcb_input_grab_mode_22_t;

typedef enum xcb_input_grab_type_t {
    XCB_INPUT_GRAB_TYPE_BUTTON = 0,
    XCB_INPUT_GRAB_TYPE_KEYCODE = 1,
    XCB_INPUT_GRAB_TYPE_ENTER = 2,
    XCB_INPUT_GRAB_TYPE_FOCUS_IN = 3,
    XCB_INPUT_GRAB_TYPE_TOUCH_BEGIN = 4,
    XCB_INPUT_GRAB_TYPE_GESTURE_PINCH_BEGIN = 5,
    XCB_INPUT_GRAB_TYPE_GESTURE_SWIPE_BEGIN = 6
} xcb_input_grab_type_t;

typedef enum xcb_input_modifier_mask_t {
    XCB_INPUT_MODIFIER_MASK_ANY = 2147483648
} xcb_input_modifier_mask_t;

/**
 * @brief xcb_input_grab_modifier_info_t
 **/
typedef struct xcb_input_grab_modifier_info_t {
    uint32_t modifiers;
    uint8_t  status;
    uint8_t  pad0[3];
} xcb_input_grab_modifier_info_t;

/**
 * @brief xcb_input_grab_modifier_info_iterator_t
 **/
typedef struct xcb_input_grab_modifier_info_iterator_t {
    xcb_input_grab_modifier_info_t *data;
    int                             rem;
    int                             index;
} xcb_input_grab_modifier_info_iterator_t;

/**
 * @brief xcb_input_xi_passive_grab_device_cookie_t
 **/
typedef struct xcb_input_xi_passive_grab_device_cookie_t {
    unsigned int sequence;
} xcb_input_xi_passive_grab_device_cookie_t;

/** Opcode for xcb_input_xi_passive_grab_device. */
#define XCB_INPUT_XI_PASSIVE_GRAB_DEVICE 54

/**
 * @brief xcb_input_xi_passive_grab_device_request_t
 **/
typedef struct xcb_input_xi_passive_grab_device_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_timestamp_t       time;
    xcb_window_t          grab_window;
    xcb_cursor_t          cursor;
    uint32_t              detail;
    xcb_input_device_id_t deviceid;
    uint16_t              num_modifiers;
    uint16_t              mask_len;
    uint8_t               grab_type;
    uint8_t               grab_mode;
    uint8_t               paired_device_mode;
    uint8_t               owner_events;
    uint8_t               pad0[2];
} xcb_input_xi_passive_grab_device_request_t;

/**
 * @brief xcb_input_xi_passive_grab_device_reply_t
 **/
typedef struct xcb_input_xi_passive_grab_device_reply_t {
    uint8_t  response_type;
    uint8_t  pad0;
    uint16_t sequence;
    uint32_t length;
    uint16_t num_modifiers;
    uint8_t  pad1[22];
} xcb_input_xi_passive_grab_device_reply_t;

/** Opcode for xcb_input_xi_passive_ungrab_device. */
#define XCB_INPUT_XI_PASSIVE_UNGRAB_DEVICE 55

/**
 * @brief xcb_input_xi_passive_ungrab_device_request_t
 **/
typedef struct xcb_input_xi_passive_ungrab_device_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_window_t          grab_window;
    uint32_t              detail;
    xcb_input_device_id_t deviceid;
    uint16_t              num_modifiers;
    uint8_t               grab_type;
    uint8_t               pad0[3];
} xcb_input_xi_passive_ungrab_device_request_t;

/**
 * @brief xcb_input_xi_list_properties_cookie_t
 **/
typedef struct xcb_input_xi_list_properties_cookie_t {
    unsigned int sequence;
} xcb_input_xi_list_properties_cookie_t;

/** Opcode for xcb_input_xi_list_properties. */
#define XCB_INPUT_XI_LIST_PROPERTIES 56

/**
 * @brief xcb_input_xi_list_properties_request_t
 **/
typedef struct xcb_input_xi_list_properties_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_input_device_id_t deviceid;
    uint8_t               pad0[2];
} xcb_input_xi_list_properties_request_t;

/**
 * @brief xcb_input_xi_list_properties_reply_t
 **/
typedef struct xcb_input_xi_list_properties_reply_t {
    uint8_t  response_type;
    uint8_t  pad0;
    uint16_t sequence;
    uint32_t length;
    uint16_t num_properties;
    uint8_t  pad1[22];
} xcb_input_xi_list_properties_reply_t;

/**
 * @brief xcb_input_xi_change_property_items_t
 **/
typedef struct xcb_input_xi_change_property_items_t {
    uint8_t  *data8;
    uint16_t *data16;
    uint32_t *data32;
} xcb_input_xi_change_property_items_t;

/** Opcode for xcb_input_xi_change_property. */
#define XCB_INPUT_XI_CHANGE_PROPERTY 57

/**
 * @brief xcb_input_xi_change_property_request_t
 **/
typedef struct xcb_input_xi_change_property_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_input_device_id_t deviceid;
    uint8_t               mode;
    uint8_t               format;
    xcb_atom_t            property;
    xcb_atom_t            type;
    uint32_t              num_items;
} xcb_input_xi_change_property_request_t;

/** Opcode for xcb_input_xi_delete_property. */
#define XCB_INPUT_XI_DELETE_PROPERTY 58

/**
 * @brief xcb_input_xi_delete_property_request_t
 **/
typedef struct xcb_input_xi_delete_property_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_input_device_id_t deviceid;
    uint8_t               pad0[2];
    xcb_atom_t            property;
} xcb_input_xi_delete_property_request_t;

/**
 * @brief xcb_input_xi_get_property_cookie_t
 **/
typedef struct xcb_input_xi_get_property_cookie_t {
    unsigned int sequence;
} xcb_input_xi_get_property_cookie_t;

/** Opcode for xcb_input_xi_get_property. */
#define XCB_INPUT_XI_GET_PROPERTY 59

/**
 * @brief xcb_input_xi_get_property_request_t
 **/
typedef struct xcb_input_xi_get_property_request_t {
    uint8_t               major_opcode;
    uint8_t               minor_opcode;
    uint16_t              length;
    xcb_input_device_id_t deviceid;
    uint8_t               _delete;
    uint8_t               pad0;
    xcb_atom_t            property;
    xcb_atom_t            type;
    uint32_t              offset;
    uint32_t              len;
} xcb_input_xi_get_property_request_t;

/**
 * @brief xcb_input_xi_get_property_items_t
 **/
typedef struct xcb_input_xi_get_property_items_t {
    uint8_t  *data8;
    uint16_t *data16;
    uint32_t *data32;
} xcb_input_xi_get_property_items_t;

/**
 * @brief xcb_input_xi_get_property_reply_t
 **/
typedef struct xcb_input_xi_get_property_reply_t {
    uint8_t    response_type;
    uint8_t    pad0;
    uint16_t   sequence;
    uint32_t   length;
    xcb_atom_t type;
    uint32_t   bytes_after;
    uint32_t   num_items;
    uint8_t    format;
    uint8_t    pad1[11];
} xcb_input_xi_get_property_reply_t;

/**
 * @brief xcb_input_xi_get_selected_events_cookie_t
 **/
typedef struct xcb_input_xi_get_selected_events_cookie_t {
    unsigned int sequence;
} xcb_input_xi_get_selected_events_cookie_t;

/** Opcode for xcb_input_xi_get_selected_events. */
#define XCB_INPUT_XI_GET_SELECTED_EVENTS 60

/**
 * @brief xcb_input_xi_get_selected_events_request_t
 **/
typedef struct xcb_input_xi_get_selected_events_request_t {
    uint8_t      major_opcode;
    uint8_t      minor_opcode;
    uint16_t     length;
    xcb_window_t window;
} xcb_input_xi_get_selected_events_request_t;

/**
 * @brief xcb_input_xi_get_selected_events_reply_t
 **/
typedef struct xcb_input_xi_get_selected_events_reply_t {
    uint8_t  response_type;
    uint8_t  pad0;
    uint16_t sequence;
    uint32_t length;
    uint16_t num_masks;
    uint8_t  pad1[22];
} xcb_input_xi_get_selected_events_reply_t;

/** Opcode for xcb_input_device_valuator. */
#define XCB_INPUT_DEVICE_VALUATOR 0

/**
 * @brief xcb_input_device_valuator_event_t
 **/
typedef struct xcb_input_device_valuator_event_t {
    uint8_t  response_type;
    uint8_t  device_id;
    uint16_t sequence;
    uint16_t device_state;
    uint8_t  num_valuators;
    uint8_t  first_valuator;
    int32_t  valuators[6];
} xcb_input_device_valuator_event_t;

typedef enum xcb_input_more_events_mask_t {
    XCB_INPUT_MORE_EVENTS_MASK_MORE_EVENTS = 128
} xcb_input_more_events_mask_t;

/** Opcode for xcb_input_device_key_press. */
#define XCB_INPUT_DEVICE_KEY_PRESS 1

/**
 * @brief xcb_input_device_key_press_event_t
 **/
typedef struct xcb_input_device_key_press_event_t {
    uint8_t         response_type;
    uint8_t         detail;
    uint16_t        sequence;
    xcb_timestamp_t time;
    xcb_window_t    root;
    xcb_window_t    event;
    xcb_window_t    child;
    int16_t         root_x;
    int16_t         root_y;
    int16_t         event_x;
    int16_t         event_y;
    uint16_t        state;
    uint8_t         same_screen;
    uint8_t         device_id;
} xcb_input_device_key_press_event_t;

/** Opcode for xcb_input_device_key_release. */
#define XCB_INPUT_DEVICE_KEY_RELEASE 2

typedef xcb_input_device_key_press_event_t xcb_input_device_key_release_event_t;

/** Opcode for xcb_input_device_button_press. */
#define XCB_INPUT_DEVICE_BUTTON_PRESS 3

typedef xcb_input_device_key_press_event_t xcb_input_device_button_press_event_t;

/** Opcode for xcb_input_device_button_release. */
#define XCB_INPUT_DEVICE_BUTTON_RELEASE 4

typedef xcb_input_device_key_press_event_t xcb_input_device_button_release_event_t;

/** Opcode for xcb_input_device_motion_notify. */
#define XCB_INPUT_DEVICE_MOTION_NOTIFY 5

typedef xcb_input_device_key_press_event_t xcb_input_device_motion_notify_event_t;

/** Opcode for xcb_input_device_focus_in. */
#define XCB_INPUT_DEVICE_FOCUS_IN 6

/**
 * @brief xcb_input_device_focus_in_event_t
 **/
typedef struct xcb_input_device_focus_in_event_t {
    uint8_t         response_type;
    uint8_t         detail;
    uint16_t        sequence;
    xcb_timestamp_t time;
    xcb_window_t    window;
    uint8_t         mode;
    uint8_t         device_id;
    uint8_t         pad0[18];
} xcb_input_device_focus_in_event_t;

/** Opcode for xcb_input_device_focus_out. */
#define XCB_INPUT_DEVICE_FOCUS_OUT 7

typedef xcb_input_device_focus_in_event_t xcb_input_device_focus_out_event_t;

/** Opcode for xcb_input_proximity_in. */
#define XCB_INPUT_PROXIMITY_IN 8

typedef xcb_input_device_key_press_event_t xcb_input_proximity_in_event_t;

/** Opcode for xcb_input_proximity_out. */
#define XCB_INPUT_PROXIMITY_OUT 9

typedef xcb_input_device_key_press_event_t xcb_input_proximity_out_event_t;

typedef enum xcb_input_classes_reported_mask_t {
    XCB_INPUT_CLASSES_REPORTED_MASK_OUT_OF_PROXIMITY = 128,
    XCB_INPUT_CLASSES_REPORTED_MASK_DEVICE_MODE_ABSOLUTE = 64,
    XCB_INPUT_CLASSES_REPORTED_MASK_REPORTING_VALUATORS = 4,
    XCB_INPUT_CLASSES_REPORTED_MASK_REPORTING_BUTTONS = 2,
    XCB_INPUT_CLASSES_REPORTED_MASK_REPORTING_KEYS = 1
} xcb_input_classes_reported_mask_t;

/** Opcode for xcb_input_device_state_notify. */
#define XCB_INPUT_DEVICE_STATE_NOTIFY 10

/**
 * @brief xcb_input_device_state_notify_event_t
 **/
typedef struct xcb_input_device_state_notify_event_t {
    uint8_t         response_type;
    uint8_t         device_id;
    uint16_t        sequence;
    xcb_timestamp_t time;
    uint8_t         num_keys;
    uint8_t         num_buttons;
    uint8_t         num_valuators;
    uint8_t         classes_reported;
    uint8_t         buttons[4];
    uint8_t         keys[4];
    uint32_t        valuators[3];
} xcb_input_device_state_notify_event_t;

/** Opcode for xcb_input_device_mapping_notify. */
#define XCB_INPUT_DEVICE_MAPPING_NOTIFY 11

/**
 * @brief xcb_input_device_mapping_notify_event_t
 **/
typedef struct xcb_input_device_mapping_notify_event_t {
    uint8_t              response_type;
    uint8_t              device_id;
    uint16_t             sequence;
    uint8_t              request;
    xcb_input_key_code_t first_keycode;
    uint8_t              count;
    uint8_t              pad0;
    xcb_timestamp_t      time;
    uint8_t              pad1[20];
} xcb_input_device_mapping_notify_event_t;

typedef enum xcb_input_change_device_t {
    XCB_INPUT_CHANGE_DEVICE_NEW_POINTER = 0,
    XCB_INPUT_CHANGE_DEVICE_NEW_KEYBOARD = 1
} xcb_input_change_device_t;

/** Opcode for xcb_input_change_device_notify. */
#define XCB_INPUT_CHANGE_DEVICE_NOTIFY 12

/**
 * @brief xcb_input_change_device_notify_event_t
 **/
typedef struct xcb_input_change_device_notify_event_t {
    uint8_t         response_type;
    uint8_t         device_id;
    uint16_t        sequence;
    xcb_timestamp_t time;
    uint8_t         request;
    uint8_t         pad0[23];
} xcb_input_change_device_notify_event_t;

/** Opcode for xcb_input_device_key_state_notify. */
#define XCB_INPUT_DEVICE_KEY_STATE_NOTIFY 13

/**
 * @brief xcb_input_device_key_state_notify_event_t
 **/
typedef struct xcb_input_device_key_state_notify_event_t {
    uint8_t  response_type;
    uint8_t  device_id;
    uint16_t sequence;
    uint8_t  keys[28];
} xcb_input_device_key_state_notify_event_t;

/** Opcode for xcb_input_device_button_state_notify. */
#define XCB_INPUT_DEVICE_BUTTON_STATE_NOTIFY 14

/**
 * @brief xcb_input_device_button_state_notify_event_t
 **/
typedef struct xcb_input_device_button_state_notify_event_t {
    uint8_t  response_type;
    uint8_t  device_id;
    uint16_t sequence;
    uint8_t  buttons[28];
} xcb_input_device_button_state_notify_event_t;

typedef enum xcb_input_device_change_t {
    XCB_INPUT_DEVICE_CHANGE_ADDED = 0,
    XCB_INPUT_DEVICE_CHANGE_REMOVED = 1,
    XCB_INPUT_DEVICE_CHANGE_ENABLED = 2,
    XCB_INPUT_DEVICE_CHANGE_DISABLED = 3,
    XCB_INPUT_DEVICE_CHANGE_UNRECOVERABLE = 4,
    XCB_INPUT_DEVICE_CHANGE_CONTROL_CHANGED = 5
} xcb_input_device_change_t;

/** Opcode for xcb_input_device_presence_notify. */
#define XCB_INPUT_DEVICE_PRESENCE_NOTIFY 15

/**
 * @brief xcb_input_device_presence_notify_event_t
 **/
typedef struct xcb_input_device_presence_notify_event_t {
    uint8_t         response_type;
    uint8_t         pad0;
    uint16_t        sequence;
    xcb_timestamp_t time;
    uint8_t         devchange;
    uint8_t         device_id;
    uint16_t        control;
    uint8_t         pad1[20];
} xcb_input_device_presence_notify_event_t;

/** Opcode for xcb_input_device_property_notify. */
#define XCB_INPUT_DEVICE_PROPERTY_NOTIFY 16

/**
 * @brief xcb_input_device_property_notify_event_t
 **/
typedef struct xcb_input_device_property_notify_event_t {
    uint8_t         response_type;
    uint8_t         state;
    uint16_t        sequence;
    xcb_timestamp_t time;
    xcb_atom_t      property;
    uint8_t         pad0[19];
    uint8_t         device_id;
} xcb_input_device_property_notify_event_t;

typedef enum xcb_input_change_reason_t {
    XCB_INPUT_CHANGE_REASON_SLAVE_SWITCH = 1,
    XCB_INPUT_CHANGE_REASON_DEVICE_CHANGE = 2
} xcb_input_change_reason_t;

/** Opcode for xcb_input_device_changed. */
#define XCB_INPUT_DEVICE_CHANGED 1

/**
 * @brief xcb_input_device_changed_event_t
 **/
typedef struct xcb_input_device_changed_event_t {
    uint8_t               response_type;
    uint8_t               extension;
    uint16_t              sequence;
    uint32_t              length;
    uint16_t              event_type;
    xcb_input_device_id_t deviceid;
    xcb_timestamp_t       time;
    uint16_t              num_classes;
    xcb_input_device_id_t sourceid;
    uint8_t               reason;
    uint8_t               pad0[11];
    uint32_t              full_sequence;
} xcb_input_device_changed_event_t;

typedef enum xcb_input_key_event_flags_t {
    XCB_INPUT_KEY_EVENT_FLAGS_KEY_REPEAT = 65536
} xcb_input_key_event_flags_t;

/** Opcode for xcb_input_key_press. */
#define XCB_INPUT_KEY_PRESS 2

/**
 * @brief xcb_input_key_press_event_t
 **/
typedef struct xcb_input_key_press_event_t {
    uint8_t                   response_type;
    uint8_t                   extension;
    uint16_t                  sequence;
    uint32_t                  length;
    uint16_t                  event_type;
    xcb_input_device_id_t     deviceid;
    xcb_timestamp_t           time;
    uint32_t                  detail;
    xcb_window_t              root;
    xcb_window_t              event;
    xcb_window_t              child;
    uint32_t                  full_sequence;
    xcb_input_fp1616_t        root_x;
    xcb_input_fp1616_t        root_y;
    xcb_input_fp1616_t        event_x;
    xcb_input_fp1616_t        event_y;
    uint16_t                  buttons_len;
    uint16_t                  valuators_len;
    xcb_input_device_id_t     sourceid;
    uint8_t                   pad0[2];
    uint32_t                  flags;
    xcb_input_modifier_info_t mods;
    xcb_input_group_info_t    group;
} xcb_input_key_press_event_t;

/** Opcode for xcb_input_key_release. */
#define XCB_INPUT_KEY_RELEASE 3

typedef xcb_input_key_press_event_t xcb_input_key_release_event_t;

typedef enum xcb_input_pointer_event_flags_t {
    XCB_INPUT_POINTER_EVENT_FLAGS_POINTER_EMULATED = 65536
} xcb_input_pointer_event_flags_t;

/** Opcode for xcb_input_button_press. */
#define XCB_INPUT_BUTTON_PRESS 4

/**
 * @brief xcb_input_button_press_event_t
 **/
typedef struct xcb_input_button_press_event_t {
    uint8_t                   response_type;
    uint8_t                   extension;
    uint16_t                  sequence;
    uint32_t                  length;
    uint16_t                  event_type;
    xcb_input_device_id_t     deviceid;
    xcb_timestamp_t           time;
    uint32_t                  detail;
    xcb_window_t              root;
    xcb_window_t              event;
    xcb_window_t              child;
    uint32_t                  full_sequence;
    xcb_input_fp1616_t        root_x;
    xcb_input_fp1616_t        root_y;
    xcb_input_fp1616_t        event_x;
    xcb_input_fp1616_t        event_y;
    uint16_t                  buttons_len;
    uint16_t                  valuators_len;
    xcb_input_device_id_t     sourceid;
    uint8_t                   pad0[2];
    uint32_t                  flags;
    xcb_input_modifier_info_t mods;
    xcb_input_group_info_t    group;
} xcb_input_button_press_event_t;

/** Opcode for xcb_input_button_release. */
#define XCB_INPUT_BUTTON_RELEASE 5

typedef xcb_input_button_press_event_t xcb_input_button_release_event_t;

/** Opcode for xcb_input_motion. */
#define XCB_INPUT_MOTION 6

typedef xcb_input_button_press_event_t xcb_input_motion_event_t;

typedef enum xcb_input_notify_mode_t {
    XCB_INPUT_NOTIFY_MODE_NORMAL = 0,
    XCB_INPUT_NOTIFY_MODE_GRAB = 1,
    XCB_INPUT_NOTIFY_MODE_UNGRAB = 2,
    XCB_INPUT_NOTIFY_MODE_WHILE_GRABBED = 3,
    XCB_INPUT_NOTIFY_MODE_PASSIVE_GRAB = 4,
    XCB_INPUT_NOTIFY_MODE_PASSIVE_UNGRAB = 5
} xcb_input_notify_mode_t;

typedef enum xcb_input_notify_detail_t {
    XCB_INPUT_NOTIFY_DETAIL_ANCESTOR = 0,
    XCB_INPUT_NOTIFY_DETAIL_VIRTUAL = 1,
    XCB_INPUT_NOTIFY_DETAIL_INFERIOR = 2,
    XCB_INPUT_NOTIFY_DETAIL_NONLINEAR = 3,
    XCB_INPUT_NOTIFY_DETAIL_NONLINEAR_VIRTUAL = 4,
    XCB_INPUT_NOTIFY_DETAIL_POINTER = 5,
    XCB_INPUT_NOTIFY_DETAIL_POINTER_ROOT = 6,
    XCB_INPUT_NOTIFY_DETAIL_NONE = 7
} xcb_input_notify_detail_t;

/** Opcode for xcb_input_enter. */
#define XCB_INPUT_ENTER 7

/**
 * @brief xcb_input_enter_event_t
 **/
typedef struct xcb_input_enter_event_t {
    uint8_t                   response_type;
    uint8_t                   extension;
    uint16_t                  sequence;
    uint32_t                  length;
    uint16_t                  event_type;
    xcb_input_device_id_t     deviceid;
    xcb_timestamp_t           time;
    xcb_input_device_id_t     sourceid;
    uint8_t                   mode;
    uint8_t                   detail;
    xcb_window_t              root;
    xcb_window_t              event;
    xcb_window_t              child;
    uint32_t                  full_sequence;
    xcb_input_fp1616_t        root_x;
    xcb_input_fp1616_t        root_y;
    xcb_input_fp1616_t        event_x;
    xcb_input_fp1616_t        event_y;
    uint8_t                   same_screen;
    uint8_t                   focus;
    uint16_t                  buttons_len;
    xcb_input_modifier_info_t mods;
    xcb_input_group_info_t    group;
} xcb_input_enter_event_t;

/** Opcode for xcb_input_leave. */
#define XCB_INPUT_LEAVE 8

typedef xcb_input_enter_event_t xcb_input_leave_event_t;

/** Opcode for xcb_input_focus_in. */
#define XCB_INPUT_FOCUS_IN 9

typedef xcb_input_enter_event_t xcb_input_focus_in_event_t;

/** Opcode for xcb_input_focus_out. */
#define XCB_INPUT_FOCUS_OUT 10

typedef xcb_input_enter_event_t xcb_input_focus_out_event_t;

typedef enum xcb_input_hierarchy_mask_t {
    XCB_INPUT_HIERARCHY_MASK_MASTER_ADDED = 1,
    XCB_INPUT_HIERARCHY_MASK_MASTER_REMOVED = 2,
    XCB_INPUT_HIERARCHY_MASK_SLAVE_ADDED = 4,
    XCB_INPUT_HIERARCHY_MASK_SLAVE_REMOVED = 8,
    XCB_INPUT_HIERARCHY_MASK_SLAVE_ATTACHED = 16,
    XCB_INPUT_HIERARCHY_MASK_SLAVE_DETACHED = 32,
    XCB_INPUT_HIERARCHY_MASK_DEVICE_ENABLED = 64,
    XCB_INPUT_HIERARCHY_MASK_DEVICE_DISABLED = 128
} xcb_input_hierarchy_mask_t;

/**
 * @brief xcb_input_hierarchy_info_t
 **/
typedef struct xcb_input_hierarchy_info_t {
    xcb_input_device_id_t deviceid;
    xcb_input_device_id_t attachment;
    uint8_t               type;
    uint8_t               enabled;
    uint8_t               pad0[2];
    uint32_t              flags;
} xcb_input_hierarchy_info_t;

/**
 * @brief xcb_input_hierarchy_info_iterator_t
 **/
typedef struct xcb_input_hierarchy_info_iterator_t {
    xcb_input_hierarchy_info_t *data;
    int                         rem;
    int                         index;
} xcb_input_hierarchy_info_iterator_t;

/** Opcode for xcb_input_hierarchy. */
#define XCB_INPUT_HIERARCHY 11

/**
 * @brief xcb_input_hierarchy_event_t
 **/
typedef struct xcb_input_hierarchy_event_t {
    uint8_t               response_type;
    uint8_t               extension;
    uint16_t              sequence;
    uint32_t              length;
    uint16_t              event_type;
    xcb_input_device_id_t deviceid;
    xcb_timestamp_t       time;
    uint32_t              flags;
    uint16_t              num_infos;
    uint8_t               pad0[10];
    uint32_t              full_sequence;
} xcb_input_hierarchy_event_t;

typedef enum xcb_input_property_flag_t {
    XCB_INPUT_PROPERTY_FLAG_DELETED = 0,
    XCB_INPUT_PROPERTY_FLAG_CREATED = 1,
    XCB_INPUT_PROPERTY_FLAG_MODIFIED = 2
} xcb_input_property_flag_t;

/** Opcode for xcb_input_property. */
#define XCB_INPUT_PROPERTY 12

/**
 * @brief xcb_input_property_event_t
 **/
typedef struct xcb_input_property_event_t {
    uint8_t               response_type;
    uint8_t               extension;
    uint16_t              sequence;
    uint32_t              length;
    uint16_t              event_type;
    xcb_input_device_id_t deviceid;
    xcb_timestamp_t       time;
    xcb_atom_t            property;
    uint8_t               what;
    uint8_t               pad0[11];
    uint32_t              full_sequence;
} xcb_input_property_event_t;

/** Opcode for xcb_input_raw_key_press. */
#define XCB_INPUT_RAW_KEY_PRESS 13

/**
 * @brief xcb_input_raw_key_press_event_t
 **/
typedef struct xcb_input_raw_key_press_event_t {
    uint8_t               response_type;
    uint8_t               extension;
    uint16_t              sequence;
    uint32_t              length;
    uint16_t              event_type;
    xcb_input_device_id_t deviceid;
    xcb_timestamp_t       time;
    uint32_t              detail;
    xcb_input_device_id_t sourceid;
    uint16_t              valuators_len;
    uint32_t              flags;
    uint8_t               pad0[4];
    uint32_t              full_sequence;
} xcb_input_raw_key_press_event_t;

/** Opcode for xcb_input_raw_key_release. */
#define XCB_INPUT_RAW_KEY_RELEASE 14

typedef xcb_input_raw_key_press_event_t xcb_input_raw_key_release_event_t;

/** Opcode for xcb_input_raw_button_press. */
#define XCB_INPUT_RAW_BUTTON_PRESS 15

/**
 * @brief xcb_input_raw_button_press_event_t
 **/
typedef struct xcb_input_raw_button_press_event_t {
    uint8_t               response_type;
    uint8_t               extension;
    uint16_t              sequence;
    uint32_t              length;
    uint16_t              event_type;
    xcb_input_device_id_t deviceid;
    xcb_timestamp_t       time;
    uint32_t              detail;
    xcb_input_device_id_t sourceid;
    uint16_t              valuators_len;
    uint32_t              flags;
    uint8_t               pad0[4];
    uint32_t              full_sequence;
} xcb_input_raw_button_press_event_t;

/** Opcode for xcb_input_raw_button_release. */
#define XCB_INPUT_RAW_BUTTON_RELEASE 16

typedef xcb_input_raw_button_press_event_t xcb_input_raw_button_release_event_t;

/** Opcode for xcb_input_raw_motion. */
#define XCB_INPUT_RAW_MOTION 17

typedef xcb_input_raw_button_press_event_t xcb_input_raw_motion_event_t;

typedef enum xcb_input_touch_event_flags_t {
    XCB_INPUT_TOUCH_EVENT_FLAGS_TOUCH_PENDING_END = 65536,
    XCB_INPUT_TOUCH_EVENT_FLAGS_TOUCH_EMULATING_POINTER = 131072
} xcb_input_touch_event_flags_t;

/** Opcode for xcb_input_touch_begin. */
#define XCB_INPUT_TOUCH_BEGIN 18

/**
 * @brief xcb_input_touch_begin_event_t
 **/
typedef struct xcb_input_touch_begin_event_t {
    uint8_t                   response_type;
    uint8_t                   extension;
    uint16_t                  sequence;
    uint32_t                  length;
    uint16_t                  event_type;
    xcb_input_device_id_t     deviceid;
    xcb_timestamp_t           time;
    uint32_t                  detail;
    xcb_window_t              root;
    xcb_window_t              event;
    xcb_window_t              child;
    uint32_t                  full_sequence;
    xcb_input_fp1616_t        root_x;
    xcb_input_fp1616_t        root_y;
    xcb_input_fp1616_t        event_x;
    xcb_input_fp1616_t        event_y;
    uint16_t                  buttons_len;
    uint16_t                  valuators_len;
    xcb_input_device_id_t     sourceid;
    uint8_t                   pad0[2];
    uint32_t                  flags;
    xcb_input_modifier_info_t mods;
    xcb_input_group_info_t    group;
} xcb_input_touch_begin_event_t;

/** Opcode for xcb_input_touch_update. */
#define XCB_INPUT_TOUCH_UPDATE 19

typedef xcb_input_touch_begin_event_t xcb_input_touch_update_event_t;

/** Opcode for xcb_input_touch_end. */
#define XCB_INPUT_TOUCH_END 20

typedef xcb_input_touch_begin_event_t xcb_input_touch_end_event_t;

typedef enum xcb_input_touch_ownership_flags_t {
    XCB_INPUT_TOUCH_OWNERSHIP_FLAGS_NONE = 0
} xcb_input_touch_ownership_flags_t;

/** Opcode for xcb_input_touch_ownership. */
#define XCB_INPUT_TOUCH_OWNERSHIP 21

/**
 * @brief xcb_input_touch_ownership_event_t
 **/
typedef struct xcb_input_touch_ownership_event_t {
    uint8_t               response_type;
    uint8_t               extension;
    uint16_t              sequence;
    uint32_t              length;
    uint16_t              event_type;
    xcb_input_device_id_t deviceid;
    xcb_timestamp_t       time;
    uint32_t              touchid;
    xcb_window_t          root;
    xcb_window_t          event;
    xcb_window_t          child;
    uint32_t              full_sequence;
    xcb_input_device_id_t sourceid;
    uint8_t               pad0[2];
    uint32_t              flags;
    uint8_t               pad1[8];
} xcb_input_touch_ownership_event_t;

/** Opcode for xcb_input_raw_touch_begin. */
#define XCB_INPUT_RAW_TOUCH_BEGIN 22

/**
 * @brief xcb_input_raw_touch_begin_event_t
 **/
typedef struct xcb_input_raw_touch_begin_event_t {
    uint8_t               response_type;
    uint8_t               extension;
    uint16_t              sequence;
    uint32_t              length;
    uint16_t              event_type;
    xcb_input_device_id_t deviceid;
    xcb_timestamp_t       time;
    uint32_t              detail;
    xcb_input_device_id_t sourceid;
    uint16_t              valuators_len;
    uint32_t              flags;
    uint8_t               pad0[4];
    uint32_t              full_sequence;
} xcb_input_raw_touch_begin_event_t;

/** Opcode for xcb_input_raw_touch_update. */
#define XCB_INPUT_RAW_TOUCH_UPDATE 23

typedef xcb_input_raw_touch_begin_event_t xcb_input_raw_touch_update_event_t;

/** Opcode for xcb_input_raw_touch_end. */
#define XCB_INPUT_RAW_TOUCH_END 24

typedef xcb_input_raw_touch_begin_event_t xcb_input_raw_touch_end_event_t;

typedef enum xcb_input_gesture_pinch_event_flags_t {
    XCB_INPUT_GESTURE_PINCH_EVENT_FLAGS_GESTURE_PINCH_CANCELLED = 1
} xcb_input_gesture_pinch_event_flags_t;

/** Opcode for xcb_input_gesture_pinch_begin. */
#define XCB_INPUT_GESTURE_PINCH_BEGIN 27

/**
 * @brief xcb_input_gesture_pinch_begin_event_t
 **/
typedef struct xcb_input_gesture_pinch_begin_event_t {
    uint8_t                   response_type;
    uint8_t                   extension;
    uint16_t                  sequence;
    uint32_t                  length;
    uint16_t                  event_type;
    xcb_input_device_id_t     deviceid;
    xcb_timestamp_t           time;
    uint32_t                  detail;
    xcb_window_t              root;
    xcb_window_t              event;
    xcb_window_t              child;
    uint32_t                  full_sequence;
    xcb_input_fp1616_t        root_x;
    xcb_input_fp1616_t        root_y;
    xcb_input_fp1616_t        event_x;
    xcb_input_fp1616_t        event_y;
    xcb_input_fp1616_t        delta_x;
    xcb_input_fp1616_t        delta_y;
    xcb_input_fp1616_t        delta_unaccel_x;
    xcb_input_fp1616_t        delta_unaccel_y;
    xcb_input_fp1616_t        scale;
    xcb_input_fp1616_t        delta_angle;
    xcb_input_device_id_t     sourceid;
    uint8_t                   pad0[2];
    xcb_input_modifier_info_t mods;
    xcb_input_group_info_t    group;
    uint32_t                  flags;
} xcb_input_gesture_pinch_begin_event_t;

/** Opcode for xcb_input_gesture_pinch_update. */
#define XCB_INPUT_GESTURE_PINCH_UPDATE 28

typedef xcb_input_gesture_pinch_begin_event_t xcb_input_gesture_pinch_update_event_t;

/** Opcode for xcb_input_gesture_pinch_end. */
#define XCB_INPUT_GESTURE_PINCH_END 29

typedef xcb_input_gesture_pinch_begin_event_t xcb_input_gesture_pinch_end_event_t;

typedef enum xcb_input_gesture_swipe_event_flags_t {
    XCB_INPUT_GESTURE_SWIPE_EVENT_FLAGS_GESTURE_SWIPE_CANCELLED = 1
} xcb_input_gesture_swipe_event_flags_t;

/** Opcode for xcb_input_gesture_swipe_begin. */
#define XCB_INPUT_GESTURE_SWIPE_BEGIN 30

/**
 * @brief xcb_input_gesture_swipe_begin_event_t
 **/
typedef struct xcb_input_gesture_swipe_begin_event_t {
    uint8_t                   response_type;
    uint8_t                   extension;
    uint16_t                  sequence;
    uint32_t                  length;
    uint16_t                  event_type;
    xcb_input_device_id_t     deviceid;
    xcb_timestamp_t           time;
    uint32_t                  detail;
    xcb_window_t              root;
    xcb_window_t              event;
    xcb_window_t              child;
    uint32_t                  full_sequence;
    xcb_input_fp1616_t        root_x;
    xcb_input_fp1616_t        root_y;
    xcb_input_fp1616_t        event_x;
    xcb_input_fp1616_t        event_y;
    xcb_input_fp1616_t        delta_x;
    xcb_input_fp1616_t        delta_y;
    xcb_input_fp1616_t        delta_unaccel_x;
    xcb_input_fp1616_t        delta_unaccel_y;
    xcb_input_device_id_t     sourceid;
    uint8_t                   pad0[2];
    xcb_input_modifier_info_t mods;
    xcb_input_group_info_t    group;
    uint32_t                  flags;
} xcb_input_gesture_swipe_begin_event_t;

/** Opcode for xcb_input_gesture_swipe_update. */
#define XCB_INPUT_GESTURE_SWIPE_UPDATE 31

typedef xcb_input_gesture_swipe_begin_event_t xcb_input_gesture_swipe_update_event_t;

/** Opcode for xcb_input_gesture_swipe_end. */
#define XCB_INPUT_GESTURE_SWIPE_END 32

typedef xcb_input_gesture_swipe_begin_event_t xcb_input_gesture_swipe_end_event_t;

/**
 * @brief xcb_input_event_for_send_t
 **/
typedef union xcb_input_event_for_send_t {
    xcb_input_device_valuator_event_t            device_valuator;
    xcb_input_device_key_press_event_t           device_key_press;
    xcb_input_device_key_release_event_t         device_key_release;
    xcb_input_device_button_press_event_t        device_button_press;
    xcb_input_device_button_release_event_t      device_button_release;
    xcb_input_device_motion_notify_event_t       device_motion_notify;
    xcb_input_device_focus_in_event_t            device_focus_in;
    xcb_input_device_focus_out_event_t           device_focus_out;
    xcb_input_proximity_in_event_t               proximity_in;
    xcb_input_proximity_out_event_t              proximity_out;
    xcb_input_device_state_notify_event_t        device_state_notify;
    xcb_input_device_mapping_notify_event_t      device_mapping_notify;
    xcb_input_change_device_notify_event_t       change_device_notify;
    xcb_input_device_key_state_notify_event_t    device_key_state_notify;
    xcb_input_device_button_state_notify_event_t device_button_state_notify;
    xcb_input_device_presence_notify_event_t     device_presence_notify;
    xcb_raw_generic_event_t                      event_header;
} xcb_input_event_for_send_t;

/**
 * @brief xcb_input_event_for_send_iterator_t
 **/
typedef struct xcb_input_event_for_send_iterator_t {
    xcb_input_event_for_send_t *data;
    int                         rem;
    int                         index;
} xcb_input_event_for_send_iterator_t;

/** Opcode for xcb_input_send_extension_event. */
#define XCB_INPUT_SEND_EXTENSION_EVENT 31

/**
 * @brief xcb_input_send_extension_event_request_t
 **/
typedef struct xcb_input_send_extension_event_request_t {
    uint8_t      major_opcode;
    uint8_t      minor_opcode;
    uint16_t     length;
    xcb_window_t destination;
    uint8_t      device_id;
    uint8_t      propagate;
    uint16_t     num_classes;
    uint8_t      num_events;
    uint8_t      pad0[3];
} xcb_input_send_extension_event_request_t;

/** Opcode for xcb_input_device. */
#define XCB_INPUT_DEVICE 0

/**
 * @brief xcb_input_device_error_t
 **/
typedef struct xcb_input_device_error_t {
    uint8_t  response_type;
    uint8_t  error_code;
    uint16_t sequence;
    uint32_t bad_value;
    uint16_t minor_opcode;
    uint8_t  major_opcode;
} xcb_input_device_error_t;

/** Opcode for xcb_input_event. */
#define XCB_INPUT_EVENT 1

/**
 * @brief xcb_input_event_error_t
 **/
typedef struct xcb_input_event_error_t {
    uint8_t  response_type;
    uint8_t  error_code;
    uint16_t sequence;
    uint32_t bad_value;
    uint16_t minor_opcode;
    uint8_t  major_opcode;
} xcb_input_event_error_t;

/** Opcode for xcb_input_mode. */
#define XCB_INPUT_MODE 2

/**
 * @brief xcb_input_mode_error_t
 **/
typedef struct xcb_input_mode_error_t {
    uint8_t  response_type;
    uint8_t  error_code;
    uint16_t sequence;
    uint32_t bad_value;
    uint16_t minor_opcode;
    uint8_t  major_opcode;
} xcb_input_mode_error_t;

/** Opcode for xcb_input_device_busy. */
#define XCB_INPUT_DEVICE_BUSY 3

/**
 * @brief xcb_input_device_busy_error_t
 **/
typedef struct xcb_input_device_busy_error_t {
    uint8_t  response_type;
    uint8_t  error_code;
    uint16_t sequence;
    uint32_t bad_value;
    uint16_t minor_opcode;
    uint8_t  major_opcode;
} xcb_input_device_busy_error_t;

/** Opcode for xcb_input_class. */
#define XCB_INPUT_CLASS 4

/**
 * @brief xcb_input_class_error_t
 **/
typedef struct xcb_input_class_error_t {
    uint8_t  response_type;
    uint8_t  error_code;
    uint16_t sequence;
    uint32_t bad_value;
    uint16_t minor_opcode;
    uint8_t  major_opcode;
} xcb_input_class_error_t;


// #ifdef __cplusplus
// }
// #endif

#endif

/**
 * @}
 */
