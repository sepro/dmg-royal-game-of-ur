/**
 * link.c
 * Serial link cable communication implementation
 * Uses polling-based serial transfer (no interrupts)
 *
 * Game Boy serial protocol:
 * - Master (internal clock, SC bit 0 = 1): transfer completes in ~1ms
 * - Slave (external clock, SC bit 0 = 0): waits for master to clock
 * - SB_REG holds outgoing byte before transfer, incoming byte after
 * - SC_REG bit 7 is cleared by hardware when transfer completes
 *
 * Handshake approach:
 * - Alternate between master-try and slave-try phases
 * - Master sends SYNC; if slave had SYNC loaded, master receives SYNC back
 * - Once roles determined, exchange ACK bytes to confirm
 */

#include <gb/gb.h>
#include <stdint.h>
#include "link.h"

// Serial control flags
#define SC_START      0x80   // Start transfer (bit 7)
#define SC_CLOCK_INT  0x01   // Internal clock = master
#define SC_CLOCK_EXT  0x00   // External clock = slave

// Handshake states
// Each _START state initiates a transfer once.
// Each _WAIT state polls for completion across frames.
#define HS_IDLE          0
#define HS_MASTER_START  1
#define HS_MASTER_WAIT   2
#define HS_SLAVE_START   3
#define HS_SLAVE_WAIT    4
#define HS_ACK_START     5
#define HS_ACK_WAIT      6

// Timing
#define MASTER_TRY_FRAMES  3    // Internal clock completes in <1 frame; give margin
#define SLAVE_TRY_FRAMES   30   // Wait up to ~0.5s for master

// Global state
LinkStatus_t link_status = LINK_DISCONNECTED;
LinkRole_t link_role = LINK_ROLE_UNDETERMINED;

// Persistent armed state for non-blocking slave recv
static uint8_t slave_recv_armed = 0;

// Buffered receive: link_pump_recv() stores game data here so the slave
// never has a "deaf window" between game phases.
static uint8_t link_pending_byte;   // Buffered received game data
static uint8_t link_has_pending;    // 1 = buffer has unread data

// Internal state
static uint8_t hs_state = HS_IDLE;
static uint8_t hs_timer = 0;

/**
 * Check if serial transfer is complete (bit 7 cleared by hardware)
 */
static uint8_t is_transfer_done(void) {
    return (SC_REG & SC_START) == 0;
}

/**
 * Initialize link cable hardware
 */
void link_init(void) {
    SB_REG = 0x00;
    SC_REG = 0x00;

    link_status = LINK_DISCONNECTED;
    link_role = LINK_ROLE_UNDETERMINED;
    hs_state = HS_IDLE;
    hs_timer = 0;
    slave_recv_armed = 0;
    link_has_pending = 0;
    link_pending_byte = 0;
}

/**
 * Reset link state
 */
void link_reset(void) {
    link_init();
}

/**
 * Exchange a byte (blocking, master mode)
 * Only use after connection is established.
 */
uint8_t link_exchange(uint8_t send_data, uint8_t* recv_data) {
    SB_REG = send_data;
    SC_REG = SC_START | SC_CLOCK_INT;

    // Busy-spin ~4096 iterations (~1ms at 4MHz) for internal clock completion.
    // Internal clock transfers complete in ~1ms; this catches them without
    // blocking an entire frame via wait_vbl_done().
    {
        uint16_t spin = 4096;
        while (spin--) {
            if (is_transfer_done()) {
                if (recv_data) *recv_data = SB_REG;
                return 1;
            }
        }
    }

    // Fall through to frame-based wait (safety net for slow completion)
    {
        uint8_t f = 0;
        while (f < LINK_TRANSFER_WAIT) {
            if (is_transfer_done()) {
                if (recv_data) {
                    *recv_data = SB_REG;
                }
                return 1;
            }
            wait_vbl_done();
            f++;
        }
    }

    SC_REG = 0;
    return 0;
}

/**
 * Send a byte (blocking, master mode)
 */
uint8_t link_send(uint8_t data) {
    uint8_t dummy;
    return link_exchange(data, &dummy);
}

/**
 * Receive a byte with timeout (blocking, slave mode)
 */
uint8_t link_receive(uint8_t* out_data, uint8_t timeout_frames) {
    SB_REG = 0xFF;
    SC_REG = SC_START | SC_CLOCK_EXT;

    {
        uint8_t f = 0;
        while (f < timeout_frames) {
            if (is_transfer_done()) {
                if (out_data) {
                    *out_data = SB_REG;
                }
                return 1;
            }
            wait_vbl_done();
            f++;
        }
    }

    SC_REG = 0;
    return 0;
}

/**
 * Exchange a byte as slave (external clock) with custom send data
 * Identical to link_receive() but loads send_data instead of 0xFF
 */
