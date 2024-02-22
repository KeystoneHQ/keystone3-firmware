#ifndef _DEFINE_H
#define _DEFINE_H

#define __I                             volatile const
#define __O                             volatile
#define __IO                            volatile

#define BUFFER_SIZE_32                  (32)
#define BUFFER_SIZE_64                  (64)
#define BUFFER_SIZE_128                 (128)
#define BUFFER_SIZE_256                 (256)

#ifndef UNUSED
#define UNUSED(X) (void)X
#endif

#define GET_STRING(X)                   #X
#define NUMBER_OF_ARRAYS(arr)           (sizeof(arr)/sizeof((arr)[0]))
#define FIELD_POS(type, field)          ((uint32_t) & ((type *)0)->field)
#define FIELD_SIZE(type, field )        (sizeof(((type *)0)->field))
#define LSB_TO_SHORT(array)             ((((uint16_t)(array)[0]) * 256) + (array)[1])
#define MSB_TO_SHORT(array)             ((((uint16_t)(array)[1]) * 256) + (array)[0])
#define LSB_TO_BYTE(array, val)         (array[0] = (val) / 256;array[1] = (val) % 256)
#define MSB_TO_BYTE(array, val)         (array[1] = (val) / 256;array[0] = (val) % 256)

#define GET_MULTIPLE_NUM(x, y)       ((((x) + (y - 1)) / y ) * y)


#endif /* _DEFINE_H */

