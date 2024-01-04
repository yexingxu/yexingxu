/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-12 18:23:12
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-12 18:23:28
 */

#pragma once

#define SER_LIKELY(expr) (__builtin_expect(!!(expr), 1))
#define SER_UNLIKELY(expr) (__builtin_expect(!!(expr), 0))
