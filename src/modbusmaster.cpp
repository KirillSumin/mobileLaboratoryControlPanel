#include "modbusmaster.h"

ModbusMaster::ModbusMaster(QObject* parent)
    : QModbusRtuSerialMaster(parent)
{
    QModbusResponse::registerDataSizeCalculator(
        QModbusPdu::FunctionCode(0x34),
        [](const QModbusResponse&) -> int {
            return 5;
        });
    QModbusResponse::registerDataSizeCalculator(
        QModbusPdu::FunctionCode(0x35),
        [](const QModbusResponse&) -> int {
            return 4;
        });
}

void ModbusMaster::setModbusLabParameters(
    QString lab_port,
    QString lab_bauds,
    QString lab_stop_bits,
    QString lab_data_bits,
    QString lab_parity,
    QString lab_response_time,
    QString lab_modbus_net_addr)
{
    LAB_PORT = (QVariant)("COM" + lab_port); //потенциальная ошибка при получении "не цифры"
    LAB_BAUDS = (QVariant)lab_bauds.toInt(nullptr, DEC_BASE);
    LAB_STOP_BITS = (QVariant)lab_stop_bits.toInt(nullptr, DEC_BASE);
    LAB_DATA_BITS = (QVariant)lab_data_bits.toInt(nullptr, DEC_BASE);
    LAB_PARITY = (QVariant)qstr_to_parity_map.value(lab_parity, QSerialPort::NoParity);
    LAB_RESPONSE_TIME = lab_response_time.toInt(nullptr, DEC_BASE);
    LAB_MODBUS_NET_ADDRESS = lab_modbus_net_addr.toInt(nullptr, DEC_BASE);
    //общее время ожидания ответа от modbus qt класса (+500 для отработки неожиданной задержки)
    MODBUS_QT_LIBRARY_RESPONSE_TIME = LAB_RESPONSE_TIME * MODBUS_TRY_SEND_COUNT + 500;
}

ModbusMaster::ModbusError ModbusMaster::connectModbus()
{
    int counter { 0 };
    if (QModbusDevice::state() == QModbusDevice::ConnectedState) {
        QModbusDevice::disconnectDevice();
    }
    QModbusDevice::setConnectionParameter(
        QModbusDevice::SerialPortNameParameter, LAB_PORT);
    setConnectionParameter(SerialParityParameter, LAB_PARITY);
    setConnectionParameter(SerialBaudRateParameter, LAB_BAUDS);
    setConnectionParameter(SerialDataBitsParameter, LAB_DATA_BITS);
    setConnectionParameter(SerialStopBitsParameter, LAB_STOP_BITS);
    QModbusClient::setTimeout(LAB_RESPONSE_TIME);
    QModbusClient::setNumberOfRetries(MODBUS_TRY_SEND_COUNT - 1); //(отсчёт идёт с 0, поэтому -1)
    // qDebug() << "COM connect";
    while (!QModbusDevice::connectDevice()) {
        if (counter < RECONNECT_COM_COUNT - 1) {
            //  qDebug() << "COM connect";
            counter++;
        } else {
            return COM_CONNECTION_ERROR;
        }
    }
    return NO_ERROR;
}

