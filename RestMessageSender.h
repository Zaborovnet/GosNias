#ifndef REST_MESSAGE_SENDER_H
#define REST_MESSAGE_SENDER_H

#include <atomic>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>
#include "MessageStructures.h"

/**
 * @brief Класс для неблокирующей отправки REST сообщений
 *
 * Реализует потокобезопасную очередь сообщений ограниченного размера
 * с фоновой отправкой через cpp-httplib
 */
class RestMessageSender
{
public:
  /**
   * @brief Получить экземпляр синглтона
   * @param max_queue_size_uptr Максимальный размер очереди
   * @return Ссылка на единственный экземпляр
   */
  static RestMessageSender& getInstance(size_t max_queue_size_uptr = 1000);

  /**
   * @brief Добавить сообщение в очередь для отправки
   * @param msg_src Сообщение для отправки
   * @param image_vec Вектор байтов изображения
   */
  void post(Message msg_src, std::vector<uint8_t> image_vec);

  // Запрещаем копирование и присваивание
  RestMessageSender(const RestMessageSender&) = delete;
  RestMessageSender& operator=(const RestMessageSender&) = delete;

  /**
   * @brief Остановить фоновый поток
   */
  void stop();

private:
  /**
   * @brief Приватный конструктор
   * @param max_queue_size_uptr Максимальный размер очереди
   */
  explicit RestMessageSender(size_t max_queue_size_uptr);

  /**
   * @brief Деструктор
   */
  ~RestMessageSender();

  /**
   * @brief Фоновая обработка очереди сообщений
   */
  void processQueue();

  /**
   * @brief Отправить HTTP запрос
   * @param msg_src Сообщение для отправки
   * @param image_vec Вектор байтов изображения
   */
  void sendRequest(const Message& msg_src, const std::vector<uint8_t>& image_vec) const;

  // Приватные поля
  std::deque<std::pair<Message, std::vector<uint8_t>>> _queue_deq;
  mutable std::mutex _mutex_mtx;
  std::condition_variable _cond_var;
  std::thread _worker_thread;
  const size_t _max_queue_size_uptr;
  std::atomic<bool> _stop_flag_atomic;
};

#endif // REST_MESSAGE_SENDER_H
