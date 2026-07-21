#ifndef BTSTACK_CONFIG_H
#define BTSTACK_CONFIG_H

// Core BLE Roles (Fixes the 'le_advertisements_state' error)
// #define ENABLE_BLE
#define ENABLE_LE_PERIPHERAL
// #define ENABLE_LE_SECURE_CONNECTIONS

// Memory Allocation & Alignment (Fixes the MALLOC and ALIGNMENT errors)
#define HAVE_MALLOC
#define HCI_ACL_CHUNK_SIZE_ALIGNMENT 4
#define HCI_OUTGOING_PRE_BUFFER_SIZE 4
#define HCI_ACL_PAYLOAD_SIZE (255 + 4)

// Database Limits (Fixes the NVM errors)
#define MAX_NR_HCI_CONNECTIONS 1
#define NVM_NUM_DEVICE_DB_ENTRIES 1
#define NVM_NUM_LINK_KEYS 1
#define MAX_NR_LE_DEVICE_DB_ENTRIES 1
#define MAX_NR_SM_LOOKUP_ENTRIES 1
#define MAX_NR_WHITELIST_ENTRIES 1

// GATT / L2CAP Requirements
#define ENABLE_GATT_SERVER
#define MAX_NR_GATT_CLIENTS 0
#define MAX_NR_L2CAP_SERVICES  2
#define MAX_NR_L2CAP_CHANNELS  2

#define ECC_P256_KEYGEN_EXTRA_RANDOM 0

// Debugging
#define ENABLE_LOG_ERROR
#define ENABLE_LOG_INFO
#define ENABLE_PRINTF_HEXDUMP

#endif