QPair<int, ModbusMaster::ModbusError> ModbusMaster::readBit(int cur_bit_address)
{
    if (QModbusDevice::state() != ConnectedState) {
        if (connectModbus() != NO_ERROR) {
            //COM порт при вызове метода не был подключен
            //переподключиться к COM порту не получилось
            return qMakePair(UNDEFINED, COM_CONNECTION_ERROR);
        }
    }
    auto bit_data_unit = QModbusDataUnit(QModbusDataUnit::Coils, cur_bit_address, ModbusMaster::ONE_BIT_OR_REGISTER);
    auto* cur_reply = QModbusClient::sendReadRequest(bit_data_unit, ModbusMaster::LAB_MODBUS_NET_ADDRESS);
    if (cur_reply) {
        QSignalSpy spy(cur_reply, &QModbusReply::finished);
        bool got_answer = spy.wait(MODBUS_QT_LIBRARY_RESPONSE_TIME);
        if (got_answer && cur_reply->error() == QModbusDevice::NoError
            && cur_reply->result().isValid()) {
            //ответ был получен, ошибок нет, формат ответа верный
            return qMakePair((int)cur_reply->result().value(0), NO_ERROR);
        }
    }
    if (connectModbus() != NO_ERROR) {
        //скорее всего, произошло "горячее отключение" COM
        //переподключение не помогло
        return qMakePair(UNDEFINED, COM_CONNECTION_ERROR);
    }
    //повторная отправка запроса
    cur_reply = QModbusClient::sendReadRequest(bit_data_unit, ModbusMaster::LAB_MODBUS_NET_ADDRESS);
    if (cur_reply) {
        QSignalSpy spy(cur_reply, &QModbusReply::finished);
        bool got_answer = spy.wait(MODBUS_QT_LIBRARY_RESPONSE_TIME);
        if (got_answer) {
            if (cur_reply->error() == QModbusDevice::NoError
                && cur_reply->result().isValid()) {
                //ответ был получен, ошибок нет, формат ответа верный
                return qMakePair((int)cur_reply->result().value(0), NO_ERROR);
            }
            if (cur_reply->error() == QModbusDevice::TimeoutError) {
                //master смог отправить запрос в Modbus сеть
                //но slave с указанным адресом не откликнулся
                return qMakePair(ModbusMaster::UNDEFINED, NO_RESPONSE_FROM_MODBUS_SLAVE);
            }
            //причина ошибки не выявлена
            return qMakePair(ModbusMaster::UNDEFINED, UNKNOWN_MODBUS_ERROR);
        }
        //master смог отправить запрос в Modbus сеть
        //но slave с указанным адресом не откликнулся
        return qMakePair(ModbusMaster::UNDEFINED, NO_RESPONSE_FROM_MODBUS_SLAVE);
    }
    //причина ошибки не выявлена
    return qMakePair(ModbusMaster::UNDEFINED, UNKNOWN_MODBUS_ERROR);
}

QPair<QBitArray, ModbusMaster::ModbusError> ModbusMaster::readRegister(int cur_register_address)
{
    if (QModbusDevice::state() != ConnectedState) {
        if (connectModbus() != NO_ERROR) {
            //COM порт при вызове метода не был подключен
            //переподключиться к COM порту не получилось
            return qMakePair(QBitArray(), COM_CONNECTION_ERROR);
        }
    }
    auto register_data_unit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, cur_register_address, ModbusMaster::ONE_BIT_OR_REGISTER);
    auto* cur_reply = QModbusClient::sendReadRequest(register_data_unit, ModbusMaster::LAB_MODBUS_NET_ADDRESS);
    if (cur_reply) {
        QSignalSpy spy(cur_reply, &QModbusReply::finished);
        bool got_answer = spy.wait(MODBUS_QT_LIBRARY_RESPONSE_TIME);
        if (got_answer
            && cur_reply->error() == QModbusDevice::NoError
            && cur_reply->result().isValid()) {
            QVector<quint16> cur_modbus_data_vector = cur_reply->result().values();
            if (!cur_modbus_data_vector.isEmpty()) {
                //ответ был получен, ошибок нет, формат ответа верный
                quint16& cur_modbus_data = cur_modbus_data_vector.first();
                return qMakePair(decToQBitArray(cur_modbus_data), NO_ERROR);
            }
        }
    }
    if (connectModbus() != NO_ERROR) {
        //скорее всего, произошло "горячее отключение" COM
        //переподключение не помогло
        return qMakePair(QBitArray(), COM_CONNECTION_ERROR);
    }
    cur_reply = QModbusClient::sendReadRequest(register_data_unit, ModbusMaster::LAB_MODBUS_NET_ADDRESS);
    if (cur_reply) {
        QSignalSpy spy(cur_reply, &QModbusReply::finished);
        bool got_answer = spy.wait(MODBUS_QT_LIBRARY_RESPONSE_TIME);
        if (got_answer) {
            QVector<quint16> cur_modbus_data_vector = cur_reply->result().values();
            if (cur_reply->error() == QModbusDevice::NoError
                && cur_reply->result().isValid() && !cur_modbus_data_vector.isEmpty()) {
                //ответ был получен, ошибок нет, формат ответа верный
                quint16& cur_modbus_data = cur_modbus_data_vector.first();
                return qMakePair(decToQBitArray(cur_modbus_data), NO_ERROR);
            }
            if (cur_reply->error() == QModbusDevice::TimeoutError) {
                //master смог отправить запрос в Modbus сеть
                //но slave с указанным адресом не откликнулся
                return qMakePair(QBitArray(), NO_RESPONSE_FROM_MODBUS_SLAVE);
            }
            //причина ошибки не выявлена
            return qMakePair(ModbusMaster::UNDEFINED, UNKNOWN_MODBUS_ERROR);
        }
        //master смог отправить запрос в Modbus сеть
        //но slave с указанным адресом не откликнулся
        return qMakePair(QBitArray(), NO_RESPONSE_FROM_MODBUS_SLAVE);
    }
    //причина ошибки не выявлена
    return qMakePair(QBitArray(), UNKNOWN_MODBUS_ERROR);
}

