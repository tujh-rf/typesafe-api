
#include <iostream>
#include <string>
#include <memory>

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
// C++ with classes

struct device_open_exception: public std::exception {
  device_open_exception( const uint8_t param ):
    _param( param )
  {
    exception_text = "cannot open connection to the device "
      + std::to_string( param );
  }

  const char* what() const throw( ) override {
    return exception_text.c_str();
  }

  uint8_t _param;
  std::string exception_text;
};

class device {
public:
  device( const uint8_t param ):
    m_id( open_device( param ) )
  {
    if( m_id == INVALID_DEVICE_ID ) {
      throw device_open_exception( param );
    }
  }
  ~device() {
    if( m_id != INVALID_DEVICE_ID ) {
      close_device( m_id );
    }
  }

  inline device_id get_device_id() {
    return m_id;
  }

  bool write8( const uint8_t page, const uint8_t address, const uint8_t value ) {
    uint8_t writable_value = value;
    return ( device_write( m_id, page, address, &writable_value, sizeof( writable_value ) ) >= 0 );
  }
  bool write16( const uint8_t page, const uint8_t address, const uint16_t value ) {
    uint16_t correct_value = swap_bytes( value );
    return ( device_write( m_id, page, address, &correct_value, sizeof( correct_value ) ) >= 0 );
  }

  bool read8( const uint8_t page, const uint8_t address, uint8_t& value ) {
    return ( device_read( m_id, page, address, &value, sizeof( value ) ) >= 0 );
  }
  bool read16( uint8_t page, uint8_t address, uint16_t& value ) {
    uint16_t local_value;
    if( device_read( m_id, page, address, &local_value, sizeof( local_value ) ) < 0 ) {
      return false;
    }

    value = swap_bytes( local_value );
    return true;
  }

protected:
  inline static uint16_t swap_bytes( const uint16_t in ) {
    return ( ( static_cast<uint8_t>( in & 0xFF ) << 8 ) | ( in & 0xFF00 ) >> 8 );
  }

private:
  device_id m_id = INVALID_DEVICE_ID;

};

//------------------------------------------------------------------------------

bool communicate_to_device_1() {
  static const uint8_t device_param = 1;

  std::unique_ptr< device > dev;

  try {
    dev = std::make_unique< device >( device_param );
  }
  catch( device_open_exception & ex ) {
    std::cout << ex.what() << std::endl;
    return false;
  }

  uint8_t power_on = 0xFD;
  if( !dev->write16( 0x00, 0x00, power_on ) ) { // wrong function 8/16
    return false;
  }

  uint8_t device_value;
  if( !dev->read8( 0x10, 0xA0, device_value ) ) {
    return false;
  }

  if( device_value != 42 ) {
    return false;
  }

  return true;
}

bool communicate_to_device_2() {
  static const uint8_t device_param = 2;
  std::unique_ptr< device > dev = std::make_unique< device >( device_param );

  uint16_t hello_message = 0x100;
  if( !dev->write16( 0x00, 0xFF, hello_message ) ) { // still wrong address
    return false;
  }

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
