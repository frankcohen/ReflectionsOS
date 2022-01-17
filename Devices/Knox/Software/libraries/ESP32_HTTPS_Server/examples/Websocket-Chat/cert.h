#ifndef CERT_H_
#define CERT_H_
unsigned char example_crt_DER[] = {
  0x30, 0x82, 0x02, 0x18, 0x30, 0x82, 0x01, 0x81, 0x02, 0x01, 0x02, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05,
  0x05, 0x00, 0x30, 0x54, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
  0x06, 0x13, 0x02, 0x44, 0x45, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x04, 0x08, 0x0c, 0x02, 0x42, 0x45, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x03,
  0x55, 0x04, 0x07, 0x0c, 0x06, 0x42, 0x65, 0x72, 0x6c, 0x69, 0x6e, 0x31,
  0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x09, 0x4d, 0x79,
  0x43, 0x6f, 0x6d, 0x70, 0x61, 0x6e, 0x79, 0x31, 0x13, 0x30, 0x11, 0x06,
  0x03, 0x55, 0x04, 0x03, 0x0c, 0x0a, 0x6d, 0x79, 0x63, 0x61, 0x2e, 0x6c,
  0x6f, 0x63, 0x61, 0x6c, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x31, 0x30, 0x35,
  0x33, 0x30, 0x30, 0x31, 0x31, 0x39, 0x31, 0x39, 0x5a, 0x17, 0x0d, 0x33,
  0x31, 0x30, 0x35, 0x32, 0x38, 0x30, 0x31, 0x31, 0x39, 0x31, 0x39, 0x5a,
  0x30, 0x55, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
  0x02, 0x44, 0x45, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08,
  0x0c, 0x02, 0x42, 0x45, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x03, 0x55, 0x04,
  0x07, 0x0c, 0x06, 0x42, 0x65, 0x72, 0x6c, 0x69, 0x6e, 0x31, 0x12, 0x30,
  0x10, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x09, 0x4d, 0x79, 0x43, 0x6f,
  0x6d, 0x70, 0x61, 0x6e, 0x79, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55,
  0x04, 0x03, 0x0c, 0x0b, 0x65, 0x73, 0x70, 0x33, 0x32, 0x2e, 0x6c, 0x6f,
  0x63, 0x61, 0x6c, 0x30, 0x81, 0x9f, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86,
  0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 0x8d,
  0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xb1, 0xac, 0x21, 0xa6,
  0xc1, 0xcd, 0x37, 0xe7, 0x86, 0xe0, 0x87, 0x53, 0xf4, 0xf7, 0xb3, 0xff,
  0x40, 0xc8, 0xe8, 0x29, 0x69, 0x23, 0x20, 0x59, 0x2d, 0x19, 0xf2, 0xfe,
  0xb9, 0xc7, 0xd3, 0xff, 0xf9, 0x8c, 0xdc, 0x2f, 0xad, 0xce, 0x71, 0x0a,
  0x51, 0x25, 0x9f, 0x01, 0x57, 0xcf, 0x79, 0xfe, 0xe0, 0xd8, 0xab, 0x0b,
  0x2a, 0xd9, 0x9b, 0x9c, 0x05, 0x63, 0x47, 0x06, 0xa9, 0x6a, 0x00, 0x47,
  0x40, 0x24, 0x1f, 0x07, 0x10, 0x80, 0xed, 0x61, 0x82, 0x33, 0x7d, 0xbc,
  0xa3, 0x8c, 0x2b, 0xb6, 0x53, 0x15, 0xa0, 0x5f, 0x8c, 0x07, 0xd8, 0xfb,
  0x17, 0x80, 0x17, 0x44, 0x94, 0x5b, 0xd8, 0xab, 0x07, 0x0a, 0xba, 0xe7,
  0x8d, 0x23, 0x03, 0xf9, 0xbc, 0xd0, 0xc4, 0x67, 0xc0, 0xbf, 0x8e, 0xfc,
  0x17, 0x45, 0xea, 0xac, 0xb7, 0x90, 0x3b, 0x47, 0xca, 0xa8, 0x14, 0xf9,
  0x20, 0xce, 0xc3, 0xa3, 0x02, 0x03, 0x01, 0x00, 0x01, 0x30, 0x0d, 0x06,
  0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00,
  0x03, 0x81, 0x81, 0x00, 0x48, 0x8a, 0xff, 0x76, 0xf7, 0x9a, 0x63, 0x3d,
  0xba, 0x19, 0x6b, 0x25, 0xbb, 0x91, 0xd4, 0xa8, 0x9f, 0xec, 0xb6, 0xba,
  0xbe, 0x12, 0xc7, 0x38, 0xd0, 0x55, 0x42, 0xf1, 0xb8, 0x00, 0xdb, 0xc6,
  0x26, 0x3c, 0xd6, 0x09, 0xd2, 0xc5, 0xe9, 0x46, 0xfb, 0xff, 0xf1, 0xb7,
  0xeb, 0x84, 0xd0, 0x3c, 0xa3, 0x5d, 0x4a, 0x61, 0xdd, 0x6f, 0xb3, 0x9a,
  0x3a, 0xe6, 0x16, 0x17, 0xe8, 0xfa, 0xc9, 0xbd, 0x9d, 0x93, 0x34, 0xaf,
  0x0e, 0x5f, 0xcf, 0x61, 0x3c, 0xe1, 0x3f, 0xc0, 0x21, 0x9d, 0x5c, 0xd8,
  0xee, 0x35, 0xd2, 0x4f, 0x3f, 0xde, 0x35, 0x6e, 0xc0, 0x1f, 0x05, 0x76,
  0x27, 0x30, 0x6e, 0x31, 0xe2, 0x4c, 0xa0, 0xcc, 0x23, 0x54, 0xe2, 0xce,
  0x3f, 0x91, 0x4c, 0x99, 0xc0, 0x93, 0x54, 0xc3, 0x33, 0x4f, 0x9e, 0x46,
  0x34, 0xcd, 0xc5, 0xb1, 0x48, 0x8e, 0x78, 0xff, 0xdb, 0x0a, 0x34, 0xcb
};
unsigned int example_crt_DER_len = 540;
#endif