ModbusMaster::ModbusError ModbusMaster::writeBit(int cur_bit_address, bool cur_bit_state)
{
    if (QModbusDevice::state() != ConnectedState) {
        if (connectModbus() != NO_ERROR) {
            //COM порт при вызове метода не был подключен
            //переподключиться к COM порту не получилось
            return COM_CONNECTION_ERROR;
        }
    }
    const QVector<quint16> cur_data_to_write(1, (quint16)cur_bit_state);
    auto bit_data_unit = QModbusDataUnit(QModbusDataUnit::Coils, cur_bit_address, cur_data_to_write);
    auto* cur_reply = QModbusClient::sendWriteRequest(bit_data_unit, ModbusMaster::LAB_MODBUS_NET_ADDRESS);
    if (cur_reply) {
        QSignalSpy spy(cur_reply, &QModbusReply::finished);
        bool got_answer = spy.wait(MODBUS_QT_LIBRARY_RESPONSE_TIME);
        if (got_answer && cur_reply->error() == QModbusDevice::NoError
            && cur_reply->result().isValid()) {
            //ответ был получен, ошибок нет, формат ответа верный
            return NO_ERROR;
        }
    }
    if (connectModbus() != NO_ERROR) {
        //скорее всего, произошло "горячее отключение" COM
        //переподключение не помогло
        return COM_CONNECTION_ERROR;
    }
    cur_reply = QModbusClient::sendWriteRequest(bit_data_unit, ModbusMaster::LAB_MODBUS_NET_ADDRESS);
    if (cur_reply) {
        QSignalSpy spy(cur_reply, &QModbusReply::finished);
        bool got_answer = spy.wait(MODBUS_QT_LIBRARY_RESPONSE_TIME);
        if (got_answer) {
            if (cur_reply->error() == QModbusDevice::NoError
                && cur_reply->result().isValid()) {
                //ответ был получен, ошибок нет, формат ответа верный
                return NO_ERROR;
            }
            if (cur_reply->error() == QModbusDevice::TimeoutError) {
                //master смог отправить запрос в Modbus сеть
                //но slave с указанным адресом не откликнулся
                return NO_RESPONSE_FROM_MODBUS_SLAVE;
            }
            //причина ошибки не выявлена
            return UNKNOWN_MODBUS_ERROR;
        }
        //master смог отправить запрос в Modbus сеть
        //но slave с указанным адресом не откликнулся
        return NO_RESPONSE_FROM_MODBUS_SLAVE;
    }
    //причина ошибки не выявлена
    return UNKNOWN_MODBUS_ERROR;
}

