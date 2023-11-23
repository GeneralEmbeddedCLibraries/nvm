// Copyright (c) 2023 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      nvm.c
*@brief     Non-Volatile memory
*@author    Ziga Miklosic
*@date      15.02.2023
*@version	V2.1.0
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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "nvm.h"
#include "nvm_ee.h"

// Interface
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
 * 	Pointer to NVM configuration tables
 */
static const nvm_region_t *     gp_nvm_regions = NULL;
static const nvm_mem_driver_t * gp_nvm_drivers = NULL;

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
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
static nvm_status_t nvm_check_config(void);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*		Check for NVM valid configuration
*
* @return 	status - Status of configuration
*/
////////////////////////////////////////////////////////////////////////////////
static nvm_status_t nvm_check_config(void)
{
    nvm_status_t status = eNVM_OK;
    
    // Check all memory drivers are configured OK
    for ( uint32_t mem_drv = 0U; mem_drv < eNVM_MEM_DRV_NUM_OF; mem_drv++)
    {   
        // All low level interfaces must be defined
        if  (   ( NULL == gp_nvm_drivers[mem_drv].pf_nvm_init   )
            ||  ( NULL == gp_nvm_drivers[mem_drv].pf_nvm_deinit )
            ||  ( NULL == gp_nvm_drivers[mem_drv].pf_nvm_read   )
            ||  ( NULL == gp_nvm_drivers[mem_drv].pf_nvm_write  )
            ||  ( NULL == gp_nvm_drivers[mem_drv].pf_nvm_erase  ))
        {
            status = eNVM_ERROR;
            break;
        }
    }
    
    // Check all regions are configuraed OK
    for ( uint32_t reg_idx = 0U; reg_idx < eNVM_REGION_NUM_OF; reg_idx++)
    {
        if  (   ( NULL == gp_nvm_regions[reg_idx].name )
            ||  ( NULL == gp_nvm_regions[reg_idx].p_driver )
            ||  ( 0U == gp_nvm_regions[reg_idx].size ))
        {
            status = eNVM_ERROR;
            break;        
        }
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup NVM_API
* @{ <!-- BEGIN GROUP -->
*
* 	Following function are part of NVM API.
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*		Initialized NVM regions
*
* @return 	status - Status of initialization
*/
////////////////////////////////////////////////////////////////////////////////
nvm_status_t nvm_init(void)
{
	nvm_status_t status = eNVM_OK;

	if ( false == gb_is_init )
	{
		// Get table configuration
		gp_nvm_regions = nvm_cfg_get_regions();
		gp_nvm_drivers = nvm_cfg_get_drivers();
		NVM_ASSERT( NULL != gp_nvm_regions );
		NVM_ASSERT( NULL != gp_nvm_drivers );

        // Check for valid configuration
        if ( eNVM_OK == nvm_check_config())
        {
    		// Low level driver init
    		for ( uint32_t mem_drv_num = 0; mem_drv_num < eNVM_MEM_DRV_NUM_OF; mem_drv_num++ )
    		{
                // Init low level memory driver
                status |= gp_nvm_drivers[mem_drv_num].pf_nvm_init();

                NVM_DBG_PRINT( "NVM: Low level memory driver #%d initialize with status: %s", mem_drv_num, nvm_get_status_str( status ));
    		}

            // Init NVM EEPROM Emulation
            status |= nvm_ee_init();

    		// Init NVM interface
    		status |= nvm_if_init();

    		// Init success
    		if ( eNVM_OK == status )
    		{
    			gb_is_init = true;
    		}
        }

        // Invalid NVM configuration
        else
        {
            status = eNVM_ERROR;

            NVM_DBG_PRINT( "NVM: Configuration invalid!" );
            NVM_ASSERT( 0 );
        }
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		De-Initialized NVM memory drivers
*
*
* @return 	status - Status of initialization
*/
////////////////////////////////////////////////////////////////////////////////
nvm_status_t nvm_deinit(void)
{
    nvm_status_t status = eNVM_OK;
    
    if ( true == gb_is_init )
    {
        // Low level driver de-init
        for ( uint32_t mem_drv_num = 0; mem_drv_num < eNVM_MEM_DRV_NUM_OF; mem_drv_num++ )
        {
            if ( NULL != gp_nvm_regions[mem_drv_num].p_driver->pf_nvm_deinit )
            {
                status |= gp_nvm_regions[mem_drv_num].p_driver->pf_nvm_deinit();

                NVM_DBG_PRINT( "NVM: Low level memory driver #%d de-initialize with status: %s", mem_drv_num, nvm_get_status_str( status ));
            }
        } 

        // De-init success
        if ( eNVM_OK == status )
        {
            gb_is_init = false;
        }
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get NVM initialization state
*
* @param[out]   p_is_init   - Pointer to init flag
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
nvm_status_t nvm_is_init(bool * const p_is_init)
{
    nvm_status_t status = eNVM_OK;

    if ( NULL != p_is_init )
    {
        *p_is_init = gb_is_init;
    }
    else
    {
        status = eNVM_ERROR;
    }

    return status;
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

	NVM_ASSERT( true == gb_is_init );
	NVM_ASSERT( region < eNVM_REGION_NUM_OF );
	NVM_ASSERT(		(( addr + gp_nvm_regions[region].start_addr ) < ( gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))
				&& 	( size <= gp_nvm_regions[region].size ));

    // Is init and valid range
	if  (   ( true == gb_is_init )
        &&  ( region < eNVM_REGION_NUM_OF  ))
	{
		// Valid address and size
		if (    (( addr + gp_nvm_regions[region].start_addr ) < ( gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))
            && 	( size <= gp_nvm_regions[region].size ))
		{
			#if ( 1 == NVM_CFG_MUTEX_EN )
				if ( eNVM_OK == nvm_if_aquire_mutex())
				{
			#endif
                    // EEPROM emulated region
                    if ( true == gp_nvm_regions[region].p_driver->ee_en )
                    {
                        status = nvm_ee_write( region, addr, size, p_data );
                    }

                    // Simple write
                    else
                    {
    					// Write
    					if ( eNVM_OK != gp_nvm_regions[region].p_driver->pf_nvm_write( gp_nvm_regions[region].start_addr + addr, size, p_data ))
    					{
    						status = eNVM_ERROR;
    					}
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

	NVM_DBG_PRINT( "NVM: Writing to region <%d> addr: 0x%04X. Status: %s", region, addr, nvm_get_status_str( status ));

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

	NVM_ASSERT( true == gb_is_init );
	NVM_ASSERT( region < eNVM_REGION_NUM_OF );
	NVM_ASSERT(		(( addr + gp_nvm_regions[region].start_addr ) < ( gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))
				&& 	( size <= gp_nvm_regions[region].size ));

    // Is init and valid range
	if  (   ( true == gb_is_init )
        &&  ( region < eNVM_REGION_NUM_OF  ))
	{
		// Valid address and size
		if (    (( addr + gp_nvm_regions[region].start_addr ) < ( gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))
            && 	( size <= gp_nvm_regions[region].size ))
		{
			#if ( 1 == NVM_CFG_MUTEX_EN )
				if ( eNVM_OK == nvm_if_aquire_mutex())
				{
			#endif
                    // EEPROM emulated region
                    if ( true == gp_nvm_regions[region].p_driver->ee_en )
                    {
                        status = nvm_ee_read( region, addr, size, p_data );
                    }

                    // Simple read
                    else
                    {
    					// Read
    					if ( eNVM_OK != gp_nvm_regions[region].p_driver->pf_nvm_read( gp_nvm_regions[region].start_addr + addr, size, p_data ))
    					{
    						status = eNVM_ERROR;
    					}
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

	NVM_DBG_PRINT( "NVM: Reading region <%d> from addr: 0x%04X. Status: %s", region, addr, nvm_get_status_str( status ));

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

	NVM_ASSERT( true == gb_is_init );
	NVM_ASSERT( region < eNVM_REGION_NUM_OF );
	NVM_ASSERT(		(( addr + gp_nvm_regions[region].start_addr ) < ( gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))
				&& 	( size <= gp_nvm_regions[region].size ));

    // Is init and valid range
	if  (   ( true == gb_is_init )
        &&  ( region < eNVM_REGION_NUM_OF  ))
	{
		// Valid address and size
		if (    (( addr + gp_nvm_regions[region].start_addr ) < ( gp_nvm_regions[region].start_addr + gp_nvm_regions[region].size ))
            && 	( size <= gp_nvm_regions[region].size ))
		{
			#if ( 1 == NVM_CFG_MUTEX_EN )
				if ( eNVM_OK == nvm_if_aquire_mutex())
				{
			#endif
					
                    // EEPROM emulated region
                    if ( true == gp_nvm_regions[region].p_driver->ee_en )
                    {
                        status = nvm_ee_erase( region, addr, size );
                    }

                    // Simple erase
                    else
                    {
                        // Erase
    					if ( eNVM_OK != gp_nvm_regions[region].p_driver->pf_nvm_erase( gp_nvm_regions[region].start_addr + addr, size ))
    					{
    						status = eNVM_ERROR;
    					}
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

	NVM_DBG_PRINT( "NVM: Erasing from region <%d> addr: 0x%04X. Status: %s", region, addr, nvm_get_status_str( status ));

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Sync NVM region
*
* @brief    When region is using EEPROM emulated memory driver this function
*           moved data from RAM to FLASH
*
* @param[in]	region	- NVM region defined in config table
* @return 		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
nvm_status_t nvm_sync(const nvm_region_name_t region)
{
	nvm_status_t status = eNVM_OK;

	NVM_ASSERT( true == gb_is_init );
    NVM_ASSERT( region < eNVM_REGION_NUM_OF );

	// Check init
	if  (   ( true == gb_is_init )
        &&  ( region < eNVM_REGION_NUM_OF ))
	{
        #if ( 1 == NVM_CFG_MUTEX_EN )
            if ( eNVM_OK == nvm_if_aquire_mutex())
            {
        #endif
                
        // Sync local RAM data to FLASH memory
        status = nvm_ee_sync( region );

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

	NVM_DBG_PRINT( "NVM: Sync region <%d> status: %s", region, nvm_get_status_str( status ));

	return status;    
}

#if ( 1 == NVM_CFG_DEBUG_EN )

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
