#ifndef H_CRSF_CONTROLLER
#define H_CRSF_CONTROLLER

#include "handset.h"
#include "crsf_protocol.h"
#ifndef TARGET_NATIVE
#ifdef USE_DMA_SERIAL
#include "SerialPortDriver.h"
#else
#include "HardwareSerial.h"
#endif
#endif
#include "common.h"

#ifdef PLATFORM_ESP32
#include "driver/uart.h"
#endif

class CRSFHandset final : public Handset
{

public:
    /////Variables/////
    void Begin() override;
    void End() override;

#ifdef CRSF_TX_MODULE
    bool IsArmed() override { return CRSF_to_BIT(ChannelData[4]); } // AUX1
    void handleInput() override;
    void handleOutput();

#ifdef USE_DMA_SERIAL
    static SerialPort& Port;
#else
    static HardwareSerial Port;
#endif
    static Stream *PortSecondary; // A second UART used to mirror telemetry out on the TX, not read from

    static uint8_t modelId;         // The model ID as received from the Transmitter
    static bool ForwardDevicePings; // true if device pings should be forwarded OTA
    static bool elrsLUAmode;

    static uint32_t GoodPktsCountResult; // need to latch the results
    static uint32_t BadPktsCountResult;  // need to latch the results

    static void makeLinkStatisticsPacket(uint8_t buffer[LinkStatisticsFrameLength + 4]);

    static void packetQueueExtended(uint8_t type, void *data, uint8_t len);

    void setPacketInterval(int32_t PacketInterval) override;
    void JustSentRFpacket() override;
    void sendTelemetryToTX(uint8_t *data) override;

    static uint8_t getModelID() { return modelId; }

    uint8_t GetMaxPacketBytes() const override { return maxPacketBytes; }
    static uint32_t GetCurrentBaudRate() { return UARTrequestedBaud; }

private:
    inBuffer_U inBuffer = {};

    /// OpenTX mixer sync ///
    volatile uint32_t dataLastRecv = 0;
    volatile int32_t OpenTXsyncOffset = 0;
    volatile int32_t OpenTXsyncWindow = 0;
    volatile int32_t OpenTXsyncWindowSize = 0;
    uint32_t OpenTXsyncLastSent = 0;

    /// UART Handling ///
    uint8_t SerialInPacketLen = 0; // length of the CRSF packet as measured
    uint8_t SerialInPacketPtr = 0; // index where we are reading/writing
    bool CRSFframeActive = false;  // since we get a copy of the serial data use this flag to know when to ignore it
    uint32_t GoodPktsCount = 0;
    uint32_t BadPktsCount = 0;
    uint32_t UARTwdtLastChecked = 0;
    uint8_t maxPacketBytes = CRSF_MAX_PACKET_LEN;
    uint8_t maxPeriodBytes = CRSF_MAX_PACKET_LEN;

    static uint8_t UARTcurrentBaudIdx;
    static uint32_t UARTrequestedBaud;

#if defined(PLATFORM_ESP32)
    bool UARTinverted = false;
#endif

    void sendSyncPacketToTX();
    void adjustMaxPacketSize();
    void duplex_set_RX() const;
    void duplex_set_TX() const;
    void RcPacketToChannelsData();
    bool processInternalCrsfPackage(uint8_t *package);
    bool ProcessPacket();
    bool UARTwdt();
    uint32_t autobaud();
    void flush_port_input();
#endif
};

#endif
