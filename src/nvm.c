////////////////////////////////////////////////////////////////////////////////
/**
*@file      nvm.c
*@brief     Non-Volatile memory
*@author    Ziga Miklosic
*@date      04.06.2021
*@version	V1.0.0
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
	nvm_status_t 	status 	= eNVM_OK;
	uint8_t			reg_num	= 0U;

	// Get table configuration
	gp_nvm_regions = nvm_cfg_get_regions();
	NVM_ASSERT( NULL != gp_nvm_regions );

	// Low level driver init
	for ( reg_num = 0; reg_num < eNVM_REGION_NUM_OF; reg_num++ )
	{
		if ( NULL != gp_nvm_regions[reg_num].p_driver->pf_nvm_init )
		{
			status |= gp_nvm_regions[reg_num].p_driver->pf_nvm_init();

			NVM_DBG_PRINT( "NVM: Region <%s> initialize with status: %d", gp_nvm_regions[reg_num].name, status );
		}
	}

	// Init NVM interface
	status |= nvm_if_init();

	NVM_ASSERT( eNVM_OK == status );

	// Init done
	gb_is_init = true;

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

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
