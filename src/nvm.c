// Copyright (c) 2021 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      nvm.c
*@brief     Non-Volatile memory
*@author    Ziga Miklosic
*@date      04.06.2021
*@version	V1.0.1
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup NVM
* @{ <!-- BEGIN GROUP -->
*
* 	Non-volatile memory API functions definition.
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "nvm.h"
#include "../../nvm_if.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/**
 * 	Initialization guard
 */
static bool gb_is_init = false;

/**
 * 	Pointer to NVM
 */
static const nvm_region_t * gp_nvm_regions = NULL;

#if ( NVM_CFG_DEBUG_EN )

	/**
	 * 	Status strings
	 */
	static const char * gs_status[] =
	{
		"OK",
		"ERROR",
	};
#endif

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*		Initialized NVM regions
*
* @brief	Low level memory interface drivers are being initialized.
*
* @return 	status - Status of initialization
*/
////////////////////////////////////////////////////////////////////////////////
nvm_status_t nvm_init(void)
{
	nvm_status_t 	status 		= eNVM_OK;
	uint8_t			mem_drv_num	= 0U;

	if ( false == gb_is_init )
	{
		// Get table configuration
		gp_nvm_regions = nvm_cfg_get_regions();
		NVM_ASSERT( NULL != gp_nvm_regions );

		// Low level driver init
		for ( mem_drv_num = 0; mem_drv_num < eNVM_MEM_DRV_NUM_OF; mem_drv_num++ )
		{
			if ( NULL != gp_nvm_regions[mem_drv_num].p_driver->pf_nvm_init )
			{
				status |= gp_nvm_regions[mem_drv_num].p_driver->pf_nvm_init();

				NVM_DBG_PRINT( "NVM: Low level memory driver #%d initialize with status: %s", mem_drv_num, nvm_get_status_str( status ));
			}
		}

		// Init NVM interface
		status |= nvm_if_init();

		// Init success
		if ( eNVM_OK == status )
		{
			gb_is_init = true;
		}
	}
	else
	{
		status = eNVM_ERROR;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get NVM initialization state
*
* @return 	gb_is_init - Initialization flag
*/
////////////////////////////////////////////////////////////////////////////////
bool nvm_is_init(void)
{
	return gb_is_init;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Write data to NVM region
*
* @note		Input address argument of write function in NVM region is
* 			offset by defined start address of region!
*
* @param[in]	region	- NVM region defined in config table
* @param[in]	addr	- Start region address + address
* @param[in]	size	- Size of written data in bytes
* @param[in]	p_data	- Pointer to written data
* @return 		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
nvm_status_t nvm_write(const nvm_region_name_t region, const uint32_t addr, const uint32_t size, const uint8_t * const p_data)
{
	nvm_status_t status = eNVM_OK;

	// Is init
	NVM_ASSERT( true == gb_is_init );

	// Check inputs
	NVM_ASSERT( region < eNVM_REGION_NUM_OF );
	NVM_ASSERT(		(( addr + gp_nvm_regions[region].start_addr ) < ( gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))
				&& 	(( addr + gp_nvm_regions[region].start_addr + size ) < ( addr + gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size )));

	// Check init
	if ( true == gb_is_init )
	{
		// Check valid input
		if ( 	( region < eNVM_REGION_NUM_OF )
				&&	( 	(( addr + gp_nvm_regions[region].start_addr ) < ( gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))
					&& 	(( addr + gp_nvm_regions[region].start_addr + size ) < ( addr + gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))))
		{
			#if ( 1 == NVM_CFG_MUTEX_EN )
				if ( eNVM_OK == nvm_if_aquire_mutex())
				{
			#endif
					// Write
					if ( eNVM_OK != gp_nvm_regions[region].p_driver->pf_nvm_write( gp_nvm_regions[region].start_addr + addr, size, p_data ))
					{
						status = eNVM_ERROR;
					}

			#if ( 1 == NVM_CFG_MUTEX_EN )
					nvm_if_release_mutex();
				}

				// Mutex not acquire
				else
				{
					status = eNVM_ERROR;
				}
			#endif
		}
		else
		{
			status = eNVM_ERROR;
		}
	}
	else
	{
		status = eNVM_ERROR;
	}

	NVM_DBG_PRINT( "NVM: Writing to addr: 0x%04X. Status: %s", addr, nvm_get_status_str( status ));

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Read data from NVM region
*
* @note		Input address argument of write function in NVM region is
* 			offset by defined start address of region!
*
* @param[in]	region	- NVM region defined in config table
* @param[in]	addr	- Start region address + address
* @param[in]	size	- Size of written data in bytes
* @param[in]	p_data	- Pointer to read data
* @return 		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
nvm_status_t nvm_read(const nvm_region_name_t region, const uint32_t addr, const uint32_t size, uint8_t * const p_data)
{
	nvm_status_t status = eNVM_OK;

	// Is init
	NVM_ASSERT( true == gb_is_init );

	// Check inputs
	NVM_ASSERT( region < eNVM_REGION_NUM_OF );
	NVM_ASSERT(		(( addr + gp_nvm_regions[region].start_addr ) < ( gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))
				&& 	(( addr + gp_nvm_regions[region].start_addr + size ) < ( addr + gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size )));

	// Check init
	if ( true == gb_is_init )
	{
		// Check valid input
		if ( 	( region < eNVM_REGION_NUM_OF )
				&&	( 	(( addr + gp_nvm_regions[region].start_addr ) < ( gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))
					&& 	(( addr + gp_nvm_regions[region].start_addr + size ) < ( addr + gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))))
		{
			#if ( 1 == NVM_CFG_MUTEX_EN )
				if ( eNVM_OK == nvm_if_aquire_mutex())
				{
			#endif
					// Read
					if ( eNVM_OK != gp_nvm_regions[region].p_driver->pf_nvm_read( gp_nvm_regions[region].start_addr + addr, size, p_data ))
					{
						status = eNVM_ERROR;
					}

			#if ( 1 == NVM_CFG_MUTEX_EN )
					nvm_if_release_mutex();
				}

				// Mutex not acquire
				else
				{
					status = eNVM_ERROR;
				}
			#endif
		}
		else
		{
			status = eNVM_ERROR;
		}
	}
	else
	{
		status = eNVM_ERROR;
	}

	NVM_DBG_PRINT( "NVM: Reading from addr: 0x%04X. Status: %s", addr, nvm_get_status_str( status ));

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Erase data from NVM region
*
* @note		Input address argument of write function in NVM region is
* 			offset by defined start address of region!
*
* @param[in]	region	- NVM region defined in config table
* @param[in]	addr	- Start region address + address
* @param[in]	size	- Size of written data in bytes
* @return 		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
nvm_status_t nvm_erase(const nvm_region_name_t region, const uint32_t addr, const uint32_t size)
{
	nvm_status_t status = eNVM_OK;

	// Is init
	NVM_ASSERT( true == gb_is_init );

	// Check inputs
	NVM_ASSERT( region < eNVM_REGION_NUM_OF );
	NVM_ASSERT(		(( addr + gp_nvm_regions[region].start_addr ) < ( gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))
				&& 	(( addr + gp_nvm_regions[region].start_addr + size ) < ( addr + gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size )));

	// Check init
	if ( true == gb_is_init )
	{
		// Check valid input
		if ( 	( region < eNVM_REGION_NUM_OF )
				&&	( 	(( addr + gp_nvm_regions[region].start_addr ) < ( gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))
					&& 	(( addr + gp_nvm_regions[region].start_addr + size ) < ( addr + gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))))
		{
			#if ( 1 == NVM_CFG_MUTEX_EN )
				if ( eNVM_OK == nvm_if_aquire_mutex())
				{
			#endif
					// Erase
					if ( eNVM_OK != gp_nvm_regions[region].p_driver->pf_nvm_erase( gp_nvm_regions[region].start_addr + addr, size ))
					{
						status = eNVM_ERROR;
					}

			#if ( 1 == NVM_CFG_MUTEX_EN )
					nvm_if_release_mutex();
				}

				// Mutex not acquire
				else
				{
					status = eNVM_ERROR;
				}
			#endif
		}
		else
		{
			status = eNVM_ERROR;
		}
	}
	else
	{
		status = eNVM_ERROR;
	}

	NVM_DBG_PRINT( "NVM: Erasing from addr: 0x%04X. Status: %s", addr, nvm_get_status_str( status ));

	return status;
}

#if ( NVM_CFG_DEBUG_EN )

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Get status string description
	*
	* @param[in]	status	- NVM status
	* @return		str		- NVM status description
	*/
	////////////////////////////////////////////////////////////////////////////////
	const char * nvm_get_status_str(const nvm_status_t status)
	{
		uint8_t i = 0;
		const char * str = "N/A";

		if ( eNVM_OK == status  )
		{
			str = (const char*) gs_status[0];
		}
		else
		{
			for ( i = 0; i < 8; i++ )
			{
				if ( status & ( 1<<i ))
				{
					str =  (const char*) gs_status[i+1];
					break;
				}
			}
		}

		return str;
	}
#endif

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
