#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "usbh_hid_reportparser.h"


// flags for joystick components required
#define JOY_MOUSE_REQ_AXIS_X  0x01
#define JOY_MOUSE_REQ_AXIS_Y  0x02
#define JOY_MOUSE_REQ_BTN_0   0x04
#define JOY_MOUSE_REQ_BTN_1   0x08
#define JOYSTICK_COMPLETE     (JOY_MOUSE_REQ_AXIS_X | JOY_MOUSE_REQ_AXIS_Y | JOY_MOUSE_REQ_BTN_0)
#define MOUSE_COMPLETE        (JOY_MOUSE_REQ_AXIS_X | JOY_MOUSE_REQ_AXIS_Y | JOY_MOUSE_REQ_BTN_0 | JOY_MOUSE_REQ_BTN_1)

#define USAGE_PAGE_GENERIC_DESKTOP  1
#define USAGE_PAGE_SIMULATION       2
#define USAGE_PAGE_VR               3
#define USAGE_PAGE_SPORT            4
#define USAGE_PAGE_GAMING           5
#define USAGE_PAGE_GENERIC_DEVICE   6
#define USAGE_PAGE_KEYBOARD         7
#define USAGE_PAGE_LEDS             8
#define USAGE_PAGE_BUTTON           9
#define USAGE_PAGE_ORDINAL         10
#define USAGE_PAGE_TELEPHONY       11
#define USAGE_PAGE_CONSUMER        12


#define USAGE_POINTER   1
#define USAGE_MOUSE     2
#define USAGE_JOYSTICK  4
#define USAGE_GAMEPAD   5
#define USAGE_KEYBOARD  6
#define USAGE_KEYPAD    7
#define USAGE_MULTIAXIS 8

#define USAGE_X       48
#define USAGE_Y       49
#define USAGE_Z       50
#define USAGE_WHEEL   56
#define USAGE_HAT     57


typedef struct {
  uint8_t bSize: 2;
  uint8_t bType: 2;
  uint8_t bTag: 4;
} __attribute__((packed)) item_t;


int report_is_usable(uint16_t bit_count, uint8_t report_complete, hid_report_t *conf) {
// hidp_debugf("  - total bit count: %d (%d bytes, %d bits)",
//	      bit_count, bit_count/8, bit_count%8);

  conf->report_size = bit_count/8;

  // check if something useful was detected
  if( ((conf->type == REPORT_TYPE_JOYSTICK) && ((report_complete & JOYSTICK_COMPLETE) == JOYSTICK_COMPLETE)) ||
      ((conf->type == REPORT_TYPE_MOUSE)    && ((report_complete & MOUSE_COMPLETE) == MOUSE_COMPLETE)) ||
      ((conf->type == REPORT_TYPE_KEYBOARD))) {
 //   hidp_debugf("  - report %d is usable", conf->report_id);
    return 1;
  }

//  hidp_debugf("  - unusable report %d", conf->report_id);
  return 0;
}