ModbusMaster::ModbusError ModbusMaster::writeRegister(int cur_register_address, QBitArray cur_bit_array_val)
{
    if (cur_bit_array_val.isNull()) {
        //нет ни одного элемента в массиве
        return INTERNAL_INVALID_MODBUS_INPUT_DATA;
    }
    if (cur_bit_array_val.size() != 16) {
        //регистр имеет постоянный размер в 16 бит
        return INTERNAL_INVALID_MODBUS_INPUT_DATA;
    }
    if (QModbusDevice::state() != ConnectedState) {
        if (connectModbus() != NO_ERROR) {
            //COM порт при вызове метода не был подключен
            //переподключиться к COM порту не получилось
            return COM_CONNECTION_ERROR;
        }
    }
    //qDebug() << cur_bit_array_val; //для дебага
    const QVector<quint16> cur_data_to_write(1, QBitArrayToDec(cur_bit_array_val));
    auto register_data_unit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, cur_register_address, cur_data_to_write);
    auto* cur_reply = QModbusClient::sendWriteRequest(register_data_unit, ModbusMaster::LAB_MODBUS_NET_ADDRESS);
    if (cur_reply) {
        QSignalSpy spy(cur_reply, &QModbusReply::finished);
        bool got_answer = spy.wait(MODBUS_QT_LIBRARY_RESPONSE_TIME);
        if (got_answer && cur_reply->error() == NoError && cur_reply->result().isValid()) {
            //ответ был получен, ошибок нет, формат ответа верный
            return NO_ERROR;
        }
    }
    if (connectModbus() != NO_ERROR) {
        //скорее всего, произошло "горячее отключение" COM
        //переподключение не помогло
        return COM_CONNECTION_ERROR;
    }
    cur_reply = QModbusClient::sendWriteRequest(register_data_unit, ModbusMaster::LAB_MODBUS_NET_ADDRESS);
    if (cur_reply) {
        QSignalSpy spy(cur_reply, &QModbusReply::finished);
        bool got_answer = spy.wait(MODBUS_QT_LIBRARY_RESPONSE_TIME);
        if (got_answer) {
            if (cur_reply->error() == QModbusDevice::NoError
                && cur_reply->result().isValid()) {
                //ответ был получен, ошибок нет, формат ответа верный
                return NO_ERROR;
            }
            if (cur_reply->error() == QModbusDevice::TimeoutError) {
                //master смог отправить запрос в Modbus сеть
                //но slave с указанным адресом не откликнулся
                return NO_RESPONSE_FROM_MODBUS_SLAVE;
            }
            //причина ошибки не выявлена
            return UNKNOWN_MODBUS_ERROR;
        }
        //master смог отправить запрос в Modbus сеть
        //но slave с указанным адресом не откликнулся
        return NO_RESPONSE_FROM_MODBUS_SLAVE;
    }
    //причина ошибки не выявлена
    return UNKNOWN_MODBUS_ERROR;
}

QPair<ModbusMaster::Status, ModbusMaster::ModbusError> ModbusMaster::readStatus()
{
    QPair<QBitArray, ModbusMaster::ModbusError> read_register_result = readRegister(STATUS_ADDRES);
    if (read_register_result.second != NO_ERROR) {
        //при использовании метода считывания регистра была ошибка
        return qMakePair(s_ERROR, read_register_result.second);
    }
    if (read_register_result.first.isNull()) {
        //пришёл пустой массив, внутреняя ошибка
        return qMakePair(s_ERROR, INTERNAL_MODBUS_MODULE_ERROR);
    }
    if (read_register_result.first.size() != 16) {
        //пришёл массив неверного размера, внутреняя ошибка
        return qMakePair(s_ERROR, INTERNAL_MODBUS_MODULE_ERROR);
    }
    quint16 status_result = QBitArrayToDec(read_register_result.first);
    ModbusMaster::Status status_enum = s_ERROR;
    switch (status_result) {
    case 0:
        status_enum = s_AUTO;
        break;
    case 1:
        status_enum = s_MANUAL;
        break;
    case 2:
        status_enum = s_DOWNTIME;
        break;
    case 3:
        status_enum = s_STOP;
        break;
    }
    return qMakePair(status_enum, read_register_result.second);
}

