/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 00:06:13
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-28 17:32:43
 */

#pragma once

namespace serialize {

enum ser_config {
  DEFAULT = 0,
  DISABLE_TYPE_INFO = 0b1,
  ENABLE_TYPE_INFO = 0b10,
  DISABLE_ALL_META_INFO = 0b11
};

}