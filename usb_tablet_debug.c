/* USB Mouse Plus Debug Channel Example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_mouse.html
 * Copyright (c) 2009 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// Version 1.0: Initial Release
// Version 1.1: Add support for Teensy 2.0

#define USB_SERIAL_PRIVATE_INCLUDE
#include "usb_tablet_debug.h"

/**************************************************************************
 *
 *  Configurable Options
 *
 **************************************************************************/

// You can change these to give your code its own name.
#define STR_MANUFACTURER	L"MfgName"
#define STR_PRODUCT		L"Sonovabitch"


// Mac OS-X and Linux automatically load the correct drivers.  On
// Windows, even though the driver is supplied by Microsoft, an
// INF file is needed to load the driver.  These numbers need to
// match the INF file.
#define VENDOR_ID		0x16C0
#define PRODUCT_ID		0x0497


// USB devices are supposed to implment a halt feature, which is
// rarely (if ever) used.  If you comment this line out, the halt
// code will be removed, saving 102 bytes of space (gcc 4.3.0).
// This is not strictly USB compliant, but works with all major
// operating systems.
#define SUPPORT_ENDPOINT_HALT



/**************************************************************************
 *
 *  Endpoint Buffer Configuration
 *
 **************************************************************************/

#define ENDPOINT0_SIZE		32

#define MOUSE_INTERFACE		0
#define MOUSE_ENDPOINT		3
#define MOUSE_SIZE		9
#define MOUSE_BUFFER		EP_DOUBLE_BUFFER

#define DEBUG_INTERFACE		1
#define DEBUG_TX_ENDPOINT	4
#define DEBUG_TX_SIZE		32
#define DEBUG_TX_BUFFER		EP_DOUBLE_BUFFER

static const uint8_t PROGMEM endpoint_config_table[] = {
	0,
	0,
	1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(MOUSE_SIZE) | MOUSE_BUFFER,
	1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(DEBUG_TX_SIZE) | DEBUG_TX_BUFFER
};


/**************************************************************************
 *
 *  Descriptor Data
 *
 **************************************************************************/

// Descriptors are the data that your computer reads when it auto-detects
// this USB device (called "enumeration" in USB lingo).  The most commonly
// changed items are editable at the top of this file.  Changing things
// in here should only be done by those who've read chapter 9 of the USB
// spec and relevant portions of any USB class specifications!


static uint8_t PROGMEM device_descriptor[] = {
	18,					// bLength
	1,					// bDescriptorType
	0x00, 0x02,				// bcdUSB
	0,					// bDeviceClass
	0,					// bDeviceSubClass
	0,					// bDeviceProtocol
	ENDPOINT0_SIZE,				// bMaxPacketSize0
	LSB(VENDOR_ID), MSB(VENDOR_ID),		// idVendor
	LSB(PRODUCT_ID), MSB(PRODUCT_ID),	// idProduct
	0x00, 0x01,				// bcdDevice
	1,					// iManufacturer
	2,					// iProduct
	0,					// iSerialNumber
	1					// bNumConfigurations
};