ModbusMaster::ModbusError ModbusMaster::writeCommand(ModbusMaster::Command cur_command)
{
    QBitArray command_val = decToQBitArray(cur_command);
    return writeRegister(COMMAND_ADDRES, command_val);
}

QPair<QString[2], ModbusMaster::ModbusError> ModbusMaster::readID()
{
    QString result_array[2] = { "", "" };
    if (QModbusDevice::state() != ConnectedState) {
        if (connectModbus() != NO_ERROR) {
            //COM порт при вызове метода не был подключен
            //переподключиться к COM порту не получилось
            return qMakePair(result_array, COM_CONNECTION_ERROR);
        }
    }
    QModbusRequest cur_request_unit(QModbusPdu::FunctionCode(0x34), (quint16)0, (quint16)0);
    auto* cur_request = sendRawRequest(cur_request_unit, LAB_MODBUS_NET_ADDRESS);
    if (cur_request) {
        QSignalSpy spy(cur_request, &QModbusReply::finished);
        bool got_answer = spy.wait(MODBUS_QT_LIBRARY_RESPONSE_TIME);
        const QModbusResponse cur_modbus_response = cur_request->rawResult();
        if (got_answer
            && cur_request->error() == QModbusDevice::NoError
            && cur_modbus_response.isValid()
            && cur_modbus_response.dataSize() == 5
            && cur_modbus_response.data().toHex().size() == 10
            && cur_modbus_response.data().toHex().at(1) == *"4") {
            //ответ был получен, ошибок нет, формат ответа верный
            result_array[0].setNum(
                cur_modbus_response.data().toHex().remove(0, 2).remove(4, 4).toInt(nullptr, 16), 10);
            result_array[1].setNum(
                cur_modbus_response.data().toHex().remove(0, 2).remove(0, 4).toInt(nullptr, 16), 10);
            //            qDebug() << cur_modbus_response.data().toHex();
            return qMakePair(result_array, NO_ERROR);
        }
    }
    if (connectModbus() != NO_ERROR) {
        //скорее всего, произошло "горячее отключение" COM
        //переподключение не помогло
        return qMakePair(result_array, COM_CONNECTION_ERROR);
    }
    cur_request = sendRawRequest(cur_request_unit, LAB_MODBUS_NET_ADDRESS);
    if (cur_request) {
        QSignalSpy spy(cur_request, &QModbusReply::finished);
        bool got_answer = spy.wait(MODBUS_QT_LIBRARY_RESPONSE_TIME);
        if (got_answer) {
            const QModbusResponse cur_modbus_response = cur_request->rawResult();
            if (cur_request->error() == QModbusDevice::NoError
                && cur_modbus_response.isValid()
                && cur_modbus_response.dataSize() == 5
                && cur_modbus_response.data().toHex().size() == 10
                && cur_modbus_response.data().toHex().at(1) == *"4") {
                //ответ был получен, ошибок нет, формат ответа верный
                result_array[0].setNum(
                    cur_modbus_response.data().toHex().remove(0, 2).remove(4, 4).toInt(nullptr, 16), 10);
                result_array[1].setNum(
                    cur_modbus_response.data().toHex().remove(0, 2).remove(0, 4).toInt(nullptr, 16), 10);
                //                qDebug() << cur_modbus_response.data().toHex();
                return qMakePair(result_array, NO_ERROR);
            }
            if (cur_request->error() == QModbusDevice::TimeoutError) {
                //master смог отправить запрос в Modbus сеть
                //но slave с указанным адресом не откликнулся
                return qMakePair(result_array, NO_RESPONSE_FROM_MODBUS_SLAVE);
            }
            //причина ошибки не выявлена
            return qMakePair(result_array, UNKNOWN_MODBUS_ERROR);
        }
        //master смог отправить запрос в Modbus сеть
        //но slave с указанным адресом не откликнулся
        return qMakePair(result_array, NO_RESPONSE_FROM_MODBUS_SLAVE);
    }
    //причина ошибки не выявлена
    return qMakePair(result_array, UNKNOWN_MODBUS_ERROR);
}

