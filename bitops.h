#ifndef BITOPS_H
#define BITOPS_H

/* 移位 */
#define _BIT(bit)                   (1u << (bit))

/* 位置位 */
#define _BIT_SET(data, bit)         ((data) |= _BIT(bit))

/* 位清零 */
#define _BIT_CLR(data, bit)         ((data) &= ~_BIT(bit))

/* 位翻转 */
#define _BIT_TOGGLE(data, bit)      ((data) ^= _BIT(bit))

/* 位修改 */
#define _BIT_MODIFY(data, bit, value) \
    ((value) ? _BIT_SET(data, bit) : _BIT_CLR(data, bit))

/* 获取位值 */
#define _BIT_GET(data, bit)         (_BIT_ISSET(data, bit) ? 1 : 0)

/* 获取位段值 */
#define _BITS_GET(data, start, len)  \
    (((data) >> (start)) & _BITS_MASK(len))

/* 设置位段值 */
#define _BITS_SET(data, start, len, value) \
    ((data) = (((data) & ~_SBF(_BITS_MASK(len), (start))) | \
               _SBF((value) & (_BITS_MASK(len)), (start))))

/* 计算 n 位掩码值 */
#define _BIT_MASK(nr)              (1UL << ((nr) % 32))

/************************************************************************/

#define _BIT_SET_MASK(data, mask)   ((data) |= (mask))

#define _BIT_CLR_MASK(data, mask)   ((data) &= ~(mask))

#define _BIT_ISSET(data, bit)       ((data) & _BIT(bit))

/* 获取n位掩码值 */
#define _BITS_MASK(n)               (~((~0u) << (n)))

/* 值移位 */
#define _SBF(value, field)         ((value) << (field))

#endif