// Mouse Protocol 1, HID 1.11 spec, Appendix B, page 59-60, with wheel extension
static uint8_t PROGMEM mouse_hid_report_desc[] = {
0x05, 0x0D, 					// USAGE_PAGE (Digitizers) 
0x09, 0x01, 					// USAGE (Digitizer) 
0xA1, 0x01,  					// COLLECTION (Application) 
0x09, 0x02, 					//   USAGE (pen) 
0xA1, 0x02,  					//   COLLECTION (Logical) 
0x05, 0x01, 					//     USAGE_PAGE (Generic Desktop) 
0x09, 0x30, 					//     USAGE (X) 
0x16, 0x60,	0x03,				//	   LOGICAL_MINIMUM (0x0360)
0x26, 0x30, 0x62,				//     LOGICAL_MAXIMUM (0x6230) 
0x35, 0x00,						//     PHYSICAL_Minimum (0)
0x47, 0x00, 0xD0, 0x00, 0x00,	//     PHYSICAL_MAXIMUM (36750) *53248
0x65, 0x13,  					//     UNIT (Eng Lin:Distance) 
0x55, 0x0D,  					//     UNIT_EXPONENT (-3) 
0x75, 0x10,  					//     REPORT_SIZE (16) 
0x95, 0x01,  					//     REPORT_COUNT (1) 
0x81, 0x02,  					//     INPUT (Data,Var,Abs) 
0x09, 0x31, 					//     USAGE (Y)
0x16, 0x60,	0x03,				//	   LOGICAL_MINIMUM (0x0360)
0x26, 0x70, 0x3C,				//     LOGICAL_MAXIMUM (0x3c70) 
0x35, 0x00,						//     PHYSICAL_Minimum (0)
0x47, 0x00, 0xD0, 0x00, 0x00,	//     PHYSICAL_MAXIMUM (36750) *53248
0x65, 0x13,  					//     UNIT (Eng Lin:Distance) 
0x55, 0x0D,  					//     UNIT_EXPONENT (-3) 
0x75, 0x10,  					//     REPORT_SIZE (16) 
0x95, 0x01,  					//     REPORT_COUNT (1) 
0x81, 0x02,  					//     INPUT (Data,Var,Abs) 
0x05, 0x0D, 					//     USAGE_PAGE (Digitizers) 
0x09, 0x32, 					//     USAGE (In Range) 
0x09, 0x44, 					//     USAGE (Barrel Switch) 
0x09, 0x42, 					//     USAGE (Tip Switch)
0x09, 0x35,						//	   USAGE (Tap)
0x15, 0x00,  					//     LOGICAL_MINIMUM (0) 
0x25, 0x01,  					//     LOGICAL_MAXIMUM (1) 
0x35, 0x00,  					//     PHYSICAL_MINIMUM (0) 
0x45, 0x01,  					//     PHYSICAL_MAXIMUM (1) 
0x75, 0x01,  					//     REPORT_SIZE (1) 
0x95, 0x03,  					//     REPORT_COUNT (4) 
0x65, 0x00,  					//     UNIT (None) 
0x81, 0x02,  					//     INPUT (Data,Var,Abs) 
0x95, 0x01,  					//     REPORT_COUNT (1) 
0x75, 0x05,  					//     REPORT_SIZE (4) 
0x81, 0x03,  					//     INPUT (Cnst,Var,Abs) 
0x09, 0x30, 					//     USAGE (Tip Pressure) 
0x15, 0x00,  					//     LOGICAL_MINIMUM (0) 
0x25, 0x7F,  					//     LOGICAL_MAXIMUM (127) 
0x35, 0x00,  					//     PHYSICAL_MINIMUM (0) 
0x45, 0x2D,  					//     PHYSICAL_MAXIMUM (45) 
0x67, 0x11, 0xE1, 0x00, 0x00,  	//     UNIT (SI Lin:Force) 
0x55, 0x04,  					//     UNIT_EXPONENT (4) 
0x75, 0x08,  					//     REPORT_SIZE (8) 
0x95, 0x01,  					//     REPORT_COUNT (1) 
0x81, 0x12,  					//     INPUT (Data,Var,Abs,NLin)
0x09, 0x56,                     //     USAGE (Scan Time)
0x66, 0x01, 0x10,               //     UNIT (Seconds)        
0x55, 0x0C,                     //     UNIT_EXPONENT (-4)           
0x47, 0xff, 0xff, 0x00, 0x00,	//     PHYSICAL_MAXIMUM (255)
0x27, 0xff, 0xff, 0x00, 0x00,	//     LOGICAL_MAXIMUM (255) 
0x75, 0x10,                     //     REPORT_SIZE (16)             
0x95, 0x01,                     //     REPORT_COUNT (1) 
0x81, 0x02,                     //     INPUT (Data,Var,Abs)         
0xC0, 							//   END_COLLECTION 
0xC0 							// END_COLLECTION 
};

