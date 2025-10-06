#ifndef MESSAGE_STRUCTURES_H
#define MESSAGE_STRUCTURES_H

#include <string>
#include <vector>

/**
 * @brief Структура для представления объекта детектирования
 */
struct Object
{
  float x_c_f;      ///< центр бокса по X (нормированный)
  float y_c_f;      ///< центр по Y (нормированный)
  float width_f;    ///< ширина (нормированная)
  float height_f;   ///< высота (нормированная)
  std::string label_str;  ///< класс объекта
};

/**
 * @brief Структура для представления сообщения
 */
struct Message
{
  double shot_lat_d;   ///< широта места съёмки
  double shot_lon_d;   ///< долгота места съёмки
  std::vector<Object> objects_vec;  ///< вектор обнаруженных объектов
};

#endif // MESSAGE_STRUCTURES_H
