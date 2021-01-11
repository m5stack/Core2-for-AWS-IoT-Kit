#ifndef CONFIG_STRUCTS_H
#define CONFIG_STRUCTS_H

/** \brief
   This header file provides definitions of Configuration zone in various Secure elements
 */
#include <stddef.h>

#pragma pack(push, 1 )
typedef struct
{
    uint8_t enable : 1;
    uint8_t reserved : 7;
}aes_enable_t;

typedef struct
{
    uint8_t enable : 1;
    uint8_t reserved : 7;
}i2c_enable_t;

typedef struct
{
    uint8_t enable : 1;
    uint8_t reserved : 3;
    uint8_t count_match_key : 4;
}count_match_t;

typedef struct
{
    uint8_t user_extra_add : 1;
    uint8_t ttl_enable : 1;
    uint8_t watchdog_duration : 1;
    uint8_t reserved : 5;
}chip_mode_508_t;

typedef struct
{
    uint8_t user_extra_add : 1;
    uint8_t ttl_enable : 1;
    uint8_t watchdog_duration : 1;
    uint8_t clock_divider : 5;
}chip_mode_608_t;

typedef struct
{
    uint8_t use_flag;
    uint8_t update_count;
}counter_204_t;

typedef struct
{
    uint16_t read_key : 4;
    uint16_t no_mac : 1;
    uint16_t limited_use : 1;
    uint16_t encrypt_read : 1;
    uint16_t is_secret : 1;
    uint16_t write_key : 4;
    uint16_t write_config : 4;
}slot_config_t;

typedef struct
{
    uint8_t uselock_enable : 4;
    uint8_t uselock_key : 4;
}use_lock_t;

typedef struct
{
    uint8_t volatilekey_permit_slot : 4;
    uint8_t reserved : 3;
    uint8_t volatilekey_permit_enable : 1;
}volatile_key_permission_t;

typedef struct
{
    uint16_t secureboot_mode : 2;
    uint16_t reserved0 : 1;
    uint16_t secureboot_persistent_enable : 1;
    uint16_t secureboot_rand_nonce : 1;
    uint16_t reserved1 : 3;
    uint16_t secureboot_sigdig : 4;
    uint16_t secureboot_pubkey : 4;
}secureboot_t;

typedef struct
{
    uint16_t power_on_self_test : 1;
    uint16_t io_protection_key_enable : 1;
    uint16_t kdf_aes_enable : 1;
    uint16_t auto_clear_first_fail : 1;
    uint16_t reserved : 4;
    uint16_t ecdh_protection_bits : 2;
    uint16_t kdf_protection_bits : 2;
    uint16_t io_protection_key : 4;
}chip_options_t;

typedef struct
{
    uint8_t public_position : 4;
    uint8_t template_length : 4;
}x509_format_t;

typedef struct
{
    uint16_t private: 1;
    uint16_t pub_info : 1;
    uint16_t key_type : 3;
    uint16_t lockable : 1;
    uint16_t req_random : 1;
    uint16_t req_auth : 1;
    uint16_t auth_key : 4;
    uint16_t persistent_disable : 1;
    uint16_t rfu : 1;
    uint16_t x509_id : 2;
}key_config_t;

typedef struct
{
    uint8_t       sn03[4];
    uint8_t       rev_num[4];
    uint8_t       sn48[5];
    uint8_t       reserved13;
    i2c_enable_t  i2c_enable;
    uint8_t       reserved15;
    uint8_t       i2c_address;
    uint8_t       checkmac_config;
    uint8_t       otp_mode;
    uint8_t       selector_mode;
    slot_config_t slot_config;
    counter_204_t counter[8];
    uint8_t       last_key_use[16];
    uint8_t       user_extra;
    uint8_t       selector;
    uint8_t       lock_value;
    uint8_t       lock_config;
}atsha204_config_t;

typedef struct
{
    uint8_t         sn03[4];
    uint8_t         rev_num[4];
    uint8_t         sn48[5];
    uint8_t         reserved13;
    i2c_enable_t    i2c_enable;
    uint8_t         reserved15;
    uint8_t         i2c_address;
    uint8_t         reserved17;
    uint8_t         otp_mode;
    chip_mode_508_t chip_mode;
    slot_config_t   slot_config[16];
    uint8_t         counter0[8];
    uint8_t         counter1[8];
    uint8_t         last_key_use[16];
    uint8_t         user_extra;
    uint8_t         selector;
    uint8_t         lock_value;
    uint8_t         lock_config;
    uint16_t        slot_locked;
    uint16_t        rfu;
    x509_format_t   x509_format[4];
    key_config_t    key_config[16];
}atecc508_config_t;

typedef struct
{
    uint8_t                   sn03[4];
    uint8_t                   rev_num[4];
    uint8_t                   sn48[5];
    aes_enable_t              aes_enable;
    i2c_enable_t              i2c_enable;
    uint8_t                   reserved15;
    uint8_t                   i2c_address;
    uint8_t                   reserved17;
    count_match_t             count_match;
    chip_mode_608_t           chip_mode;
    slot_config_t             slot_config[16];
    uint8_t                   counter0[8];
    uint8_t                   counter1[8];
    use_lock_t                use_lock;
    volatile_key_permission_t volatile_key_permission;
    secureboot_t              secureboot;
    uint8_t                   kdf_iv_loc;
    uint16_t                  kdf_iv_str;
    uint8_t                   reserved68[9];
    uint8_t                   user_extra;
    uint8_t                   user_extra_add;
    uint8_t                   lock_value;
    uint8_t                   lock_config;
    uint16_t                  slot_locked;
    chip_options_t            chip_options;
    x509_format_t             x509_format[4];
    key_config_t              key_config[16];
}atecc608_config_t;

#pragma pack(pop)

#endif