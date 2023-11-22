#! /bin/sh
### BEGIN INIT INFO
# Provides:          ps3netsrv
# Required-Start:    networking
# Required-Stop:     networking
# Default-Start:     2 3 5
# Default-Stop:      0 1 6
# Short-Description: Start PS3NetSrv.
# Description:       Autostart PlayStation 3 Sharing Service.
### END INIT INFO
#

# Name of the demon (Название демона)
name="ps3netsrv"
# The path to the demon (Путь до демона)
dir="/opt/ps3netsrv"
# Launch options (Параметры для запуска)
arg="/mnt/Playstation 4500 192.168.*.*"
# Search PID (Поиск PID)
pid="$(pgrep ps3netsrv)"
# User (Пользователь)
user="ps3netsrv"

# Command start (Команда start)
start()
{
    # от имени пользователя ps3netsrv запуск ps3netsrv
    su $user -c "$dir/$name $arg"
    echo "$name start"
}
# Command stop (Команда stop)
stop()
{
    # убиваем ps3netsrv
    su $user -c "kill $pid"
    echo "$name stop"
}
# Restart command :: Stops, waits 1 second, starts the daemon again (Команда restart :: Останавливает, ждёт 1 сек, снова запускает демона)
restart()
{
    stop
    sleep 1
    start
}
# Launch conditions (Условия запуска функций)
case "$1" in
    start)
        start
    ;;
    stop)
        stop
    ;;
    reload|restart)
        restart
    ;;
    *)
# Output usage help (Вывод help по скрипту)
        echo "Usage: {start|stop|restart}"
        exit 1
    ;;
esac

exit 0