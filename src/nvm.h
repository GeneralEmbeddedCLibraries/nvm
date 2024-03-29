// Copyright (c) 2023 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      nvm.h
*@brief     Non-Volatile memory
*@author    Ziga Miklosic
*@date      15.02.2023
*@version	V2.1.0
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup NVM
* @{ <!-- BEGIN GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

#ifndef _NVM_H_
#define _NVM_H_

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../nvm_cfg.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 * 	Module version
 */
#define NVM_VER_MAJOR		( 2 )
#define NVM_VER_MINOR		( 1 )
#define NVM_VER_DEVELOP		( 0 )

/**
 * 	Status
 */
typedef enum
{
	eNVM_OK 	= 0,		/**<Normal operation */
	eNVM_ERROR	= 0x01,		/**<General error */
} nvm_status_t;

/**
 * 	Memory device driver
 */
typedef struct nvm_mem_driver_s
{
	nvm_status_t (*pf_nvm_init)		(void);                                                                     /**<Initialize low level interface pointer function */
	nvm_status_t (*pf_nvm_deinit)	(void);                                                                     /**<De-initialize low level interface pointer function */
	nvm_status_t (*pf_nvm_write)	(const uint32_t addr, const uint32_t size, const uint8_t * const p_data);   /**<Write low level interface pointer function */
	nvm_status_t (*pf_nvm_read)		(const uint32_t addr, const uint32_t size, uint8_t * const p_data);         /**<Read low level interface pointer function */
	nvm_status_t (*pf_nvm_erase)	(const uint32_t addr, const uint32_t size);                                 /**<Erase low level interface pointer function */
    bool ee_en;                                                                                                 /**<Enable/Disable EEPROM emulation switch */
} nvm_mem_driver_t;

/**
 * 	Memory region
 */
typedef struct nvm_region_s
{
	const char *				name;			/**<Name of region */
	const uint32_t 				start_addr;		/**<Start address of region */
	const uint32_t 				size;			/**<Size of region in bytes */
	const nvm_mem_driver_t *	p_driver;		/**<Low level memory driver */
} nvm_region_t;

////////////////////////////////////////////////////////////////////////////////
// Functions Prototypes
////////////////////////////////////////////////////////////////////////////////
nvm_status_t 	nvm_init	(void);
nvm_status_t    nvm_deinit  (void);
nvm_status_t    nvm_is_init	(bool * const p_is_init);
nvm_status_t 	nvm_write	(const nvm_region_name_t region, const uint32_t addr, const uint32_t size, const uint8_t * const p_data);
nvm_status_t 	nvm_read	(const nvm_region_name_t region, const uint32_t addr, const uint32_t size, uint8_t * const p_data);
nvm_status_t 	nvm_erase	(const nvm_region_name_t region, const uint32_t addr, const uint32_t size);
nvm_status_t    nvm_sync    (const nvm_region_name_t region);

#if ( NVM_CFG_DEBUG_EN )
	const char * nvm_get_status_str		(const nvm_status_t status);
#endif

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
#endif // _NVM_H_
