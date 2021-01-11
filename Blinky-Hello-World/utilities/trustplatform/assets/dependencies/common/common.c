#include "cryptoauthlib.h"
#include "atca_iface.h"
#include "atca_devtypes.h"

#include <stdio.h>


/** \brief - Returns the device type based on the revision_byte
 */
ATCADeviceType get_device_type_id(uint8_t revision_byte)
{
    ATCADeviceType dev_type;

    switch (revision_byte)
    {
    case 0x00:
    case 0x02:
        dev_type = ATSHA204A;
        break;

    case 0x10:
        dev_type = ATECC108A;
        break;

    case 0x50:
        dev_type = ATECC508A;
        break;

    case 0x60:
        dev_type = ATECC608;
        break;

    default:
        dev_type = ATCA_DEV_UNKNOWN;
        break;
    }

    return dev_type;
}

/** \brief - Updates the devtype in cfg based on the info response
 */
ATCA_STATUS check_device_type_in_cfg(ATCAIfaceCfg* cfg, bool overwrite)
{
   uint8_t revision[4];
   uint8_t serial_number[9];
   ATCADeviceType device_type;
   ATCA_STATUS status = !ATCA_SUCCESS;
   char displaystr[200];
   size_t displaylen;

   do
   {
      if ((status = atcab_init(cfg)) != ATCA_SUCCESS)
      {
         printf("atcab_init() failed with ret=0x%08X\r\n", status);
         break;
      }

      // Request the Revision Number
      if ((status = atcab_info(revision)) != ATCA_SUCCESS)
      {
         printf("atcab_info() failed with ret=0x%08X\r\n", status);
         break;
      }
      displaylen = sizeof(displaystr);
      atcab_bin2hex(revision, sizeof(revision), displaystr, &displaylen);
      printf("\nDevice revision: %s\r\n", displaystr);

      // Request the Serial Number
      if ((status = atcab_read_serial_number(serial_number)) != ATCA_SUCCESS)
      {
         printf("atcab_info() failed with ret=0x%08X\r\n", status);
         break;
      }
      displaylen = sizeof(displaystr);
      atcab_bin2hex(serial_number, sizeof(serial_number), displaystr, &displaylen);
      printf("\nDevice serial number: %s\r\n\r\n", displaystr);

      //Selecting the right device based on the revision
      device_type = get_device_type_id(revision[2]);
      if (cfg->devtype != device_type)
      {
         printf("Selected device id: %d, Identified device id: %d\r\n", cfg->devtype, device_type);
         if (overwrite)
         {
            printf("Device type updated!\r\n");
            cfg->devtype = device_type;
         }
         else
         {
            printf("Selected device is not found on the bus!\r\n");
            status = ATCA_GEN_FAIL;
         }
      }
   } while (0);

   (void)atcab_release();

   return status;
}