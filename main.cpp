#include "RestMessageSender.h"
#include <random>
#include <thread>
#include <chrono>
#include <vector>
#include <iostream>

/**
 * @brief Генерирует случайное изображение для тестирования
 * @param size_bytes_uptr Размер изображения в байтах
 * @return Вектор с случайными байтами
 */
std::vector<uint8_t> generateRandomImage(size_t size_bytes_uptr = 1024 * 1024)
{
  std::vector<uint8_t> image_vec(size_bytes_uptr);
  std::random_device rd_dev;
  std::mt19937 generator_mt(rd_dev());
  std::uniform_int_distribution<uint8_t> distribution(0, 255);

  for (auto& byte_ref : image_vec) {
      byte_ref = distribution(generator_mt);
    }

  return image_vec;
}

/**
 * @brief Создает тестовое сообщение
 * @return Тестовое сообщение
 */
Message createTestMessage()
{
  Message msg_test;
  msg_test.shot_lat_d = 55.751244;
  msg_test.shot_lon_d = 37.618423;

  // Добавляем объекты как в примере из задания
  msg_test.objects_vec = {
    {0.5f, 0.4f, 0.05f, 0.1f, "person"},
    {0.3f, 0.2f, 0.15f, 0.3f, "car"},
    {0.7f, 0.8f, 0.1f, 0.08f, "bicycle"}
  };

  return msg_test;
}

int main()
{
  try {
      // Получаем экземпляр синглтона с размером очереди 100 сообщений
      auto& sender_ref = RestMessageSender::getInstance(100);

      std::cout << "Starting REST message sender test..." << std::endl;

      // Отправляем несколько тестовых сообщений
      for (int i = 0; i < 5; ++i) {
          auto msg_test = createTestMessage();
          auto image_vec = generateRandomImage(512 * 1024);  // 512KB

          sender_ref.post(std::move(msg_test), std::move(image_vec));
          std::cout << "Posted message " << (i + 1) << std::endl;

          // Небольшая задержка между сообщениями
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

      // Даем время на отправку всех сообщений
      std::cout << "Waiting for messages to be sent..." << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(3));

      // Останавливаем sender перед выходом
      sender_ref.stop();
      std::cout << "Test completed successfully" << std::endl;

    } catch (const std::exception& e) {
      std::cerr << "Error in main: " << e.what() << std::endl;
      return 1;
    }

  return 0;
}
