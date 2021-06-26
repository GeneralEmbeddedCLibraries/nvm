# NVM - Non Volatile Memory module
Non-volatile memory storage module, responsible for storing parameters.

NVM module is used for storage into persistant memory and it is divided into regions specified as a part of configuration table. Complete module is being configurable inside **nvm_cfg.c/.h** files. 

## Dependencies
Definition of float32_t must be provided by user. In current implementation it is defined in "project_config.h". Just add following statement to your code where it suits the best.

```C
// Define float
typedef float float32_t;
```

Additionaly module uses "static_assert()" function defined in <assert.h>.


## API
 - nvm_status_t 	**nvm_init**	(void);
 - bool			**nvm_is_init**	(void);
 - nvm_status_t 	**nvm_write**	(const nvm_region_name_t region, const  - uint32_t addr, const uint32_t size, const uint8_t * const p_data);
 - nvm_status_t 	**nvm_read**	(const nvm_region_name_t region, const uint32_t addr, const uint32_t size, uint8_t * const p_data);
 - nvm_status_t 	**nvm_erase**	(const nvm_region_name_t region, const uint32_t addr, const uint32_t size);

## Usage

**Put all user code between sections: USER CODE BEGIN & USER CODE END!**

1. Copy template files to root directory of module.
2. List all NVM regions and memory drivers inside **nvm_cfg.h** file as enumerations

```C
/**
 * 	NVM Region options
 *
 * 	@note 	Must always start with 0!
 */
typedef enum
{
	// USER CODE BEGIN...

	eNVM_REGION_EEPROM_RUN_PAR = 0,

	// USER CODE END...

	eNVM_REGION_NUM_OF
} nvm_region_name_t;

/**
 * 	NVM Low-level memory drivers
 *
 * 	@note 	Must always start with 0!
 */
typedef enum
{
	// USER CODE BEGIN...

	eNVM_MEM_DRV_EEPROM = 0,

	// User shall add more here if needed...

	// USER CODE END...

	eNVM_MEM_DRV_NUM_OF
} nvm_mem_drv_name_t;
```

3. Connect NVM memory driver with an wanted device driver

```C
/**
 * 	NVM low-level memory driver
 *
 * 	@brief	User shall specified low level memory drivers API functions
 * 			for defined memory driver.
 *
 * 			Each driver expect four functions: init, write, read and erase with
 * 			exact predefined function prototypes.
 *
 * 	@note	It is important that low level driver return status has the same
 * 			interface. Rule is that 0 means OK, non-zero values means error
 * 			codes. For know NVM module is written that detects only
 * 			error code by checking for 0 return code as enumeration type.
 *
 *
 */
static const nvm_mem_driver_t g_mem_driver[ eNVM_MEM_DRV_NUM_OF ]=
{
	// USER CODE BEGIN...

	// EEPROM LOW LEVEL MEMORY DRIVERS
	{
		(nvm_status_t (*)(void))																	_25lcxxxx_init,
		(nvm_status_t (*)(const uint32_t addr, const uint32_t size, const uint8_t * const p_data)) 	_25lcxxxx_write,
		(nvm_status_t (*)(const uint32_t addr, const uint32_t size, uint8_t * const p_data))		_25lcxxxx_read,
		(nvm_status_t (*)(const uint32_t addr, const uint32_t size))								_25lcxxxx_erase,
	},

	// User shall add more here if needed...


	// USER CODE END...
};
```
4. Add NVM region details

```C
/**
 * 		NVM region definitions
 *
 *	@brief	User shall specified NVM regions name, start, size
 *			and pointer to low level driver.
 *
 * 	@note	Special care with start address and its size!
 */
static const nvm_region_t g_nvm_region[ eNVM_REGION_NUM_OF ] =
{
	// USER CODE BEGIN...

	// -----------------------------------------------------------------------------------------------------------------------------------------------
	// 	Region Name						Start address			Size [byte]			Low level driver
	// -----------------------------------------------------------------------------------------------------------------------------------------------

	{	.name = "device parameters",	.start_addr = 0x0,		.size = 1024,		.p_driver = &g_mem_driver[ eNVM_MEM_DRV_EEPROM ]				},

	// -----------------------------------------------------------------------------------------------------------------------------------------------

	// User shall add more here if needed...

	// USER CODE END...
};
```

5. Initialize module

```C
if ( eNVM_OK != nvm_init())
{
    PROJECT_CONFIG_ASSERT( 0 );
}
```

6. Write/Read/Erase from/to specified NVM region

```C

// Write example
if ( eNVM_OK != nvm_write( eNVM_REGION_EEPROM_RUN_PAR, 0x100, 10U, (const uint8_t*) &u8_write_data))
{
    // NVM write error...
    // Further actions here...
}

// Read example
par_status_t status 	= ePAR_OK;
uint32_t     sign	= 0UL;

if ( eNVM_OK != nvm_read( eNVM_REGION_EEPROM_RUN_PAR, PAR_NVM_SIGNATURE_ADDR_OFFSET, 4U, (uint8_t*) &sign ))
{
    // NVM read error...
    // Further actions here...
}

// Erase example
nvm_erase( eNVM_REGION_EEPROM_RUN_PAR, 0x123, 4U );

```

