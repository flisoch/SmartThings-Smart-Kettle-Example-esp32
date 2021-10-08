# Алгоритм создания и обновления кастомного Capability

1. Скачайте smartthings-cli из https://github.com/SmartThingsCommunity/smartthings-cli/releases

2. (Опционально) Скопируйте конфигурационный json-файл наиболее близкого Capability, который можно взять за основу. Альтернатива - использовать шаблонные файлы из этого репозитория.

    `$./smartthings capabilities thermostatHeatingSetpoint 1 -j -o heatingCustom.json`
    
    `$./smartthings capabilities:presentation thermostatHeatingSetpoint 1 -j -o heatingCustomPresentation.json`

3. В файле heatingCustom.json удалите поля Id, версия, статус, установите новое имя и измените другие поля, которые хочется кастомизировать

4. Создайте новое кастомное Capability и сохраните результат создания в том же файле heatingCustom.json

    `$./smartthings capabilities:create -j -i heatingCustom.json -o heatingCustom.json`

5. Скопируйте идентификатор/id Capability из полученного файла в файл heatingCustomPresentation.json. Измените другие поля, которые хочется кастомизировать

6. Создайте новое UI представление для кастомного Capability и сохраните результат в customCapabilityPresentation.json

    `$./smartthings capabilities:presentation:create -j -i capabilityPresentationCopy.json -o customCapabilityPresentation.json`

7. Добавьте файл capability-helper в директорию iot-core/src/include/caps и замените в нем идентификатор/id на полученный для кастомного Capability
8. Включайте файл-помощник в файлы работы с Capability через директиву #include

    `#include "caps/iot_caps_helper_heatingSetpoint.h"`
