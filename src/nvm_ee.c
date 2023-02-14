// Copyright (c) 2023 Ziga Miklosic
// All Rights Reserved
////////////////////////////////////////////////////////////////////////////////
/**
*@file      nvm_ee.c
*@brief     NVM EEPROM Emulation
*@author    Ziga Miklosic
*@email		ziga.miklosic@gmail.com
*@date      14.02.2023
*@version   V2.0.0
*/
////////////////////////////////////////////////////////////////////////////////
/*!
* @addtogroup NVM_EE
* @{ <!-- BEGIN GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "nvm_ee.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/**
 *  Initialization guard
 */
static bool gb_is_init = false;

/**
 * 	Pointer to NVM configuration tables
 */
static const nvm_region_t *     gp_nvm_regions = NULL;
static const nvm_mem_driver_t * gp_nvm_drivers = NULL;

/**
 *  RAM space as intermediate memory for Flash
 */
static uint8_t * gp_ram_mem = NULL;


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
static nvm_status_t nvm_ee_copy_ram_to_flash    (void);
static nvm_status_t nvm_ee_copy_flash_to_ram    (void);


////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////


static nvm_status_t nvm_ee_copy_ram_to_flash(void)
{
    nvm_status_t    status      = eNVM_OK;
    uint32_t        ram_offset  = 0U;

    // Re-write complete RAM space
    for (uint32_t region = 0U; region < eNVM_REGION_NUM_OF; region++)
    {
        // Check if EEPROM emulation is enabled
        if ( true == gp_nvm_regions[region].p_driver->ee.en )
        {
            // Write complete NVM region
            if ( eNVM_OK != gp_nvm_regions[region].p_driver->pf_nvm_write( gp_nvm_regions[region].start_addr, gp_nvm_regions[region].size, (const uint8_t*) &gp_ram_mem[ram_offset] ))
            {
                status = eNVM_ERROR;
            }

            // Increment RAM offset by region size
            ram_offset = ( ram_offset + gp_nvm_regions[region].size );
        }
    }

    return status;
}



static nvm_status_t nvm_ee_copy_flash_to_ram(void)
{
    nvm_status_t    status      = eNVM_OK;
    uint32_t        ram_offset  = 0U;

    // Re-write complete RAM space
    for (uint32_t region = 0U; region < eNVM_REGION_NUM_OF; region++)
    {
        // Check if EEPROM emulation is enabled
        if ( true == gp_nvm_regions[region].p_driver->ee.en )
        {
            // Read complete NVM region
            if ( eNVM_OK != gp_nvm_regions[region].p_driver->pf_nvm_read( gp_nvm_regions[region].start_addr, gp_nvm_regions[region].size, (uint8_t*) &gp_ram_mem[ram_offset] ))
            {
                status = eNVM_ERROR;
            }

            // Increment RAM offset by region size
            ram_offset = ( ram_offset + gp_nvm_regions[region].size );
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
*@addtogroup NVM_EE_API
* @{ <!-- BEGIN GROUP -->
*
* 	Following function are part of NVM EEPROM Emulation API.
*/
////////////////////////////////////////////////////////////////////////////////





nvm_status_t nvm_ee_init(void)
{
    nvm_status_t    status      = eNVM_OK;
    uint32_t        ram_space   = 0U;

    if ( false == gb_is_init )
    {
        // Get table configuration
        gp_nvm_regions = nvm_cfg_get_regions();
        gp_nvm_drivers = nvm_cfg_get_drivers();
    
        // Count regions which wants to emulate eeprom
        for (uint32_t region = 0U; region < eNVM_REGION_NUM_OF; region++)
        {
            // Check if EEPROM emulation is enabled
            if ( true == gp_nvm_regions[region].p_driver->ee.en )
            {
                // Accumulate all RAM space needed to contain Flash memory
                ram_space += gp_nvm_regions[region].size;
            }
        }

        // Allocate RAM space
        gp_ram_mem = malloc( ram_space );

        // Allocation success?
        if ( NULL == gp_ram_mem )
        {
            status = eNVM_ERROR;
        }
        else
        {
            // Copy all content from Flash to RAM
            status = nvm_ee_copy_flash_to_ram();
        }

        if ( eNVM_OK == status )
        {
            gb_is_init = true;
        }
    }

    return status;
}


nvm_status_t nvm_ee_write(const nvm_region_name_t region, const uint32_t addr, const uint32_t size, const uint8_t * const p_data)
{
    nvm_status_t status = eNVM_OK;
    
    NVM_ASSERT( true == gb_is_init );

    if ( true == gb_is_init )
    {
        // Calculate RAM offset
        const uint32_t ram_offset = (uint32_t)( addr - gp_nvm_regions[region].start_addr );

        // First copy data to RAM space
        memcpy( &gp_ram_mem[ram_offset], p_data, size );

        // Erase flash page
        if ( eNVM_OK != gp_nvm_regions[region].p_driver->pf_nvm_erase( gp_nvm_regions[region].start_addr, size ))
        {
            status = eNVM_ERROR;
        }

        // Copy from RAM to Flash
        status |= nvm_ee_copy_ram_to_flash();
    }
    else
    {
        status = eNVM_ERROR;
    }

    return status;
}


nvm_status_t nvm_ee_read(const nvm_region_name_t region, const uint32_t addr, const uint32_t size, uint8_t * const p_data)
{
    nvm_status_t status = eNVM_OK;

    NVM_ASSERT( true == gb_is_init );

    if ( true == gb_is_init )
    {
        // Calculate RAM offset
        const uint32_t ram_offset = (uint32_t)( addr - gp_nvm_regions[region].start_addr );

        // Read only from local RAM
        memcpy( p_data, &gp_ram_mem[ram_offset], size );
    }
    else
    {
        status = eNVM_ERROR;
    }

    return status;
}


nvm_status_t nvm_ee_erase(const nvm_region_name_t region, const uint32_t addr, const uint32_t size)
{
    nvm_status_t status = eNVM_OK;

    NVM_ASSERT( true == gb_is_init );

    if ( true == gb_is_init )
    {
        // Erase flash page
        if ( eNVM_OK != gp_nvm_regions[region].p_driver->pf_nvm_erase( addr, size ))
        {
            status = eNVM_ERROR;
        }

        // Copy all content from Flash to RAM
        status = nvm_ee_copy_flash_to_ram();
    }
    else
    {
        status = eNVM_ERROR;
    }

    return status;
}



////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
