
#include <cstdlib>
#include <cstdint>
#include <cstdio>

#pragma once

//------------------------------------------------------------------------------
// Interface

//! type of display ID
typedef uint8_t device_id;

#define INVALID_DEVICE_ID 0xFF

/*! \param[in]  param   input parameter to open device
 *
 *  \return valid device id or INVALID_DEVICE_ID if connection to the device is not possible
 *
 * Open device connection                                       */
device_id open_device( const uint8_t param );

/*! \param[in]  id  ID of device to close connection
 *
 * Close device connection                                      */
void close_device( device_id id );

/*! \param[in]   id          device ID
 *  \param[in]   page        memory page on device
 *  \param[in]   address     address on the memory page
 *  \param[in]   data_ptr    pointer to data
 *  \param[in]   data_len    size of data to write
 *
 *  \return length of written data or negative value if it's not possible to write
 *
 * Write any date to the device - black box                     */
int device_write( device_id id, uint8_t page, uint8_t address, void *data_ptr, size_t data_len );

/*! \param[in]   id          device ID
 *  \param[in]   page        memory page on device
 *  \param[in]   address     address on the memory page
 *  \param[in]   result_ptr  pointer to memory for a value
 *  \param[in]   result_len  size of value memory
 *
 *  \return length of read data or negative value if it's not possible to read
 *
 * Read any date from the device - black box                    */
int device_read( device_id id, uint8_t page, uint8_t address, void *result_ptr, size_t result_len );


//------------------------------------------------------------------------------
// Implementation - Black box for us

char symbols[] = {
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'
};

device_id open_device( const uint8_t param )
{
  // in the range 1..9 return input value as device ID in other case - return an error
  if( ( param == 0 ) || ( param > 9 ) ) {
    return INVALID_DEVICE_ID;
  }
  return param;
}

void close_device( device_id id )
{
  id = id;                                  // no compilation warning anymore
}

int device_write( device_id id, uint8_t page, uint8_t address, void *data_ptr, size_t data_len )
{
  uint8_t *data = static_cast< uint8_t* > ( data_ptr );
  char buffer[512] = { 0 };

  int pos = snprintf( buffer, sizeof( buffer ),
                      "device[%2X]\tpage[%2X]\taddress[%2X]\twrite[",
                      id, page, address );
  if( pos <= 0 )
      return -1;

  for( size_t i = 0; i < data_len; ++i ) {
    uint8_t byte = data[i];
    buffer[pos] = symbols[ ( byte & 0xf0 ) >> 4 ];
    ++pos;
    buffer[pos] = symbols[ ( byte & 0xf ) ];
    ++pos;
  }
  buffer[pos] = ']';

  printf( "%s\n", buffer );

  return static_cast< int >( data_len );
}

int device_read( device_id id, uint8_t page, uint8_t address, void *result_ptr, size_t result_len ) {
    uint8_t *result_value = static_cast< uint8_t* > ( result_ptr );
    *result_value = 42;

    printf( "device[%2X]\tpage[%2X]\taddress[%2X]\tread [2A]\n",
            id, page, address );

    return static_cast< int > ( result_len );
}