static uint8_t PROGMEM debug_hid_report_desc[] = {
	0x06, 0x31, 0xFF,		// Usage Page 0xFF31 (vendor defined)
	0x09, 0x74,				// Usage 0x74
	0xA1, 0x53,				// Collection 0x53
	0x75, 0x08,				// report size = 8 bits
	0x15, 0x00,				// logical minimum = 0
	0x26, 0xFF, 0x00,		// logical maximum = 255
	0x95, DEBUG_TX_SIZE,	// report count
	0x09, 0x75,				// usage
	0x81, 0x02,				// Input (array)
	0xC0					// end collection
};

#define CONFIG1_DESC_SIZE        (9+9+9+7+9+9+7)
#define MOUSE_HID_DESC_OFFSET    (9+9)
#define DEBUG_HID_DESC_OFFSET    (9+9+9+7+9)
static uint8_t PROGMEM config1_descriptor[CONFIG1_DESC_SIZE] = {
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9, 					// bLength;
	2,					// bDescriptorType;
	LSB(CONFIG1_DESC_SIZE),			// wTotalLength
	MSB(CONFIG1_DESC_SIZE),
	2,					// bNumInterfaces
	1,					// bConfigurationValue
	0,					// iConfiguration
	0xC0,					// bmAttributes
	50,					// bMaxPower
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,					// bLength
	4,					// bDescriptorType
	MOUSE_INTERFACE,			// bInterfaceNumber
	0,					// bAlternateSetting
	1,					// bNumEndpoints
	0x03,					// bInterfaceClass (0x03 = HID)
	0x01,					// bInterfaceSubClass (0x01 = Boot)
	0x02,					// bInterfaceProtocol (0x02 = Mouse)
	0,					// iInterface
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,					// bLength TODO
	0x21,					// bDescriptorType
	0x11, 0x01,				// bcdHID
	0,					// bCountryCode
	1,					// bNumDescriptors
	0x22,					// bDescriptorType
	sizeof(mouse_hid_report_desc),		// wDescriptorLength
	0,
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,					// bLength
	5,					// bDescriptorType
	MOUSE_ENDPOINT | 0x80,			// bEndpointAddress
	0x03,					// bmAttributes (0x03=intr)
	MOUSE_SIZE, 0,					// wMaxPacketSize
	1,					// bInterval
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,					// bLength
	4,					// bDescriptorType
	DEBUG_INTERFACE,			// bInterfaceNumber
	0,					// bAlternateSetting
	1,					// bNumEndpoints
	0x03,					// bInterfaceClass (0x03 = HID)
	0x00,					// bInterfaceSubClass
	0x00,					// bInterfaceProtocol
	0,					// iInterface
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,					// bLength
	0x21,					// bDescriptorType
	0x11, 0x01,				// bcdHID
	0,					// bCountryCode
	1,					// bNumDescriptors
	0x22,					// bDescriptorType
	sizeof(debug_hid_report_desc),		// wDescriptorLength
	0,
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,					// bLength
	5,					// bDescriptorType
	DEBUG_TX_ENDPOINT | 0x80,		// bEndpointAddress
	0x03,					// bmAttributes (0x03=intr)
	DEBUG_TX_SIZE, 0,			// wMaxPacketSize
	1					// bInterval
};

// If you're desperate for a little extra code memory, these strings
// can be completely removed if iManufacturer, iProduct, iSerialNumber
// in the device desciptor are changed to zeros.
struct usb_string_descriptor_struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	int16_t wString[];
};
static struct usb_string_descriptor_struct PROGMEM string0 = {
	4,
	3,
	{0x0409}
};
static struct usb_string_descriptor_struct PROGMEM string1 = {
	sizeof(STR_MANUFACTURER),
	3,
	STR_MANUFACTURER
};
static struct usb_string_descriptor_struct PROGMEM string2 = {
	sizeof(STR_PRODUCT),
	3,
	STR_PRODUCT
};

