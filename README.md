# IP-Guard

Легковесный и быстрый сервис на C++ для мониторинга атак на сервер и мгновенного уведомления в Telegram бот. Интегрируется с Fail2ban внутри Docker-контейнера.

Итак. Что делает этот код?

1. Принимает данные от Fail2ban: Получает IP-адрес злоумышленника и название «тюрьмы» (jail).
2. Обрабатывает событие: С помощью библиотеки libcurl отправляет POST-запрос к Telegram Bot API.
3. Уведомляет владельца: Вы получаете сообщение с IP-адресом, временем бана и типом атаки (например, SSH brute-force).

   Требования:
1. ОС: Linux (протестировано на Raspberry Pi / Debian).
2. Компилятор: g++.

   Библиотеки:
1. libcurl.

    Инструменты:
1. Docker & Docker Compose.

    Установка и компиляция:
1. Установите зависимости:
   //sudo apt update && sudo apt install libcurl4-openssl-dev g++
2. Скомпилируйте проект:
   //g++ -O3 ip_guard.cpp -lcurl -o ip_guard

   Интеграция в Docker (Fail2ban):

Для стабильной работы в Docker-контейнере используйте следующую структуру:

1. Настройка Dockerfile
Используйте debian-slim для минимизации размера образа:

   Dockerfile
/*
FROM debian:bookworm-slim
RUN apt update && apt install -y fail2ban iptables libcurl4 curl ca-certificates && \
    rm -rf /var/lib/apt/lists/*
RUN mkdir -p /var/run/fail2ban /var/lib/fail2ban
*/


# Важно: запускаем с выводом логов в консоль

CMD ["fail2ban-server", "-f", "-x", "--logtarget=STDOUT", "start"]

2. Настройка docker-compose.yml
Критически важно: пробросьте часовой пояс, чтобы Fail2ban корректно читал логи хоста.

//YAML

/*
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
      - '/var/log:/var/log:ro'
      - './fail2ban/data/jail.d/local.conf:/etc/fail2ban/jail.d/local.conf'
      - './fail2ban/data/action.d/telegram.conf:/etc/fail2ban/action.d/telegram.conf'
      - '/путь/к/вашему/проекту:/ip_guard_bin:ro'
    restart: unless-stopped

 */   

3. Конфигурация Fail2ban (action.d/telegram.conf)
Создайте файл действия, который будет вызывать ваш C++ бинарник:

//Ini, TOML

[Definition]
actionban = /ip_guard_bin/ip_guard <ip> <name>
actionunban = 



⚠️ Важные нюансы (Troubleshooting)
Часовой пояс: Если время в контейнере и на хосте отличается, Fail2ban проигнорирует записи в логах. Всегда синхронизируйте /etc/localtime.

Локаль (Locale): Если системные логи пишутся на языке, отличном от английского (например, "дек" вместо "Dec"), Fail2ban может не распознать дату.

Права доступа: Убедитесь, что бинарник имеет права на выполнение внутри контейнера (chmod +x ip_guard).


📝 Лицензия
MIT License. Используйте во благо безопасности ваших серверов!
