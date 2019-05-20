# Lab_3
## Цель
* Познакомиться с классами для сетевого взаимодействия в Qt.
* Познакомиться с основами протоколов TCP\IP.
* Разработать небольшое сетевое приложение, демонстрирующее работу сервера и клиента.

## Требование к работе
Необходимый набор условий для сдачи лабораторной работы:
- [ ] Компилируется?
- [ ] Приложение не падает с критической ошибкой в процессе его работы?
- [ ] Класс сервера содержит объект класса QTcpServer?
- [ ] Класс клиента содержит объект класса QTcpSocket?
- [ ] В процессе работы данные передаются в две стороны: от сервера к клиенту и от клиента к серверу?

## Важное замечание
Любые дополнения к заданию приветствуются. Для примера: к сетевому взаимодействию можно добавить работу с OpenGL. 

## Задание
Создать серверное приложение, которое прослушивает определенный TCP порт и обладает следующими свойствами:
1) При новом подключении, сервер добавляет указатель на объект класса соответствующего сокета во внутренний список для последующей работы с этим сокетом.
2) Сервер хранит 50 последних сообщений от всех пользователей в своей памяти.
3) Сервер пересылает 50 последних сообщений каждому новому пользователю.
4) При получении нового сообщения от пользователя, сервер отправляет это сообщение всем текущим пользователям.
5) При отключении пользователя от сервера, сервер должен удалить соответствующий указатель из своего внутреннего списка активных пользователей.

Создать приложение-клиент, которое имеет возможность подключаться к заданному TCP порту и обладает следующими свойствами:
1) Имеет поля для ввода адреса и порта сервера.
2) Имеет поле для отображения истории сообщений.
3) Имеет поле для ввода нового сообщения.
4) Имеет кнопки для подключения к серверу, отключения от сервера и отправки текущего сообщения.
5) Кнопки активны только в случае, когда поля адреса и порта сервера не пустые.
