const IValue *GetPerDosValue(uint32_t id)
{
    if (id < 0x20)
        return nullptr;

    id -= 0x20;
    if (id >= sizeof_array(perdosIds))
        return nullptr;
    return perdosIds[id];
}

class PerDat
{
public:
    static void Write7BitEncodedUInt(uint8_t *const data, uint32_t &index, const uint32_t maxIndex, uint32_t value)
    {
        while (index < maxIndex)
        {
            uint8_t byte = value & 0x7F;
            value >>= 7;
            if (value)
                byte |= 1 << 7;
            data[index] = byte;
            index++;
            if (value == 0)
                break;
        }
    }

    static void Read7BitEncodedUInt(const uint8_t *const data, uint32_t &index, const uint32_t maxIndex, uint32_t &value)
    {
        value = 0;
        int shift = 0;
        uint8_t b;
        while (index < maxIndex)
        {
            b = data[index];
            value |= (uint32_t)(b & 0x7F) << shift;
            index++;
            if ((b >> 7 & 1) == 0)
                break;
            shift += 7;
        }
    }

    /**
     * \brief
     * \param data The data to parse
     * \param length The length of data
     * \param connected Will be set to true if add file (0x02) is received
     *                  and false if end (0x00) is received
     */
    static void Receive(uint8_t *data, uint16_t length, bool &connected)
    {
        uint32_t index = 0;
        while (index < length)
        {
            uint32_t code;
            Read7BitEncodedUInt(data, index, length, code);

            if (code != 0x10)
            {
                uint32_t dataLength;
                Read7BitEncodedUInt(data, index, length, dataLength);

                if (dataLength + index > length)
                {
                    Breakpoint();
                    return;
                }

                if (code < 0x20)
                {
                    switch (code)
                    {
                    case 0:
                        connected = false;
                        break;
                    case 2:
                        connected = true;
                        break;
                    case 0x0A:
                        // check that rtc instance is not null
                        if (rtcp)
                        {
                            unsigned long *value = reinterpret_cast<unsigned long *>(&data[index]);
                            time_t time = static_cast<time_t>(*value);
                            rtcp->SetTime(time);
                        }
                        break;
                    default:
                        Breakpoint();
                        return;
                    }
                }
                else
                {
                    const IValue *value = GetPerDosValue(code);
                    if (value != nullptr && (value->Size() == dataLength || (value->Size() == 0 && dataLength == 1)))
                    {
                        value->Set(&data[index]);
                    }
                    else
                        Breakpoint();
                }

                index += dataLength;
            }
        }
    }
};

struct PerDosSendInfo
{
    PerDosSendInfo(uint32_t id, uint16_t sendPeriod)
        : Id(id),
          SendPeriod(sendPeriod),
          Countdown(sendPeriod)
    {
    }

    uint32_t Id;
    uint16_t SendPeriod;
    uint16_t Countdown;
};

const int maxPerdosLength = 262;
const int minPerdosLength = 8;
const uint32_t perdosHeader = 0xFFFFFFU;
class PerDos
{
public:
    PerDos(peripheral::Uart &uart, uint16_t sendInfoSize, PerDosSendInfo *sendInfos)
        : _uart(uart),
          _sendInfoSize(sendInfoSize),
          _sendInfos(sendInfos)
    {
    }

    void Receive();

    void Transmit(bool alwaysTransmit);

    bool Connected()
    {
        uint32_t currentTime = HAL_GetTick();

        if (currentTime - _lastConnected < configTICK_RATE_HZ)
            return true;

        _lastConnected = currentTime - configTICK_RATE_HZ;
        return false;
    }

private:
    peripheral::Uart &_uart;
    uint8_t _rxBuffer[maxPerdosLength];
    uint16_t _rxEndIndex = 0;
    uint16_t _sendInfoSize;
    PerDosSendInfo *_sendInfos;
    uint32_t _lastConnected = -configTICK_RATE_HZ;
    bool _shouldBeConnected = false;
    bool _wasConnected = false;
    uint32_t _timeOffset = 0;

    int AttemptParse(int start, int endIndex);

    static bool IsHeader(uint8_t *start)
    {
        uint32_t header = *reinterpret_cast<uint32_t *>(start);
        return (header & perdosHeader) == perdosHeader;
    }
};