// This table defines which descriptor data is sent for each specific
// request from the host (in wValue and wIndex).
static struct descriptor_list_struct {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint8_t		length;
} PROGMEM descriptor_list[] = {
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config1_descriptor, sizeof(config1_descriptor)},
	{0x2200, MOUSE_INTERFACE, mouse_hid_report_desc, sizeof(mouse_hid_report_desc)},
	{0x2100, MOUSE_INTERFACE, config1_descriptor+MOUSE_HID_DESC_OFFSET, 9},
	{0x2200, DEBUG_INTERFACE, debug_hid_report_desc, sizeof(debug_hid_report_desc)},
	{0x2100, DEBUG_INTERFACE, config1_descriptor+DEBUG_HID_DESC_OFFSET, 9},
	{0x0300, 0x0000, (const uint8_t *)&string0, 4},
	{0x0301, 0x0409, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER)},
	{0x0302, 0x0409, (const uint8_t *)&string2, sizeof(STR_PRODUCT)}
};
#define NUM_DESC_LIST (sizeof(descriptor_list)/sizeof(struct descriptor_list_struct))


/**************************************************************************
 *
 *  Variables - these are the only non-stack RAM usage
 *
 **************************************************************************/

// zero when we are not configured, non-zero when enumerated
static volatile uint8_t usb_configuration=0;

// the time remaining before we transmit any partially full
// packet, or send a zero length packet.
static volatile uint8_t debug_flush_timer=0;

// which buttons are currently pressed
// static uint8_t mouse_buttons=0;

// protocol setting from the host.  We use exactly the same report
// either way, so this variable only stores the setting since we
// are required to be able to report which setting is in use.
static uint8_t mouse_protocol=1;


/**************************************************************************
 *
 *  Public Functions - these are the API intended for the user
 *
 **************************************************************************/


// initialize USB
void usb_init(void)
{
	HW_CONFIG();
	USB_FREEZE();				// enable USB
	PLL_CONFIG();				// config PLL
	while (!(PLLCSR & (1<<PLOCK))) ;	// wait for PLL lock
	USB_CONFIG();				// start USB clock
	UDCON = 0;				// enable attach resistor
	usb_configuration = 0;
	UDIEN = (1<<EORSTE)|(1<<SOFE);
	sei();
}

// return 0 if the USB is not configured, or the configuration
// number selected by the HOST
uint8_t usb_configured(void)
{
	return usb_configuration;
}



// Move the mouse.  x, y and wheel are -127 to 127.  Use 0 for no movement.
int8_t usb_tablet_update(uint8_t xl, uint8_t xh, uint8_t yl, uint8_t yh, uint8_t buttons, uint8_t pressure, uint16_t time)
{
	uint8_t intr_state, timeout;

	if (!usb_configuration) return -1;
	
	intr_state = SREG;
	cli();
	UENUM = MOUSE_ENDPOINT;
	timeout = UDFNUML + 50;
	while (1) {
		// are we ready to transmit?
		if (UEINTX & (1<<RWAL)) break;
		SREG = intr_state;
		// has the USB gone offline?
		if (!usb_configuration) return -1;
		// have we waited too long?
		if (UDFNUML == timeout) return -1;
		// get ready to try checking again
		intr_state = SREG;
		cli();
		UENUM = MOUSE_ENDPOINT;
	}
	UEDATX = xl;
	UEDATX = xh;
	UEDATX = yl;
	UEDATX = yh;
	UEDATX = buttons;
	UEDATX = pressure;
	UEDATX = (time & 0x00FF);
	UEDATX = ((time & 0xFF00) >> 8);
	UEINTX = 0x3A;
	SREG = intr_state;
	return 0;
}

// transmit a character.  0 returned on success, -1 on error
int8_t usb_debug_putchar(uint8_t c)
{
	static uint8_t previous_timeout=0;
	uint8_t timeout, intr_state;

	// if we're not online (enumerated and configured), error
	if (!usb_configuration) return -1;
	// interrupts are disabled so these functions can be
	// used from the main program or interrupt context,
	// even both in the same program!
	intr_state = SREG;
	cli();
	UENUM = DEBUG_TX_ENDPOINT;
	// if we gave up due to timeout before, don't wait again
	if (previous_timeout) {
		if (!(UEINTX & (1<<RWAL))) {
			SREG = intr_state;
			return -1;
		}
		previous_timeout = 0;
	}
	// wait for the FIFO to be ready to accept data
	timeout = UDFNUML + 4;
	while (1) {
		// are we ready to transmit?
		if (UEINTX & (1<<RWAL)) break;
		SREG = intr_state;
		// have we waited too long?
		if (UDFNUML == timeout) {
			previous_timeout = 1;
			return -1;
		}
		// has the USB gone offline?
		if (!usb_configuration) return -1;
		// get ready to try checking again
		intr_state = SREG;
		cli();
		UENUM = DEBUG_TX_ENDPOINT;
	}
	// actually write the byte into the FIFO
	UEDATX = c;
	// if this completed a packet, transmit it now!
	if (!(UEINTX & (1<<RWAL))) {
		UEINTX = 0x3A;
		debug_flush_timer = 0;
	} else {
		debug_flush_timer = 2;
	}
	SREG = intr_state;
	return 0;
}


