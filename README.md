# DSViz

## Установка

Чтобы склонировать репозиторий из ветки ```dev``` нужно выполнить

```git clone -b dev --recurse-submodules git@github.com:rgolovachev/DSViz.git```

После чего появится папка с проектом, который можно запустить, открыв в qt creator или вручную вызвав qmake:

```
mkdir build
cd build
qmake ../DSViz/DSViz.pro
make
```

Ну и дальше открыть получившееся приложение

## Интерфейс

Интерфейс в целом думаю интуитивно понятен. Единственное что может вызвать вопросы это checkbox который называется Animation off. Он отключает пошаговую визуализацию происходящего с деревом. Это нужно для того, чтобы накидать по-быстрому в дерево побольше вершин, а потом уже включить пошаговую анимацию и внимательно смотреть, что происходит с деревом. 

Также во время пошаговой визуализации загорается кнопка Pause/Continue, нажимая на которую можно останавливать/продолжать исполнение операции над деревом.

Двигать полотно нужно с помощью ЛКМ, приближать с помощью ползунка слева (возможность приближать колесиком я не добавил т.к. у меня мак)

![На экране должно происходить что-то вот такое](/img/pic.png)