void PerDos::Receive()
{
    bool shouldRepeat;
    do
    {
        const int count = _uart.TryReceive(_rxBuffer + _rxEndIndex, maxPerdosLength - _rxEndIndex);

        if (count > 0)
        {
            _lastConnected = HAL_GetTick();
        }

        _rxEndIndex += count;
        shouldRepeat = _rxEndIndex == maxPerdosLength;

        int start = 0;
        while (start + minPerdosLength < _rxEndIndex)
        {
            if (IsHeader(&_rxBuffer[start]))
            {
                const int size = AttemptParse(start, _rxEndIndex);
                start += size;
                if (size == 0)
                    break;
            }
            else
            {
                do
                    start++;
                while (start + 4 < _rxEndIndex && !IsHeader(&_rxBuffer[start]));
            }
        }

        _rxEndIndex -= start;

        if (_rxEndIndex < 0 || _rxEndIndex > maxPerdosLength)
        {
            Breakpoint();
            _rxEndIndex = 0;
        }

        if (shouldRepeat && _rxEndIndex == maxPerdosLength)
        {
            Breakpoint();
            _rxEndIndex = 0;
        }

        memmove(_rxBuffer, &_rxBuffer[start], _rxEndIndex);
    } while (shouldRepeat);
}

void PerDos::Transmit(bool alwaysTransmit)
{
    const bool connected = Connected();
    if (!alwaysTransmit && !connected)
    {
        _wasConnected = connected;
        return;
    }
    uint8_t sendBuffer[maxPerdosLength];
    uint32_t endIndex = 4;

    if (!_wasConnected)
        _timeOffset = HAL_GetTick();

    _wasConnected = connected;

    sendBuffer[endIndex++] = 0x0A;
    uint64_t time = static_cast<uint64_t>(10000) * (HAL_GetTick() - _timeOffset);
    sendBuffer[endIndex++] = sizeof(time);
    memcpy(&sendBuffer[endIndex], &time, sizeof(time));
    endIndex += sizeof(time);

    for (int i = 0; i < _sendInfoSize;)
    {
        while (i < _sendInfoSize)
        {
            PerDosSendInfo *sendInfo = &_sendInfos[i];
            sendInfo->Countdown--;
            if (sendInfo->Countdown == 0)
            {
                sendInfo->Countdown = sendInfo->SendPeriod;
                uint32_t id = sendInfo->Id;
                const IValue *value = GetPerDosValue(id);
                size_t size;
                if (value == nullptr || (size = value->Size()) > 252)
                {
                    Breakpoint();
                    i++;
                    continue;
                }

                if (size == 0)
                    size = 1;

                if (size + endIndex + 5 >= maxPerdosLength - 14)
                {
                    sendInfo->Countdown = 1;
                    break;
                }

                PerDat::Write7BitEncodedUInt(sendBuffer, endIndex, maxPerdosLength, id);
                sendBuffer[endIndex++] = static_cast<uint8_t>(size);
                value->Get(&sendBuffer[endIndex]);
                endIndex += size;
            }
            i++;
        }

        if (endIndex > 14)
        {
            if (endIndex % 2 != 0)
                sendBuffer[endIndex++] = 0x10;

            *reinterpret_cast<uint32_t *>(sendBuffer) = perdosHeader | (endIndex - 4) << 24;

            *reinterpret_cast<uint32_t *>(&sendBuffer[endIndex]) = Fletcher32(reinterpret_cast<uint16_t *>(sendBuffer + 4), (endIndex - 4) / 2);
            endIndex += 4;

            _uart.TryTransmit(sendBuffer, endIndex, false);
        }
        endIndex = 14;
    }
}

int PerDos::AttemptParse(int start, int endIndex)
{
    const uint8_t dataLength = _rxBuffer[start + 3];
    if (dataLength % 2 != 0)
        return 4;

    if (start + 8 + dataLength <= endIndex)
    {
        const uint16_t *data = reinterpret_cast<uint16_t *>(&_rxBuffer[start + 4]);
        const uint32_t checksum = Fletcher32(data, dataLength / 2);
        if (checksum != *reinterpret_cast<uint32_t *>(&_rxBuffer[start + 4 + dataLength]))
            return 4;

        PerDat::Receive(&_rxBuffer[start + 4], dataLength, _shouldBeConnected);

        return 8 + dataLength;
    }
    return 0;
}