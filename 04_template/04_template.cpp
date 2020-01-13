
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
// dark magic of templates

struct device_open_exception: public std::exception {
  device_open_exception( const uint8_t param ):
    _param( param ) {
    exception_text = "cannot open connection to the device "
      + std::to_string( param );
  }

  const char* what() const throw( ) override {
    return exception_text.c_str();
  }

  uint8_t _param;
  std::string exception_text;
};

template<size_t Page, size_t Address, typename DataType>
struct device_data {
  typedef DataType data_type;

  enum {
    PAGE = Page,
    ADDRESS = Address
  };

  device_data( DataType _value = 0 ):
    value( _value )
  {
  }
  DataType value;
};

namespace device_memory {

static const device_data< 0x00, 0x00, uint8_t > power_on ( 0xFD  );
static const device_data< 0xAA, 0xFF, uint16_t> hello;
static const device_data< 0x10, 0xA0, uint8_t > readable_value;

}

class device {
public:
  device( const uint8_t param ):
    m_id( open_device( param ) ) {
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

  template<typename Data>
  bool write( const Data &data );

  template<typename Data>
  bool read( Data& data );

private:
  device_id m_id = INVALID_DEVICE_ID;

};

static uint16_t swap_bytes( const uint16_t in ) {
  return ( ( static_cast<uint8_t>( in & 0xFF ) << 8 ) | ( in & 0xFF00 ) >> 8 );
}

template<typename A>
struct data_dispatch;

template<>
struct data_dispatch< uint8_t > {
  static bool write( const device_id id, const uint8_t page, const uint8_t address, const uint8_t value ) {
    uint8_t value_to_write = value;
    return ( device_write( id, page, address, &value_to_write, sizeof( value_to_write ) ) >= 0 );
  }
};

template<>
struct data_dispatch< uint16_t > {
  static bool write( const device_id id, const uint8_t page, const uint8_t address, const uint16_t value ) {
    uint16_t value_to_write = swap_bytes( value );
    return ( device_write( id, page, address, &value_to_write, sizeof( value_to_write ) ) >= 0 );
  }
};

template<typename A>
struct data_receive;

template<>
struct data_receive< uint8_t > {
  static bool read( const device_id id, const uint8_t page, const uint8_t address, uint8_t &value ) {
    return ( device_read( id, page, address, &value, sizeof( value ) ) >= 0 );
  }
};

template<>
struct data_receive< uint16_t > {
  static bool read( const device_id id, const uint8_t page, const uint8_t address, uint16_t& value ) {
    uint16_t local_value;
    if( device_read( id, page, address, &local_value, sizeof( local_value ) ) < 0 ) {
      return false;
    }

    value = swap_bytes( local_value );
    return true;
  }
};

template<typename Data>
inline bool device::write( const Data& data ) {
  return data_dispatch< typename Data::data_type >::write( this->m_id, data.PAGE, data.ADDRESS, data.value );
}

template<typename Data>
inline bool device::read( Data& data ) {
  return data_receive< typename Data::data_type >::read( this->m_id, data.PAGE, data.ADDRESS, data.value );
}

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

  if( !dev->write( device_memory::power_on ) ) {
    return false;
  }

  auto memory_value = device_memory::readable_value;
  if( !dev->read( memory_value ) ) {
    return false;
  }

  if( memory_value.value != 42 ) {
    return false;
  }

  return true;
}

bool communicate_to_device_2() {
  static const uint8_t device_param = 2;
  std::unique_ptr< device > dev = std::make_unique< device >( device_param );

  auto hello_message = device_memory::hello;
  hello_message.value = 0x100;
  if( !dev->write( hello_message ) ) {
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