// immediately transmit any buffered output.
void usb_debug_flush_output(void)
{
	uint8_t intr_state;

	intr_state = SREG;
	cli();
	if (debug_flush_timer) {
		UENUM = DEBUG_TX_ENDPOINT;
		while ((UEINTX & (1<<RWAL))) {
			UEDATX = 0;
		}
		UEINTX = 0x3A;
		debug_flush_timer = 0;
	}
	SREG = intr_state;
}



/**************************************************************************
 *
 *  Private Functions - not intended for general user consumption....
 *
 **************************************************************************/



// USB Device Interrupt - handle all device-level events
// the transmit buffer flushing is triggered by the start of frame
//
ISR(USB_GEN_vect)
{
	uint8_t intbits, t;

        intbits = UDINT;
        UDINT = 0;
        if (intbits & (1<<EORSTI)) {
		UENUM = 0;
		UECONX = 1;
		UECFG0X = EP_TYPE_CONTROL;
		UECFG1X = EP_SIZE(ENDPOINT0_SIZE) | EP_SINGLE_BUFFER;
		UEIENX = (1<<RXSTPE);
		usb_configuration = 0;
        }
	if ((intbits & (1<<SOFI)) && usb_configuration) {
		t = debug_flush_timer;
		if (t) {
			debug_flush_timer = -- t;
			if (!t) {
				UENUM = DEBUG_TX_ENDPOINT;
				while ((UEINTX & (1<<RWAL))) {
					UEDATX = 0;
				}
				UEINTX = 0x3A;
			}
		}
	}
}



// Misc functions to wait for ready and send/receive packets
static inline void usb_wait_in_ready(void)
{
	while (!(UEINTX & (1<<TXINI))) ;
}
static inline void usb_send_in(void)
{
	UEINTX = ~(1<<TXINI);
}
static inline void usb_wait_receive_out(void)
{
	while (!(UEINTX & (1<<RXOUTI))) ;
}
static inline void usb_ack_out(void)
{
	UEINTX = ~(1<<RXOUTI);
}



