/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTIL_H
#define UTIL_H

/* Decodes an int32 encoded in big endian */
template<typename T>
inline uint32_t toBEInt32(T data) {
  return ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
}

template<typename T>
inline void writeBEInt32(uint32_t i, T data) {
  data[0] = (i & 0xFF000000) >> 24;
  data[1] = (i & 0xFF0000) >> 16;
  data[2] = (i & 0xFF00) >> 8;
  data[3] = i & 0xFF;
}

/* Decodes an int32 encoded in little endian */
template<typename T>
inline uint32_t toLEInt32(T data) {
  return (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

/* Decodes a float encoded in big endian */
template<typename T>
inline float toBEFloat(T data) {
  uint32_t raw = toBEInt32(data);
  return *(float *)(&raw);
}

/* Decodes a float encoded in little endian */
template<typename T>
inline float toLEFloat(T data) {
  uint32_t raw = toLEInt32(data);
  return *(float *)(&raw);
}

#endif
