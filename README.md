# 🛡️ IP-Guard

**IP-Guard** — это легковесный и высокопроизводительный сервис на **C++**, предназначенный для мониторинга атак на сервер и мгновенного уведомления владельца через **Telegram**. Проект оптимизирован для работы в связке с **Fail2ban** внутри Docker-контейнера.

---

## 🚀 Основные функции

* **Интеграция с Fail2ban**: Получает IP-адрес злоумышленника и название "тюрьмы" (jail) в реальном времени.
* **Высокая скорость**: Благодаря C++ и `libcurl`, отправка уведомления происходит практически мгновенно с минимальным потреблением ресурсов.
* **Информативность**: Уведомления включают IP, время бана и тип сервиса, подвергшегося атаке (например, SSH brute-force).

---

## 🛠 Требования и зависимости

| Категория | Инструменты |
| :--- | :--- |
| **ОС** | Linux (протестировано на Raspberry Pi / Debian) |
| **Стек** | C++, Docker, Docker Compose |
| **Библиотеки** | `libcurl` |
| **Компилятор** | `g++` |

---

## 📦 Установка и компиляция

1.  **Установите необходимые пакеты:**
    ```bash
    sudo apt update && sudo apt install libcurl4-openssl-dev g++
    ```

2.  **Скомпилируйте проект:**
    ```bash
    g++ -O3 ip_guard.cpp -lcurl -o ip_guard
    ```

---

## 🐳 Интеграция в Docker (Fail2ban)

Для стабильной работы Fail2ban внутри Docker используйте следующую конфигурацию:

### 1. Dockerfile
Рекомендуется использовать образ `debian-slim` для минимизации размера:

```dockerfile
FROM debian:bookworm-slim

RUN apt update && apt install -y fail2ban iptables libcurl4 curl ca-certificates && \
    rm -rf /var/lib/apt/lists/*

RUN mkdir -p /var/run/fail2ban /var/lib/fail2ban

# Запуск с выводом логов в STDOUT для мониторинга через 'docker logs'
CMD ["fail2ban-server", "-f", "-x", "--logtarget=STDOUT", "start"]



2. docker-compose.yml
Важно: Обязательно синхронизируйте часовой пояс, иначе Fail2ban может игнорировать логи хоста.

services:
  fail2ban:
    container_name: fail2ban
    build: ./fail2ban
    network_mode: "host"
    cap_add:
      - NET_ADMIN
      - NET_RAW
    environment:
      - TZ=Europe/Kyiv # Укажите ваш часовой пояс
    volumes:
      - '/etc/localtime:/etc/localtime:ro'
      - '/etc/timezone:/etc/timezone:ro'
      - '/var/log:/var/log:ro'
      - './fail2ban/data/jail.d/local.conf:/etc/fail2ban/jail.d/local.conf'
      - './fail2ban/data/action.d/telegram.conf:/etc/fail2ban/action.d/telegram.conf'
      - '/path/to/your/project:/ip_guard_bin:ro'
    restart: unless-stopped



3. Конфигурация действия (action.d/telegram.conf)
Создайте файл, который будет вызывать скомпилированный бинарник:

[Definition]
actionban = /ip_guard_bin/ip_guard <ip> <name>
actionunban =



Права доступа: Убедитесь, что бинарник имеет права на выполнение: chmod +x ip_guard.


📝 Лицензия
Распространяется под лицензией MIT. Используйте во благо безопасности ваших серверов!
