# Лабораторный практикум по операционным системам #

## 2 лаба ##

Вариант 5. Межпроцессное взаимодействие:

Тематическая пошаговая игра **Волк и семеро козлят**. Хост - волк, клиент(ы) - козлята, n- количество козлят. На каждом ходу волк выбрасывает число от 1 до 100 (у пользователя есть возможность вводить число в приложении самостоятельно; если на протяжении 3 сек. число не было введено, оно выбирается случайно), живые козлята выбрасывают число от 1 до 100 случайным образом, а мертвые - число от 1 до 50. Если число живого козленка отличается от числа волка не более, чем на $\frac{70}{n}$, то считается, что он спрятался. Козлята, которые не спрятались, попадаются и считаются мертвыми. Если число, полученное мертвым козленком, отличается не более, чем на $\frac{20}{n}$ - он воскресает. До подключения козлят ничего не происходит, игра заканчивается, если в течение 2 ходов подряд все козлята мертвы. За логику отвечает хост, прогресс игры отображается в его выводе в консоль, клиенты шлют ему числа козлят, а в ответ получают статус козленка (жив, мертв). В каждом раунде нужно выводить числа, выбрасываемые волком и козлятами и число спрятавшихся, попавшихся и мертвых козлят.

Конфигурация: Независимые, один к одному

Типы взаимодействия: 

- Общая память (shmget). TYPE_CODE - seg
- Очереди сообщений (mq_open). TYPE_CODE - mq
- Именованные каналы (mkfifo). TYPE_CODE - fifo

(2, 4, 6)

Баллы: 15

Дедлайн: 25 ноября 23:59