// USB Endpoint Interrupt - endpoint 0 is handled here.  The
// other endpoints are manipulated by the user-callable
// functions, and the start-of-frame interrupt.
//
ISR(USB_COM_vect)
{
    uint8_t intbits;
	const uint8_t *list;
    const uint8_t *cfg;
	uint8_t i, n, len, en;
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
	uint16_t desc_val;
	const uint8_t *desc_addr;
	uint8_t	desc_length;

        UENUM = 0;
	intbits = UEINTX;
        if (intbits & (1<<RXSTPI)) {
                bmRequestType = UEDATX;
                bRequest = UEDATX;
                wValue = UEDATX;
                wValue |= (UEDATX << 8);
                wIndex = UEDATX;
                wIndex |= (UEDATX << 8);
                wLength = UEDATX;
                wLength |= (UEDATX << 8);
                UEINTX = ~((1<<RXSTPI) | (1<<RXOUTI) | (1<<TXINI));
                if (bRequest == GET_DESCRIPTOR) {
			list = (const uint8_t *)descriptor_list;
			for (i=0; ; i++) {
				if (i >= NUM_DESC_LIST) {
					UECONX = (1<<STALLRQ)|(1<<EPEN);  //stall
					return;
				}
				desc_val = pgm_read_word(list);
				if (desc_val != wValue) {
					list += sizeof(struct descriptor_list_struct);
					continue;
				}
				list += 2;
				desc_val = pgm_read_word(list);
				if (desc_val != wIndex) {
					list += sizeof(struct descriptor_list_struct)-2;
					continue;
				}
				list += 2;
				desc_addr = (const uint8_t *)pgm_read_word(list);
				list += 2;
				desc_length = pgm_read_byte(list);
				break;
			}
			len = (wLength < 256) ? wLength : 255;
			if (len > desc_length) len = desc_length;
			do {
				// wait for host ready for IN packet
				do {
					i = UEINTX;
				} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
				if (i & (1<<RXOUTI)) return;	// abort
				// send IN packet
				n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
				for (i = n; i; i--) {
					UEDATX = pgm_read_byte(desc_addr++);
				}
				len -= n;
				usb_send_in();
			} while (len || n == ENDPOINT0_SIZE);
			return;
        }
		if (bRequest == SET_ADDRESS) {
			usb_send_in();
			usb_wait_in_ready();
			UDADDR = wValue | (1<<ADDEN);
			return;
		}
		if (bRequest == SET_CONFIGURATION && bmRequestType == 0) {
			usb_configuration = wValue;
			usb_send_in();
			cfg = endpoint_config_table;
			for (i=1; i<5; i++) {
				UENUM = i;
				en = pgm_read_byte(cfg++);
				UECONX = en;
				if (en) {
					UECFG0X = pgm_read_byte(cfg++);
					UECFG1X = pgm_read_byte(cfg++);
				}
			}
        		UERST = 0x1E;
        		UERST = 0;
			return;
		}
		if (bRequest == GET_CONFIGURATION && bmRequestType == 0x80) {
			usb_wait_in_ready();
			UEDATX = usb_configuration;
			usb_send_in();
			return;
		}

		if (bRequest == GET_STATUS) {
			usb_wait_in_ready();
			i = 0;
			#ifdef SUPPORT_ENDPOINT_HALT
			if (bmRequestType == 0x82) {
				UENUM = wIndex;
				if (UECONX & (1<<STALLRQ)) i = 1;
				UENUM = 0;
			}
			#endif
			UEDATX = i;
			UEDATX = 0;
			usb_send_in();
			return;
		}
		#ifdef SUPPORT_ENDPOINT_HALT
		if ((bRequest == CLEAR_FEATURE || bRequest == SET_FEATURE)
		  && bmRequestType == 0x02 && wValue == 0) {
			i = wIndex & 0x7F;
			if (i >= 1 && i <= MAX_ENDPOINT) {
				usb_send_in();
				UENUM = i;
				if (bRequest == SET_FEATURE) {
					UECONX = (1<<STALLRQ)|(1<<EPEN);
				} else {
					UECONX = (1<<STALLRQC)|(1<<RSTDT)|(1<<EPEN);
					UERST = (1 << i);
					UERST = 0;
				}
				return;
			}
		}
		#endif
		if (wIndex == MOUSE_INTERFACE) {
			if (bmRequestType == 0xA1) {
				if (bRequest == HID_GET_REPORT) {
					usb_wait_in_ready();
					UEDATX = 0;
					UEDATX = 0;
					UEDATX = 0;
					usb_send_in();
					return;
				}
				if (bRequest == HID_GET_PROTOCOL) {
					usb_wait_in_ready();
					UEDATX = mouse_protocol;
					usb_send_in();
					return;
				}
			}
			if (bmRequestType == 0x21) {
				if (bRequest == HID_SET_PROTOCOL) {
					mouse_protocol = wValue;
					usb_send_in();
					return;
				}
			}
		}
		if (wIndex == DEBUG_INTERFACE) {
			if (bRequest == HID_GET_REPORT && bmRequestType == 0xA1) {
				len = wLength;
				do {
					// wait for host ready for IN packet
					do {
						i = UEINTX;
					} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
					if (i & (1<<RXOUTI)) return;	// abort
					// send IN packet
					n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
					for (i = n; i; i--) {
						UEDATX = 0;
					}
					len -= n;
					usb_send_in();
				} while (len || n == ENDPOINT0_SIZE);
				return;
			}
		}
	}
	UECONX = (1<<STALLRQ) | (1<<EPEN);	// stall
}


