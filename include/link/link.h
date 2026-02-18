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
#define LINK_READY_BYTE   0xBB   // Screen-transition ready sync
#define LINK_CANCEL_BYTE  0xCC   // Peer cancellation notification
#define LINK_PROFILE_TAG  0x80   // Profile bytes sent as 0x80|index
#define LINK_PROFILE_MASK 0x03   // Extract profile index from tagged byte

// Game protocol tags (distinct from all other protocol bytes)
#define LINK_DICE_TAG    0x10  // 0x10-0x14 = dice roll (value 0-4)
#define LINK_PIECE_TAG   0x20  // 0x20-0x26 = piece index (0-6)
#define LINK_NO_MOVES    0xF0  // No valid moves this turn
#define LINK_IDLE_BYTE   0x00  // Idle/no-data marker

#define LINK_GAME_TIMEOUT  255 // Max frames for game exchange (~4.25s)

// Reliable transfer protocol
#define LINK_READY_RECV   0xDD  // Receiver loads this to signal readiness
#define LINK_SEND_RETRIES 180   // Max retry frames for master send (~3s safety margin)

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
extern uint8_t link_alive_flag;  // Set when an exchange proves the link is alive

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

/**
 * Soft-reset serial port state without clearing link_role/link_status.
 * Clears SB/SC registers, slave_recv_armed, buffered data, and alive flag.
 * Use between screens that share a link connection (e.g. link_connect → link_profile).
 */
void link_soft_reset(void);

/**
 * Non-blocking ready sync for screen transitions
 * Both devices call this each frame when ready to advance.
 * @return 0=still waiting, 1=both sides ready, 2=peer cancelled
 */
uint8_t link_ready_sync(void);

/**
 * Send cancel notification to peer, then reset link
 * Best-effort: if transfer fails (peer disconnected), resets anyway.
 */
void link_cancel(void);

/**
 * Pump slave receive — call every frame during link gameplay.
 * Keeps slave serial port armed to eliminate deaf windows.
 * Buffers any received game data for the next link_game_recv() call.
 * No-op if role is master.
 */
void link_pump_recv(void);

/**
 * Master keepalive — clock an idle exchange so the slave knows
 * the link is still active.  Sets link_alive_flag if the slave
 * responds with LINK_READY_RECV.  No-op if role is slave.
 */
void link_keepalive(void);

/**
 * Send a game data byte over the link cable
 * Master uses internal clock, slave uses external clock.
 * @param data Game data byte to send
 * @return 1 on success, 0 on timeout
 */
uint8_t link_game_send(uint8_t data);

/**
 * Receive a game data byte over the link cable
 * Non-blocking receive. Returns 1 and stores data in *out if valid game
 * data is available; returns 0 immediately if nothing yet.
 * @param out Pointer to store received byte
 */
uint8_t link_game_recv(uint8_t *out);

#endif // LINK_H