int parse_report_descriptor(uint8_t *rep, uint16_t rep_size,hid_report_t *conf) {
  int8_t app_collection = 0;
  int8_t phys_log_collection = 0;
  uint8_t skip_collection = 0;
  int8_t generic_desktop = -1;   // depth at which first gen_desk was found
  uint8_t collection_depth = 0;

  uint8_t i;

  //
  uint8_t report_size, report_count;
  uint16_t bit_count = 0, usage_count = 0;
  uint16_t logical_minimum=0, logical_maximum=0;

  // mask used to check of all required components have been found, so
  // that e.g. both axes and the button of a joystick are ready to be used
  uint8_t report_complete = 0;

  // joystick/mouse components
  int8_t axis[2] = { -1, -1};
  uint8_t btns = 0;
  int8_t hat = -1;


  while(rep_size) {
    // extract short item
    uint8_t tag = ((item_t*)rep)->bTag;
    uint8_t type = ((item_t*)rep)->bType;
    uint8_t size = ((item_t*)rep)->bSize;

    rep++;
    rep_size--;   // one byte consumed



    uint32_t value = 0;
     if(size) {      // size 1/2/3
       value = *rep++;
       rep_size--;
     }

     if(size > 1) {  // size 2/3
       value = (value & 0xff) + ((uint32_t)(*rep++)<<8);
       rep_size--;
     }

     if(size > 2) {  // size 3
       value &= 0xffff;
       value |= ((uint32_t)(*rep++)<<16);
       value |= ((uint32_t)(*rep++)<<24);
       rep_size-=2;
     }


       // we are currently skipping an unknown/unsupported collection)
           if(skip_collection) {
             if(!type) {  // main item
       				// any new collection increases the depth of collections to skip
       				if(tag == 10) {
       					skip_collection++;
       					collection_depth++;
       				}

       				// any end collection decreases it
       				if(tag == 12) {
       					skip_collection--;
       					collection_depth--;

       					// leaving the depth the generic desktop was valid for
       					if(generic_desktop > collection_depth)
       						generic_desktop = -1;
       				}
             }

           } else {


                 switch(type) {
                 case 0:
           	// main item

           	switch(tag) {
           	case 8:
           	  // handle found buttons
           	  if(btns) {
           	    if((conf->type == REPORT_TYPE_JOYSTICK) ||
           	       (conf->type == REPORT_TYPE_MOUSE)) {
           	      // scan for up to four buttons
           	      char b;
           	      for(b=0;b<12;b++) {
           		if(report_count > b) {
           		  uint16_t this_bit = bit_count+b;

           		  conf->joystick_mouse.button[b].byte_offset = this_bit/8;
           		  conf->joystick_mouse.button[b].bitmask = 1 << (this_bit%8);
           		}
           					conf->joystick_mouse.button_count = report_count * report_size;
           	      }

           	      // we found at least one button which is all we want to accept this as a valid
           	      // joystick
           	      report_complete |= JOY_MOUSE_REQ_BTN_0;
           	      if(report_count > 1) report_complete |= JOY_MOUSE_REQ_BTN_1;
           	    }
           	  }

           	  // handle found axes
           	  char c;
           	  for(c=0;c<2;c++) {
           	    if(axis[c] >= 0) {
           	      uint16_t cnt = bit_count + report_size * axis[c];

           	      if((conf->type == REPORT_TYPE_JOYSTICK) || (conf->type == REPORT_TYPE_MOUSE)) {
           		// save in joystick report
           		conf->joystick_mouse.axis[c].offset = cnt;
           		conf->joystick_mouse.axis[c].size = report_size;
           		conf->joystick_mouse.axis[c].logical.min = logical_minimum;
           		conf->joystick_mouse.axis[c].logical.max = logical_maximum;
           		conf->joystick_mouse.axis[c].size = report_size;
           		if(c==0) report_complete |= JOY_MOUSE_REQ_AXIS_X;
           		if(c==1) report_complete |= JOY_MOUSE_REQ_AXIS_Y;
           	      }
           	    }
           	  }

           	  // handle found hat
           	  if(hat >= 0) {
           	    uint16_t cnt = bit_count + report_size * hat;
           	    if(conf->type == REPORT_TYPE_JOYSTICK) {
           	      conf->joystick_mouse.hat.offset = cnt;
           	      conf->joystick_mouse.hat.size = report_size;
           	    }
           	  }


           	  // reset for next inputs
           	  bit_count += report_count * report_size;
           	  usage_count = 0;
           	  btns = 0;
           	  axis[0] = axis[1] = -1;
           	  hat = -1;
           	  break;

           	case 9:
           	  break;

           	case 11:
           	  break;

           	case 10:
           	  collection_depth++;
           	  usage_count = 0;

           	  if(value == 1) {	   // app collection
           	    app_collection++;
           	  } else if(value == 0) {  // physical collection
           	   // hidp_extreme_debugf("  -> physical");
           	    phys_log_collection++;
           	  } else if(value == 2) {  // logical collection
           	    phys_log_collection++;
           	  } else {
           	    skip_collection++;
           	  }
           	  break;

           	case 12:
           	  collection_depth--;

           	  // leaving the depth the generic desktop was valid for
           	  if(generic_desktop > collection_depth)
           	    generic_desktop = -1;

           	  if(phys_log_collection) {

           	    phys_log_collection--;
           	  } else if(app_collection) {

           	    app_collection--;
           	    // check if report is usable and stop parsing if it is
           	     if(report_is_usable(bit_count, report_complete, conf))
           	      return 1;
           	    else {
           	      // retry with next report
           	      bit_count = 0;
           	      report_complete = 0;
           	    }

           	  } else {
           	    return 0;
           	  }
           	  break;

           	default:
           	  return 0;
           	  break;
           	}
           	break;

                 case 1:
           	// global item
           	switch(tag) {
           	case 0:

           	  if(value == USAGE_PAGE_KEYBOARD) {
           	  } else if(value == USAGE_PAGE_GAMING) {
           	  } else if(value == USAGE_PAGE_LEDS) {
           	  } else if(value == USAGE_PAGE_CONSUMER) {
           	  } else if(value == USAGE_PAGE_BUTTON) {
           	    btns = 1;
           	  } else if(value == USAGE_PAGE_GENERIC_DESKTOP) {
           	    if(generic_desktop < 0)
           	      generic_desktop = collection_depth;
           	  } else

           	  break;

           	case 1:
           	  logical_minimum = value;
           	  break;

           	case 2:
           	  logical_maximum = value;
           	  break;

           	case 3:
           	  break;

           	case 4:
           	  break;

           	case 5:
           	  break;

           	case 6:
           	  break;

           	case 7:
           	  report_size = value;
           		break;

           	case 8:
           	  conf->report_id = value;
           	  break;

           	case 9:
           		report_count = value;
           	  break;

           	default:
           	  return 0;
           	  break;
           	}
           	break;

                 case 2:
           	// local item
           	switch(tag) {
           	case 0:
           	  // we only support mice, keyboards and joysticks
           	  if( !collection_depth && (value == USAGE_KEYBOARD)) {
           	    // usage(keyboard) is always allowed
           	    conf->type = REPORT_TYPE_KEYBOARD;
           	  } else if(!collection_depth && (value == USAGE_MOUSE)) {
           	    // usage(mouse) is always allowed
           	    conf->type = REPORT_TYPE_MOUSE;
           	  } else if(!collection_depth &&
           		    ((value == USAGE_GAMEPAD) || (value == USAGE_JOYSTICK))) {
           	    conf->type = REPORT_TYPE_JOYSTICK;
           	  } else if(value == USAGE_POINTER && app_collection) {
           	    // usage(pointer) is allowed within the application collection
           	  } else if((value == USAGE_X || value == USAGE_Y) && app_collection) {
           	    // usage(x) and usage(y) are allowed within the app collection
           	   // hidp_extreme_debugf(" -> axis usage");

           	    // we support x and y axis on mice and joysticks
           	    if((conf->type == REPORT_TYPE_JOYSTICK) || (conf->type == REPORT_TYPE_MOUSE)) {
           	      if(value == USAGE_X) {
           		axis[0] = usage_count;
           	      }
           	      if(value == USAGE_Y) {
           		axis[1] = usage_count;
           	      }
           	    }
           	  } else if((value == USAGE_HAT) && app_collection) {
           	    // usage(hat) is allowed within the app collection
           	    // we support hat on joysticks only
           	    if(conf->type == REPORT_TYPE_JOYSTICK) {
           	     // hidp_extreme_debugf("JOYSTICK: found hat @ %d", usage_count);
           	      hat = usage_count;
           	    }
           	  } else {

           	  }

           	  usage_count++;
           	  break;

           	case 1:

           	  usage_count -= (value-1);
           	  break;

           	case 2:

           	  usage_count += value;
           	  break;

           	default:

           	  break;
           	}
           	break;

                 default:
           	// reserved

           	break;
                 }
               }
             }

             // if we get here then no usable setup was found
             return 0;
           }





