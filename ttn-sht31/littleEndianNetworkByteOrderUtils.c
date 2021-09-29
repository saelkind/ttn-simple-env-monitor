/**
 *  Utilities for converting short and long ints
 *  on a little-endian platform to/from network byte order
 *  (which is big-endian).
 *
 *  Using non-standard names for these, to avoid conflicts if
 *  the proper libraries are also being used.
 */


// for little-endian architectures, like ESP32
short htons_le(short num) {
  return (num<< 8 & 0xFF00) | (num>> 8 & 0x00FF);
};

short ntohs_le(short num) {
  return htons_le(num);
}

long htonl_le(short num) {
  return  (num<<24 & 0xFF000000UL) | 
             (num<<8 & 0x00FF0000UL) | 
             (num>>8 & 0x0000FF00UL) |
             (num>>24 & 0x000000FFUL);
}

long ntohl_le(short num) {
  return htonl_le(num);
}