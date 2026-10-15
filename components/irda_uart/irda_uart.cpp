// irda_uart.cpp
//
// Puts an ESP-IDF UART into IrDA mode, as either a receiver or a transmitter.
// The Resideo/Honeywell water heater controllers signal with IrDA-style short
// pulses rather than ordinary UART levels.
//
// Register layout differs between chips:
//   ESP32-C3 (and classic ESP32, S2, S3): CONF0 takes effect immediately.
//   ESP32-C6, C5, H2, P4: the register is CONF0_SYNC and a change only takes
//   effect after the REG_UPDATE strobe is written.
// Both are handled below. Tested on the ESP32-C3 upstream; the ESP32-C6 path
// is new in this repository.

#include "irda_uart.h"
#include "driver/uart.h"
#include "soc/uart_reg.h"

namespace esphome {
namespace irda_uart {

void IrdaUart::setup() {
  uart_port_t port = (uart_port_t) this->uart_->get_hw_serial_number();

  ESP_ERROR_CHECK(uart_set_mode(port, UART_MODE_IRDA));

  if (invert_) {
    ESP_ERROR_CHECK(uart_set_line_inverse(
        port, tx_en_ ? UART_SIGNAL_IRDA_TX_INV : UART_SIGNAL_IRDA_RX_INV));
  }

#ifdef UART_CONF0_SYNC_REG
  // Synchronised UART config registers (ESP32-C6 and friends).
  if (tx_en_) {
    REG_SET_BIT(UART_CONF0_SYNC_REG(port), UART_IRDA_TX_EN);   // TX-only
  } else {
    REG_CLR_BIT(UART_CONF0_SYNC_REG(port), UART_IRDA_TX_EN);   // RX-only
  }
  // Push the change from the APB clock domain into the UART core.
  REG_SET_BIT(UART_REG_UPDATE_REG(port), UART_REG_UPDATE);
  while (REG_GET_BIT(UART_REG_UPDATE_REG(port), UART_REG_UPDATE)) {
  }
#else
  // Immediate UART config registers (ESP32-C3 and friends).
  if (tx_en_) {
    REG_SET_BIT(UART_CONF0_REG(port), UART_IRDA_TX_EN);   // TX-only
  } else {
    REG_CLR_BIT(UART_CONF0_REG(port), UART_IRDA_TX_EN);   // RX-only
  }
#endif
}

}  // namespace irda_uart
}  // namespace esphome