//небезопастный метод, так как при появлении ошибок, адрес устройства считается прежним,
//что может не соотвествовать действительности, нужны доп. проверки, использовать осторожно
ModbusMaster::ModbusError ModbusMaster::changeAddress(quint8 new_lab_address)
{
    if (new_lab_address >= 247 || new_lab_address < 1) {
        return INTERNAL_INVALID_MODBUS_INPUT_DATA;
    }
    if (QModbusDevice::state() != ConnectedState) {
        if (connectModbus() != NO_ERROR) {
            //COM порт при вызове метода не был подключен
            //переподключиться к COM порту не получилось
            return COM_CONNECTION_ERROR;
        }
    }
    QModbusRequest cur_request_unit(QModbusPdu::FunctionCode(0x35), (quint8)0, (quint8)0, (quint8)0, new_lab_address);
    auto* cur_request = sendRawRequest(cur_request_unit, LAB_MODBUS_NET_ADDRESS);
    if (cur_request) {
        QSignalSpy spy(cur_request, &QModbusReply::finished);
        bool got_answer = spy.wait(MODBUS_QT_LIBRARY_RESPONSE_TIME);
        const QModbusResponse cur_modbus_response = cur_request->rawResult();
        if (got_answer
            && cur_request->error() == QModbusDevice::NoError
            && cur_modbus_response.dataSize() == 4
            && cur_modbus_response.data().toHex().toLong(nullptr, 16) == new_lab_address) {
            //ответ был получен, ошибок нет, формат ответа верный
            LAB_MODBUS_NET_ADDRESS = (int)new_lab_address;
            return NO_ERROR;
        }
    }
    if (connectModbus() != NO_ERROR) {
        //скорее всего, произошло "горячее отключение" COM
        //переподключение не помогло
        return COM_CONNECTION_ERROR;
    }
    cur_request = sendRawRequest(cur_request_unit, LAB_MODBUS_NET_ADDRESS);
    if (cur_request) {
        QSignalSpy spy(cur_request, &QModbusReply::finished);
        bool got_answer = spy.wait(MODBUS_QT_LIBRARY_RESPONSE_TIME);
        if (got_answer) {
            const QModbusResponse cur_modbus_response = cur_request->rawResult();
            if (cur_request->error() == QModbusDevice::NoError
                && cur_modbus_response.dataSize() == 4
                && cur_modbus_response.data().toHex().toLong(nullptr, 16) == new_lab_address) {
                //ответ был получен, ошибок нет, формат ответа верный
                LAB_MODBUS_NET_ADDRESS = (int)new_lab_address;
                return NO_ERROR;
            }
            if (cur_request->error() == QModbusDevice::TimeoutError) {
                //master смог отправить запрос в Modbus сеть
                //но slave с указанным адресом не откликнулся
                return NO_RESPONSE_FROM_MODBUS_SLAVE;
            }
            //причина ошибки не выявлена
            return UNKNOWN_MODBUS_ERROR;
        }
        //master смог отправить запрос в Modbus сеть
        //но slave с указанным адресом не откликнулся
        return NO_RESPONSE_FROM_MODBUS_SLAVE;
    }
    //причина ошибки не выявлена
    return UNKNOWN_MODBUS_ERROR;
}

QBitArray ModbusMaster::decToQBitArray(const quint16& cur_val)
{
    QBitArray cur_bit_array = QBitArray((int)16, false);
    for (quint8 iter { 0 }; iter < 16; iter++) {
        if (cur_val & (1 << iter)) {
            cur_bit_array.setBit(iter, true);
        }
    }
    return cur_bit_array;
}

quint16 ModbusMaster::QBitArrayToDec(const QBitArray& cur_array)
{
    if (cur_array.size() != 16) {
        return (quint16)0;
    }
    quint16 cur_val = cur_array.at(0);
    for (quint8 iter { 1 }; iter < 16; iter++) {
        cur_val |= cur_array.at(iter) << iter;
    }
    return cur_val;
}