uint8_t link_exchange_slave(uint8_t send_data, uint8_t* recv_data, uint8_t timeout_frames) {
    SB_REG = send_data;
    SC_REG = SC_START | SC_CLOCK_EXT;

    {
        uint8_t f = 0;
        while (f < timeout_frames) {
            if (is_transfer_done()) {
                if (recv_data) {
                    *recv_data = SB_REG;
                }
                return 1;
            }
            wait_vbl_done();
            f++;
        }
    }

    SC_REG = 0;
    return 0;
}

/**
 * Non-blocking ready sync for screen transitions
 * Master sends LINK_READY_BYTE with internal clock,
 * slave sends with external clock (4-frame timeout).
 * @return 0=still waiting, 1=both sides ready, 2=peer cancelled
 */
uint8_t link_ready_sync(void) {
    uint8_t recv = 0;
    uint8_t ok;

    if (link_role == LINK_ROLE_MASTER) {
        ok = link_exchange(LINK_READY_BYTE, &recv);
    } else {
        ok = link_exchange_slave(LINK_READY_BYTE, &recv, LINK_TRANSFER_WAIT);
    }

    if (ok) {
        if (recv == LINK_CANCEL_BYTE) {
            return 2;
        }
        if (recv == LINK_READY_BYTE) {
            return 1;
        }
    }

    return 0;
}

/**
 * Send cancel notification to peer, then reset link
 */
void link_cancel(void) {
    uint8_t dummy;
    if (link_role == LINK_ROLE_MASTER) {
        link_exchange(LINK_CANCEL_BYTE, &dummy);
    } else {
        link_exchange_slave(LINK_CANCEL_BYTE, &dummy, LINK_TRANSFER_WAIT);
    }
    link_reset();
}

/**
 * Send a game data byte over the link cable
 * Master: retry loop until slave signals readiness (LINK_READY_RECV).
 * Slave: single exchange with long timeout (master controls clock).
 */
uint8_t link_game_send(uint8_t data) {
    uint8_t recv;

    // Cancel any pending slave recv (we're switching to send mode)
    slave_recv_armed = 0;
    link_has_pending = 0;
    SC_REG = 0;  // Disarm SC before loading send data

    if (link_role == LINK_ROLE_MASTER) {
        // Master retry loop: slave may not be listening yet
        uint8_t retries = 0;
        while (retries < LINK_SEND_RETRIES) {
            if (link_exchange(data, &recv)) {
                if (recv == LINK_READY_RECV) {
                    // Slave was listening, data delivered
                    return 1;
                }
            }
            // Slave not ready (got 0xFF or other), wait a frame and retry
            wait_vbl_done();
            retries++;
        }
        return 0;  // Exhausted retries
    }

    // Slave: single exchange, master controls clock timing
    return link_exchange_slave(data, (void *)0, LINK_GAME_TIMEOUT);
}

/**
 * Receive a game data byte over the link cable (non-blocking).
 * Returns 1 if valid game data received, 0 if nothing yet.
 *
 * Slave: uses persistent armed state — arms SC once and checks each
 * frame without blocking. Eliminates deaf windows between calls.
 * Master: single exchange per call (internal clock, instant).
 *
 * Both sides load LINK_READY_RECV so the sender's retry can confirm delivery.
 * Filters out idle (0x00), hardware (0xFF), and protocol (READY_RECV) bytes.
 */
uint8_t link_game_recv(uint8_t *out) {
    uint8_t recv;

    if (link_role == LINK_ROLE_SLAVE) {
        // Check buffered data from link_pump_recv() first
        if (link_has_pending) {
            *out = link_pending_byte;
            link_has_pending = 0;
            return 1;
        }

        if (slave_recv_armed) {
            // Already armed — check if transfer completed (non-blocking)
            if (is_transfer_done()) {
                recv = SB_REG;
                slave_recv_armed = 0;
                if (recv != LINK_IDLE_BYTE && recv != 0xFF &&
                    recv != LINK_READY_RECV) {
                    *out = recv;
                    return 1;
                }
                // Got a protocol/idle byte — re-arm immediately
                SB_REG = LINK_READY_RECV;
                SC_REG = SC_START | SC_CLOCK_EXT;
                slave_recv_armed = 1;
            }
            // Transfer still pending — return immediately, no blocking
        } else {
            // Not armed — arm now for external clock
            SB_REG = LINK_READY_RECV;
            SC_REG = SC_START | SC_CLOCK_EXT;
            slave_recv_armed = 1;
        }
        return 0;
    }

    // Master: instant exchange (internal clock, completes in <1ms)
    if (link_exchange(LINK_READY_RECV, &recv)) {
        if (recv != LINK_IDLE_BYTE && recv != 0xFF &&
            recv != LINK_READY_RECV) {
            *out = recv;
            return 1;
        }
    }
    return 0;
}

/**
 * Pump slave receive — call every frame during link gameplay.
 * Keeps the slave's serial port armed at all times so there are no
 * "deaf windows" between game phases. If game data arrives it is
 * buffered in link_pending_byte for the next link_game_recv() call.
 *
 * Safe to call when role is master (no-op) or when not in a game.
 */
