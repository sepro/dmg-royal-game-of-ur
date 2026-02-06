/**
 * link.h
 * Serial link cable communication for multiplayer
 * Handles Game Boy serial port hardware and connection handshake
 */

#ifndef LINK_H
#define LINK_H

#include <stdint.h>

// Protocol constants
#define LINK_SYNC_BYTE    0xAA   // Handshake request
#define LINK_ACK_BYTE     0x55   // Handshake acknowledgment
#define LINK_TIMEOUT      60     // Frames (~1 second)
#define LINK_TRANSFER_WAIT 4     // Frames per byte transfer

// Connection status
typedef enum {
    LINK_DISCONNECTED = 0,
    LINK_CONNECTING,
    LINK_CONNECTED,
    LINK_ERROR
} LinkStatus_t;

// Role determination (master provides clock, slave receives)
typedef enum {
    LINK_ROLE_UNDETERMINED = 0,
    LINK_ROLE_MASTER,   // Provides clock, gets LIGHT side
    LINK_ROLE_SLAVE     // Receives clock, gets DARK side
} LinkRole_t;

// Exported state
extern LinkStatus_t link_status;
extern LinkRole_t link_role;

/**
 * Initialize link cable hardware
 * Clears SB/SC registers and resets state
 */
void link_init(void);

/**
 * Non-blocking connection step
 * Call from update loop; advances handshake state machine
 * @return Current link status
 */
LinkStatus_t link_connect_step(void);

/**
 * Send a byte over the link cable (blocking)
 * @param data Byte to send
 * @return Byte received during exchange
 */
uint8_t link_send(uint8_t data);

/**
 * Receive a byte with timeout
 * @param out_data Pointer to store received byte
 * @param timeout_frames Frames to wait before timeout
 * @return 1 if byte received, 0 if timeout
 */
uint8_t link_receive(uint8_t* out_data, uint8_t timeout_frames);

/**
 * Exchange a byte (send and receive simultaneously)
 * @param send_data Byte to send
 * @param recv_data Pointer to store received byte
 * @return 1 if exchange completed, 0 if timeout
 */
uint8_t link_exchange(uint8_t send_data, uint8_t* recv_data);

/**
 * Exchange a byte as slave (external clock) with custom send data
 * Like link_receive() but loads send_data instead of 0xFF
 * @param send_data Byte to send back to master
 * @param recv_data Pointer to store received byte
 * @param timeout_frames Frames to wait before timeout
 * @return 1 if exchange completed, 0 if timeout
 */
uint8_t link_exchange_slave(uint8_t send_data, uint8_t* recv_data, uint8_t timeout_frames);

/**
 * Reset link state to disconnected
 */
void link_reset(void);

#endif // LINK_H
