
#include <iostream>

#include "../device/device.h"

/*! TASK
 *
 * 1. Device 1 communication
 *   a. write 'power on' message - write 8 bites value 0xFD to the page 0x00, address 0x00
 *   b. read universal answer as 8 bit value - page 0x10, address 0xA0, value will be '42'
 * 2. Device 2 communication
 *   a. write 'hello message' - write 16 bites value 0x100 to the page 0xAA, address 0xFF
 */

//------------------------------------------------------------------------------
// C'ish implementation

#define SWAPBYTES(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))

bool communicate_to_device_1()
{
  device_id id = open_device( 1 );
  if( id == INVALID_DEVICE_ID ) {
    return false;
  }

  uint8_t power_on = 0xFD;
  if( device_write( id, 0x00, 0x00, &power_on, sizeof( power_on ) ) < 0 ) {
    close_device( id );                     // don't forget to close device
    return false;
  }

  uint8_t device_value;
  if( device_read( id, 0x10, 0xA0, &device_value, sizeof( device_value ) ) < 0 ) {
    return false;                           // who will close the device ?
  }

  if( device_value != 42 ) {
    close_device( id );                     // don't forget to close device
    return false;
  }

  close_device( id );                       // don't forget to close device
  return true;
}

bool communicate_to_device_2() {
  device_id id = open_device( 2 );          // where the check ?
  uint16_t hello_message = SWAPBYTES( 0x100 );

  if( device_write( id, 0x00, 0xFF, &hello_message, sizeof( hello_message ) ) < 0 ) { // wrong address
    close_device( id );
    return false;
  }

  close_device( id );
  return true;
}

//------------------------------------------------------------------------------

int main() {
  bool ok = true;
  if( !communicate_to_device_1() ) {
    std::cout << "[device1] communication error" << std::endl;
    ok = false;
  }

  if( !communicate_to_device_2() ) {
    std::cout << "[device2] communication error" << std::endl;
    ok = false;
  }

  if( ok ) {
    std::cout << "tasks were completed" << std::endl;
  }

  return 0;
}