void link_pump_recv(void) {
    uint8_t recv;

    // Only pump for slave role; master drives clock, no deaf window
    if (link_role != LINK_ROLE_SLAVE) return;

    // Don't overwrite an unread buffered byte
    if (link_has_pending) return;

    if (slave_recv_armed) {
        // Check if a transfer completed
        if (is_transfer_done()) {
            recv = SB_REG;
            slave_recv_armed = 0;

            // Filter protocol/idle bytes — only buffer game data
            if (recv != LINK_IDLE_BYTE && recv != 0xFF &&
                recv != LINK_READY_RECV) {
                link_pending_byte = recv;
                link_has_pending = 1;
            }

            // Re-arm immediately
            SB_REG = LINK_READY_RECV;
            SC_REG = SC_START | SC_CLOCK_EXT;
            slave_recv_armed = 1;
        }
    } else {
        // Not armed — arm now
        SB_REG = LINK_READY_RECV;
        SC_REG = SC_START | SC_CLOCK_EXT;
        slave_recv_armed = 1;
    }
}

/**
 * Non-blocking connection step (call once per frame from update loop)
 *
 * Protocol:
 * 1. Try master: send SYNC with internal clock (completes in <1 frame)
 *    - If received SYNC back → other side was in slave mode, we're master
 *    - If received 0xFF/other → no slave ready, switch to slave mode
 * 2. Try slave: load SYNC, wait with external clock
 *    - If transfer completes → other side sent as master, we're slave
 *    - If timeout → go back to master try
 * 3. Once roles set, exchange ACK to confirm
 */
LinkStatus_t link_connect_step(void) {
    uint8_t recv;

    switch (hs_state) {
        case HS_IDLE:
            link_status = LINK_CONNECTING;
            hs_state = HS_MASTER_START;
            break;

        /* --- Master attempt: start transfer once --- */
        case HS_MASTER_START:
            SB_REG = LINK_SYNC_BYTE;
            SC_REG = SC_START | SC_CLOCK_INT;
            hs_timer = 0;
            hs_state = HS_MASTER_WAIT;
            break;

        /* --- Master attempt: poll for completion --- */
        case HS_MASTER_WAIT:
            hs_timer++;
            if (is_transfer_done()) {
                recv = SB_REG;
                if (recv == LINK_SYNC_BYTE) {
                    // Other device had SYNC loaded as slave → we are master
                    link_role = LINK_ROLE_MASTER;
                    hs_state = HS_ACK_START;
                } else {
                    // Got 0xFF or garbage → no slave listening, try slave
                    hs_state = HS_SLAVE_START;
                }
            } else if (hs_timer >= MASTER_TRY_FRAMES) {
                // Internal clock should complete in <1ms; something wrong
                SC_REG = 0;
                hs_state = HS_SLAVE_START;
            }
            break;

        /* --- Slave attempt: start listening once --- */
        case HS_SLAVE_START:
            SB_REG = LINK_SYNC_BYTE;
            SC_REG = SC_START | SC_CLOCK_EXT;
            hs_timer = 0;
            hs_state = HS_SLAVE_WAIT;
            break;

        /* --- Slave attempt: poll for incoming master transfer --- */
        case HS_SLAVE_WAIT:
            hs_timer++;
            if (is_transfer_done()) {
                recv = SB_REG;
                if (recv == LINK_SYNC_BYTE) {
                    // Master sent SYNC → we are slave
                    link_role = LINK_ROLE_SLAVE;
                    hs_state = HS_ACK_START;
                } else {
                    // Unexpected byte, retry as master
                    hs_state = HS_MASTER_START;
                }
            } else if (hs_timer >= SLAVE_TRY_FRAMES) {
                // No master found, cancel and try master again
                SC_REG = 0;
                hs_state = HS_MASTER_START;
            }
            break;

        /* --- ACK exchange: start transfer once --- */
        case HS_ACK_START:
            SB_REG = LINK_ACK_BYTE;
            if (link_role == LINK_ROLE_MASTER) {
                SC_REG = SC_START | SC_CLOCK_INT;
            } else {
                SC_REG = SC_START | SC_CLOCK_EXT;
            }
            hs_timer = 0;
            hs_state = HS_ACK_WAIT;
            break;

        /* --- ACK exchange: poll for completion --- */
        case HS_ACK_WAIT:
            hs_timer++;
            if (is_transfer_done()) {
                recv = SB_REG;
                if (recv == LINK_ACK_BYTE || recv == LINK_SYNC_BYTE) {
                    link_status = LINK_CONNECTED;
                    hs_state = HS_IDLE;
                } else {
                    // Unexpected, retry from scratch
                    hs_state = HS_MASTER_START;
                }
            } else if (hs_timer >= LINK_TIMEOUT) {
                // ACK timed out, retry from scratch
                SC_REG = 0;
                hs_state = HS_MASTER_START;
            }
            break;

        default:
            hs_state = HS_IDLE;
            break;
    }

    return link_status;
}
