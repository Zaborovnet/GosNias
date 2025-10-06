#include "RestMessageSender.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <random>
#include <iostream>
#include <memory>

using json = nlohmann::json;

RestMessageSender& RestMessageSender::getInstance(size_t max_queue_size_uptr)
{
  // Meyer's Singleton - потокобезопасный в C++11+
  static RestMessageSender instance(max_queue_size_uptr);
  return instance;
}

RestMessageSender::RestMessageSender(size_t max_queue_size_uptr)
    : _max_queue_size_uptr(max_queue_size_uptr),
      _stop_flag_atomic(false)
{
  // Запускаем фоновый поток для обработки очереди
  _worker_thread = std::thread(&RestMessageSender::processQueue, this);
}

RestMessageSender::~RestMessageSender()
{
  stop();
}

void RestMessageSender::stop()
{
  // Если уже остановлен, ничего не делаем
  if (_stop_flag_atomic.exchange(true)) {
      return;
    }

  // Будим worker thread для завершения
  _cond_var.notify_all();

  // Дожидаемся завершения потока
  if (_worker_thread.joinable()) {
      _worker_thread.join();
    }
}

void RestMessageSender::post(Message msg_src, std::vector<uint8_t> image_vec)
{
  std::lock_guard<std::mutex> lock(_mutex_mtx);

  // Проверяем не остановлен ли уже sender
  if (_stop_flag_atomic.load()) {
      std::cerr << "RestMessageSender is stopped, message rejected" << std::endl;
      return;
    }

  // Если очередь заполнена - удаляем самое старое сообщение
  if (_queue_deq.size() >= _max_queue_size_uptr) {
      _queue_deq.pop_front();
      std::cout << "Queue full, oldest message dropped" << std::endl;
    }

  // Добавляем новое сообщение в конец очереди
  _queue_deq.emplace_back(std::move(msg_src), std::move(image_vec));
  _cond_var.notify_one();  // Будим worker thread
}

void RestMessageSender::processQueue()
{
  while (!_stop_flag_atomic.load()) {
      std::pair<Message, std::vector<uint8_t>> task_pair;

      {
        std::unique_lock<std::mutex> lock(_mutex_mtx);

        // Ждем пока не появится задача или не придет сигнал остановки
        _cond_var.wait(lock, [this]() {
          return _stop_flag_atomic.load() || !_queue_deq.empty();
        });

        // Если остановка и очередь пуста - выходим
        if (_stop_flag_atomic.load() && _queue_deq.empty()) {
            break;
          }

        // Если есть задачи - берем первую
        if (!_queue_deq.empty()) {
            task_pair = std::move(_queue_deq.front());
            _queue_deq.pop_front();
          } else {
            continue;
          }
      }

      // Отправляем HTTP запрос (вне блокировки мьютекса)
      try {
          sendRequest(task_pair.first, task_pair.second);
        } catch (const std::exception& e) {
          std::cerr << "Error sending request: " << e.what() << std::endl;
        }
    }

  std::cout << "RestMessageSender worker thread stopped" << std::endl;
}

void RestMessageSender::sendRequest(const Message& msg_src, const std::vector<uint8_t>& image_vec) const
{
  // Создаем HTTP клиент (хост и порт нужно настраивать под ваш сервер)
  httplib::Client cli("http://localhost", 8080);
  cli.set_connection_timeout(5);  // 5 секунд таймаут
  cli.set_read_timeout(5);

  // Сериализуем структуру Message в JSON
  json j_json;
  j_json["shot_lat"] = msg_src.shot_lat_d;
  j_json["shot_lon"] = msg_src.shot_lon_d;
  j_json["objects"] = json::array();

  // Добавляем все объекты в JSON массив
  for (const auto& obj : msg_src.objects_vec) {
      j_json["objects"].push_back({
          {"x_c", obj.x_c_f},
          {"y_c", obj.y_c_f},
          {"width", obj.width_f},
          {"height", obj.height_f},
          {"label", obj.label_str}
      });
    }

  // Создаем multipart/form-data запрос
  httplib::MultipartFormDataItems items_vec;

  // Элемент для изображения
  httplib::MultipartFormData image_part;
  image_part.name = "image";
  image_part.content = reinterpret_cast<const char*>(image_vec.data());
  image_part.filename = "image.jpg";
  image_part.content_type = "image/jpeg";
  items_vec.push_back(image_part);

  // Элемент для JSON-данных
  httplib::MultipartFormData json_part;
  json_part.name = "payload_json";
  json_part.content = j_json.dump();
  json_part.filename = "";
  json_part.content_type = "application/json";
  items_vec.push_back(json_part);
}
