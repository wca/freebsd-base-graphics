/*-
 * Copyright (c) 2015 Landon Fuller <landon@landonf.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 * 
 * $FreeBSD$
 */

#ifndef _BHND_BHND_H_
#define _BHND_BHND_H_

#include <sys/types.h>
#include <sys/bus.h>

#include <machine/bus.h>

#include "bhnd_ids.h"
#include "bhnd_types.h"
#include "bhnd_debug.h"
#include "bhnd_bus_if.h"

extern devclass_t bhnd_devclass;
extern devclass_t bhnd_hostb_devclass;
extern devclass_t bhnd_nvram_devclass;

/**
 * bhnd child instance variables
 */
enum bhnd_device_vars {
	BHND_IVAR_VENDOR,	/**< Designer's JEP-106 manufacturer ID. */
	BHND_IVAR_DEVICE,	/**< Part number */
	BHND_IVAR_HWREV,	/**< Core revision */
	BHND_IVAR_DEVICE_CLASS,	/**< Core class (@sa bhnd_devclass_t) */
	BHND_IVAR_VENDOR_NAME,	/**< Core vendor name */
	BHND_IVAR_DEVICE_NAME,	/**< Core name */
	BHND_IVAR_CORE_INDEX,	/**< Bus-assigned core number */
	BHND_IVAR_CORE_UNIT,	/**< Bus-assigned core unit number,
				     assigned sequentially (starting at 0) for
				     each vendor/device pair. */
};

/**
 * bhnd device probe priority bands.
 */
enum {
	BHND_PROBE_ROOT         = 0,    /**< Nexus or host bridge */
	BHND_PROBE_BUS		= 1000,	/**< Busses and bridges */
	BHND_PROBE_CPU		= 2000,	/**< CPU devices */
	BHND_PROBE_INTERRUPT	= 3000,	/**< Interrupt controllers. */
	BHND_PROBE_TIMER	= 4000,	/**< Timers and clocks. */
	BHND_PROBE_RESOURCE	= 5000,	/**< Resource discovery (including NVRAM/SPROM) */
	BHND_PROBE_DEFAULT	= 6000,	/**< Default device priority */
};

/**
 * Constants defining fine grained ordering within a BHND_PROBE_* priority band.
 * 
 * Example:
 * @code
 * BHND_PROBE_BUS + BHND_PROBE_ORDER_FIRST
 * @endcode
 */
enum {
	BHND_PROBE_ORDER_FIRST		= 0,
	BHND_PROBE_ORDER_EARLY		= 25,
	BHND_PROBE_ORDER_MIDDLE		= 50,
	BHND_PROBE_ORDER_LATE		= 75,
	BHND_PROBE_ORDER_LAST		= 100

};

/*
 * Simplified accessors for bhnd device ivars
 */
#define	BHND_ACCESSOR(var, ivar, type) \
	__BUS_ACCESSOR(bhnd, var, BHND, ivar, type)

BHND_ACCESSOR(vendor,		VENDOR,		uint16_t);
BHND_ACCESSOR(device,		DEVICE,		uint16_t);
BHND_ACCESSOR(hwrev,		HWREV,		uint8_t);
BHND_ACCESSOR(class,		DEVICE_CLASS,	bhnd_devclass_t);
BHND_ACCESSOR(vendor_name,	VENDOR_NAME,	const char *);
BHND_ACCESSOR(device_name,	DEVICE_NAME,	const char *);
BHND_ACCESSOR(core_index,	CORE_INDEX,	u_int);
BHND_ACCESSOR(core_unit,	CORE_UNIT,	int);

#undef	BHND_ACCESSOR

/**
 * A bhnd(4) board descriptor.
 */
struct bhnd_board_info {
	uint16_t	board_vendor;	/**< PCI-SIG vendor ID (even on non-PCI
					  *  devices).
					  *
					  *  On PCI devices, this will generally
					  *  be the subsystem vendor ID, but the
					  *  value may be overridden in device
					  *  NVRAM.
					  */
	uint16_t	board_type;	/**< Board type (See BHND_BOARD_*)
					  *
					  *  On PCI devices, this will generally
					  *  be the subsystem device ID, but the
					  *  value may be overridden in device
					  *  NVRAM.
					  */
	uint16_t	board_rev;	/**< Board revision. */
	uint8_t		board_srom_rev;	/**< Board SROM format revision */

	uint32_t	board_flags;	/**< Board flags (see BHND_BFL_*) */
	uint32_t	board_flags2;	/**< Board flags 2 (see BHND_BFL2_*) */
	uint32_t	board_flags3;	/**< Board flags 3 (see BHND_BFL3_*) */
};


/**
 * Chip Identification
 * 
 * This is read from the ChipCommon ID register; on earlier bhnd(4) devices
 * where ChipCommon is unavailable, known values must be supplied.
 */
struct bhnd_chipid {
	uint16_t	chip_id;	/**< chip id (BHND_CHIPID_*) */
	uint8_t		chip_rev;	/**< chip revision */
	uint8_t		chip_pkg;	/**< chip package (BHND_PKGID_*) */
	uint8_t		chip_type;	/**< chip type (BHND_CHIPTYPE_*) */

	bhnd_addr_t	enum_addr;	/**< chip_type-specific enumeration
					  *  address; either the siba(4) base
					  *  core register block, or the bcma(4)
					  *  EROM core address. */

	uint8_t		ncores;		/**< number of cores, if known. 0 if
					  *  not available. */
};

/**
 * A bhnd(4) core descriptor.
 */
struct bhnd_core_info {
	uint16_t	vendor;		/**< JEP-106 vendor (BHND_MFGID_*) */
	uint16_t	device;		/**< device */
	uint16_t	hwrev;		/**< hardware revision */
	u_int		core_idx;	/**< bus-assigned core index */
	int		unit;		/**< bus-assigned core unit */
};


/**
 * A hardware revision match descriptor.
 */
struct bhnd_hwrev_match {
	uint16_t	start;	/**< first revision, or BHND_HWREV_INVALID
					     to match on any revision. */
	uint16_t	end;	/**< last revision, or BHND_HWREV_INVALID
					     to match on any revision. */
};

/**
* A bhnd(4) bus resource.
* 
* This provides an abstract interface to per-core resources that may require
* bus-level remapping of address windows prior to access.
*/
struct bhnd_resource {
	struct resource	*res;		/**< the system resource. */
	bool		 direct;	/**< false if the resource requires
					 *   bus window remapping before it
					 *   is MMIO accessible. */
};

/** 
 * Wildcard hardware revision match descriptor.
 */
#define	BHND_HWREV_ANY		{ BHND_HWREV_INVALID, BHND_HWREV_INVALID }
#define	BHND_HWREV_IS_ANY(_m)	\
	((_m)->start == BHND_HWREV_INVALID && (_m)->end == BHND_HWREV_INVALID)

/**
 * Hardware revision match descriptor for an inclusive range.
 * 
 * @param _start The first applicable hardware revision.
 * @param _end The last applicable hardware revision, or BHND_HWREV_INVALID
 * to match on any revision.
 */
#define	BHND_HWREV_RANGE(_start, _end)	{ _start, _end }

/**
 * Hardware revision match descriptor for a single revision.
 * 
 * @param _hwrev The hardware revision to match on.
 */
#define	BHND_HWREV_EQ(_hwrev)	BHND_HWREV_RANGE(_hwrev, _hwrev)

/**
 * Hardware revision match descriptor for any revision equal to or greater
 * than @p _start.
 * 
 * @param _start The first hardware revision to match on.
 */
#define	BHND_HWREV_GTE(_start)	BHND_HWREV_RANGE(_start, BHND_HWREV_INVALID)

/**
 * Hardware revision match descriptor for any revision equal to or less
 * than @p _end.
 * 
 * @param _end The last hardware revision to match on.
 */
#define	BHND_HWREV_LTE(_end)	BHND_HWREV_RANGE(0, _end)


/** A core match descriptor. */
struct bhnd_core_match {
	uint16_t		vendor;	/**< required JEP106 device vendor or BHND_MFGID_INVALID. */
	uint16_t		device;	/**< required core ID or BHND_COREID_INVALID */
	struct bhnd_hwrev_match	hwrev;	/**< matching revisions. */
	bhnd_devclass_t		class;	/**< required class or BHND_DEVCLASS_INVALID */
	int			unit;	/**< required core unit, or -1 */
};

/**
 * Core match descriptor matching against the given @p _vendor, @p _device,
 * and @p _hwrev match descriptors.
 */
#define	BHND_CORE_MATCH(_vendor, _device, _hwrev)	\
	{ _vendor, _device, _hwrev, BHND_DEVCLASS_INVALID, -1 }

/** 
 * Wildcard core match descriptor.
 */
#define	BHND_CORE_MATCH_ANY			\
	{					\
		.vendor = BHND_MFGID_INVALID,	\
		.device = BHND_COREID_INVALID,	\
		.hwrev = BHND_HWREV_ANY,	\
		.class = BHND_DEVCLASS_INVALID,	\
		.unit = -1			\
	}

/**
 * A chipset match descriptor.
 * 
 * @warning Matching on board/nvram attributes relies on NVRAM access, and will
 * fail if a valid NVRAM device cannot be found, or is not yet attached.
 */
struct bhnd_chip_match {
	/** Select fields to be matched */
	uint16_t
		match_id:1,
		match_rev:1,
		match_pkg:1,
		match_bvendor:1,
		match_btype:1,
		match_brev:1,
		match_srom_rev:1,
		match_any:1,
		match_flags_unused:8;

	uint16_t		chip_id;	/**< required chip id */
	struct bhnd_hwrev_match	chip_rev;	/**< matching chip revisions */
	uint8_t			chip_pkg;	/**< required package */

	uint16_t		board_vendor;	/**< required board vendor */
	uint16_t		board_type;	/**< required board type */
	struct bhnd_hwrev_match	board_rev;	/**< matching board revisions */

	struct bhnd_hwrev_match	board_srom_rev;	/**< matching board srom revisions */
};

#define	BHND_CHIP_MATCH_ANY		\
	{ .match_any = 1 }

#define	BHND_CHIP_MATCH_IS_ANY(_m)	\
	((_m)->match_any == 1)

#define	BHND_CHIP_MATCH_REQ_BOARD_INFO(_m)		\
	((_m)->match_srom_rev || (_m)->match_bvendor ||	\
	    (_m)->match_btype || (_m)->match_brev)

/** Set the required chip ID within a bhnd_chip_match instance */
#define	BHND_CHIP_ID(_cid)		\
	.match_id = 1, .chip_id = BHND_CHIPID_BCM ## _cid

/** Set the required chip revision range within a bhnd_chip_match instance */
#define	BHND_CHIP_REV(_rev)		\
	.match_rev = 1, .chip_rev = BHND_ ## _rev

/** Set the required package ID within a bhnd_chip_match instance */
#define	BHND_CHIP_PKG(_pkg)		\
	.match_pkg = 1, .chip_pkg = BHND_PKGID_BCM ## _pkg

/** Set the required board vendor within a bhnd_chip_match instance */
#define	BHND_CHIP_BVENDOR(_vend)		\
	.match_bvendor = 1, .board_vendor = _vend

/** Set the required board type within a bhnd_chip_match instance */
#define	BHND_CHIP_BTYPE(_btype)		\
	.match_btype = 1, .board_type = BHND_BOARD_ ## _btype

/** Set the required SROM revision range within a bhnd_chip_match instance */
#define	BHND_CHIP_SROMREV(_rev)		\
	.match_srom_rev = 1, .board_srom_rev = BHND_ ## _rev

/** Set the required board revision range within a bhnd_chip_match instance */
#define	BHND_CHIP_BREV(_rev)	\
	.match_brev = 1, .board_rev = BHND_ ## _rev

/** Set the required board vendor and type within a bhnd_chip_match instance */
#define	BHND_CHIP_BVT(_vend, _type)	\
	BHND_CHIP_BVENDOR(_vend), BHND_CHIP_BTYPE(_type)

/** Set the required board vendor, type, and revision within a bhnd_chip_match
 *  instance */
#define	BHND_CHIP_BVTR(_vend, _type, _rev)	\
	BHND_CHIP_BVT(_vend, _type), BHND_CHIP_BREV(_rev)

/** Set the required chip and package ID within a bhnd_chip_match instance */
#define	BHND_CHIP_IP(_cid, _pkg)	\
	BHND_CHIP_ID(_cid), BHND_CHIP_PKG(_pkg)

/** Set the required chip ID, package ID, and revision within a bhnd_chip_match
 *  instance */
#define	BHND_CHIP_IPR(_cid, _pkg, _rev)	\
	BHND_CHIP_ID(_cid), BHND_CHIP_PKG(_pkg), BHND_CHIP_REV(_rev)

/** Set the required chip ID and revision within a bhnd_chip_match
 *  instance */
#define	BHND_CHIP_IR(_cid, _rev)	\
	BHND_CHIP_ID(_cid), BHND_CHIP_REV(_rev)

/**
 * Chipset quirk table descriptor.
 */
struct bhnd_chip_quirk {
	const struct bhnd_chip_match	 chip;		/**< chip match descriptor */ 
	uint32_t			 quirks;	/**< quirk flags */
};

#define	BHND_CHIP_QUIRK_END	{ BHND_CHIP_MATCH_ANY, 0 }

#define	BHND_CHIP_QUIRK_IS_END(_q)	\
	(BHND_CHIP_MATCH_IS_ANY(&(_q)->chip) && (_q)->quirks == 0)

/**
 * Device quirk table descriptor.
 */
struct bhnd_device_quirk {
	struct bhnd_hwrev_match	 hwrev;		/**< applicable hardware revisions */
	uint32_t		 quirks;	/**< quirk flags */
};
#define	BHND_DEVICE_QUIRK_END		{ BHND_HWREV_ANY, 0 }
#define	BHND_DEVICE_QUIRK_IS_END(_q)	\
	(BHND_HWREV_IS_ANY(&(_q)->hwrev) && (_q)->quirks == 0)

enum {
	BHND_DF_ANY	= 0,
	BHND_DF_HOSTB	= (1<<0)	/**< core is serving as the bus'
					  *  host bridge */
};

/** Device probe table descriptor */
struct bhnd_device {
	const struct bhnd_core_match	 core;			/**< core match descriptor */ 
	const char			*desc;			/**< device description, or NULL. */
	const struct bhnd_device_quirk	*quirks_table;		/**< quirks table for this device, or NULL */
	const struct bhnd_chip_quirk	*chip_quirks_table;	/**< chipset-specific quirks for this device, or NULL */
	uint32_t			 device_flags;		/**< required BHND_DF_* flags */
};

#define	_BHND_DEVICE(_vendor, _device, _desc, _quirks, _chip_quirks,	\
     _flags, ...)							\
	{ BHND_CORE_MATCH(BHND_MFGID_ ## _vendor,			\
	    BHND_COREID_ ## _device, BHND_HWREV_ANY), _desc, _quirks,	\
	    _chip_quirks, _flags }

#define	BHND_MIPS_DEVICE(_device, _desc, _quirks, _chip_quirks, ...)	\
	_BHND_DEVICE(MIPS, _device, _desc, _quirks, _chip_quirks,	\
	    ## __VA_ARGS__, 0)

#define	BHND_ARM_DEVICE(_device, _desc, _quirks, _chip_quirks, ...)	\
	_BHND_DEVICE(ARM, _device, _desc, _quirks, _chip_quirks,	\
	    ## __VA_ARGS__, 0)

#define	BHND_DEVICE(_device, _desc, _quirks, _chip_quirks, ...)		\
	_BHND_DEVICE(BCM, _device, _desc, _quirks, _chip_quirks,	\
	    ## __VA_ARGS__, 0)

#define	BHND_DEVICE_END	{ BHND_CORE_MATCH_ANY, NULL, NULL, NULL, 0 }

const char			*bhnd_vendor_name(uint16_t vendor);
const char			*bhnd_port_type_name(bhnd_port_type port_type);

const char 			*bhnd_find_core_name(uint16_t vendor,
				     uint16_t device);
bhnd_devclass_t			 bhnd_find_core_class(uint16_t vendor,
				     uint16_t device);

const char			*bhnd_core_name(const struct bhnd_core_info *ci);
bhnd_devclass_t			 bhnd_core_class(const struct bhnd_core_info *ci);


device_t			 bhnd_match_child(device_t dev,
				     const struct bhnd_core_match *desc);

device_t			 bhnd_find_child(device_t dev,
				     bhnd_devclass_t class, int unit);

device_t			 bhnd_find_bridge_root(device_t dev,
				     devclass_t bus_class);

const struct bhnd_core_info	*bhnd_match_core(
				     const struct bhnd_core_info *cores,
				     u_int num_cores,
				     const struct bhnd_core_match *desc);

const struct bhnd_core_info	*bhnd_find_core(
				     const struct bhnd_core_info *cores,
				     u_int num_cores, bhnd_devclass_t class);

bool				 bhnd_core_matches(
				     const struct bhnd_core_info *core,
				     const struct bhnd_core_match *desc);

bool				 bhnd_chip_matches(
				     const struct bhnd_chipid *chipid,
				     const struct bhnd_board_info *binfo,
				     const struct bhnd_chip_match *desc);

bool				 bhnd_hwrev_matches(uint16_t hwrev,
				     const struct bhnd_hwrev_match *desc);

uint32_t			 bhnd_chip_quirks(device_t dev,
				     const struct bhnd_chip_quirk *table);

bool				 bhnd_device_matches(device_t dev,
				     const struct bhnd_core_match *desc);

const struct bhnd_device	*bhnd_device_lookup(device_t dev,
				     const struct bhnd_device *table,
				     size_t entry_size);

uint32_t			 bhnd_device_quirks(device_t dev,
				     const struct bhnd_device *table,
				     size_t entry_size);

struct bhnd_core_info		 bhnd_get_core_info(device_t dev);


int				 bhnd_alloc_resources(device_t dev,
				     struct resource_spec *rs,
				     struct bhnd_resource **res);

void				 bhnd_release_resources(device_t dev,
				     const struct resource_spec *rs,
				     struct bhnd_resource **res);

struct bhnd_chipid		 bhnd_parse_chipid(uint32_t idreg,
				     bhnd_addr_t enum_addr);

int				 bhnd_read_chipid(device_t dev,
				     struct resource_spec *rs,
				     bus_size_t chipc_offset,
				     struct bhnd_chipid *result);

void				 bhnd_set_custom_core_desc(device_t dev,
				     const char *name);
void				 bhnd_set_default_core_desc(device_t dev);


bool				 bhnd_bus_generic_is_hw_disabled(device_t dev,
				     device_t child);
bool				 bhnd_bus_generic_is_region_valid(device_t dev,
				     device_t child, bhnd_port_type type,
				     u_int port, u_int region);
int				 bhnd_bus_generic_read_nvram_var(device_t dev,
				     device_t child, const char *name,
				     void *buf, size_t *size);
const struct bhnd_chipid	*bhnd_bus_generic_get_chipid(device_t dev,
				     device_t child);
int				 bhnd_bus_generic_read_board_info(device_t dev,
				     device_t child,
				     struct bhnd_board_info *info);
int				 bhnd_bus_generic_get_nvram_var(device_t dev,
				    device_t child, const char *name,
				    void *buf, size_t *size);
struct bhnd_resource		*bhnd_bus_generic_alloc_resource (device_t dev,
				     device_t child, int type, int *rid,
				     rman_res_t start, rman_res_t end,
				     rman_res_t count, u_int flags);
int				 bhnd_bus_generic_release_resource (device_t dev,
				     device_t child, int type, int rid,
				     struct bhnd_resource *r);
int				 bhnd_bus_generic_activate_resource (device_t dev,
				     device_t child, int type, int rid,
				     struct bhnd_resource *r);
int				 bhnd_bus_generic_deactivate_resource (device_t dev,
				     device_t child, int type, int rid,
				     struct bhnd_resource *r);



/**
 * Return the active host bridge core for the bhnd bus, if any, or NULL if
 * not found.
 *
 * @param dev A bhnd bus device.
 */
static inline device_t
bhnd_find_hostb_device(device_t dev) {
	return (BHND_BUS_FIND_HOSTB_DEVICE(dev));
}

/**
 * Return true if the hardware components required by @p dev are known to be
 * unpopulated or otherwise unusable.
 *
 * In some cases, enumerated devices may have pins that are left floating, or
 * the hardware may otherwise be non-functional; this method allows a parent
 * device to explicitly specify if a successfully enumerated @p dev should
 * be disabled.
 *
 * @param dev A bhnd bus child device.
 */
static inline bool
bhnd_is_hw_disabled(device_t dev) {
	return (BHND_BUS_IS_HW_DISABLED(device_get_parent(dev), dev));
}

/**
 * Return the BHND chip identification info for the bhnd bus.
 *
 * @param dev A bhnd bus child device.
 */
static inline const struct bhnd_chipid *
bhnd_get_chipid(device_t dev) {
	return (BHND_BUS_GET_CHIPID(device_get_parent(dev), dev));
};

/**
 * Attempt to read the BHND board identification from the bhnd bus.
 *
 * This relies on NVRAM access, and will fail if a valid NVRAM device cannot
 * be found, or is not yet attached.
 *
 * @param dev The parent of @p child.
 * @param child The bhnd device requesting board info.
 * @param[out] info On success, will be populated with the bhnd(4) device's
 * board information.
 *
 * @retval 0 success
 * @retval ENODEV	No valid NVRAM source could be found.
 * @retval non-zero	If reading @p name otherwise fails, a regular unix
 *			error code will be returned.
 */
static inline int
bhnd_read_board_info(device_t dev, struct bhnd_board_info *info)
{
	return (BHND_BUS_READ_BOARD_INFO(device_get_parent(dev), dev, info));
}

/**
 * Determine an NVRAM variable's expected size.
 *
 * @param 	dev	A bhnd bus child device.
 * @param	name	The variable name.
 * @param[out]	len	On success, the variable's size, in bytes.
 *
 * @retval 0		success
 * @retval ENOENT	The requested variable was not found.
 * @retval ENODEV	No valid NVRAM source could be found.
 * @retval non-zero	If reading @p name otherwise fails, a regular unix
 *			error code will be returned.
 */
static inline int
bhnd_nvram_getvarlen(device_t dev, const char *name, size_t *len)
{
	return (BHND_BUS_GET_NVRAM_VAR(device_get_parent(dev), dev, name, NULL,
	    len));
}

/**
 * Read an NVRAM variable.
 *
 * @param 	dev	A bhnd bus child device.
 * @param	name	The NVRAM variable name.
 * @param	buf	A buffer large enough to hold @p len bytes. On success,
 * 			the requested value will be written to this buffer.
 * @param	len	The required variable length.
 *
 * @retval 0		success
 * @retval ENOENT	The requested variable was not found.
 * @retval EINVAL	If @p len does not match the actual variable size.
 * @retval ENODEV	No valid NVRAM source could be found.
 * @retval non-zero	If reading @p name otherwise fails, a regular unix
 *			error code will be returned.
 */
static inline int
bhnd_nvram_getvar(device_t dev, const char *name, void *buf, size_t len)
{
	size_t	var_len;
	int	error;

	if ((error = bhnd_nvram_getvarlen(dev, name, &var_len)))
		return (error);

	if (len != var_len)
		return (EINVAL);

	return (BHND_BUS_GET_NVRAM_VAR(device_get_parent(dev), dev, name, buf,
	    &len));
}

/**
 * Allocate a resource from a device's parent bhnd(4) bus.
 * 
 * @param dev The device requesting resource ownership.
 * @param type The type of resource to allocate. This may be any type supported
 * by the standard bus APIs.
 * @param rid The bus-specific handle identifying the resource being allocated.
 * @param start The start address of the resource.
 * @param end The end address of the resource.
 * @param count The size of the resource.
 * @param flags The flags for the resource to be allocated. These may be any
 * values supported by the standard bus APIs.
 * 
 * To request the resource's default addresses, pass @p start and
 * @p end values of @c 0 and @c ~0, respectively, and
 * a @p count of @c 1.
 * 
 * @retval NULL The resource could not be allocated.
 * @retval resource The allocated resource.
 */
static inline struct bhnd_resource *
bhnd_alloc_resource(device_t dev, int type, int *rid, rman_res_t start,
    rman_res_t end, rman_res_t count, u_int flags)
{
	return BHND_BUS_ALLOC_RESOURCE(device_get_parent(dev), dev, type, rid,
	    start, end, count, flags);
}


/**
 * Allocate a resource from a device's parent bhnd(4) bus, using the
 * resource's default start, end, and count values.
 * 
 * @param dev The device requesting resource ownership.
 * @param type The type of resource to allocate. This may be any type supported
 * by the standard bus APIs.
 * @param rid The bus-specific handle identifying the resource being allocated.
 * @param flags The flags for the resource to be allocated. These may be any
 * values supported by the standard bus APIs.
 * 
 * @retval NULL The resource could not be allocated.
 * @retval resource The allocated resource.
 */
static inline struct bhnd_resource *
bhnd_alloc_resource_any(device_t dev, int type, int *rid, u_int flags)
{
	return bhnd_alloc_resource(dev, type, rid, 0, ~0, 1, flags);
}

/**
 * Activate a previously allocated bhnd resource.
 *
 * @param dev The device holding ownership of the allocated resource.
 * @param type The type of the resource. 
 * @param rid The bus-specific handle identifying the resource.
 * @param r A pointer to the resource returned by bhnd_alloc_resource or
 * BHND_BUS_ALLOC_RESOURCE.
 * 
 * @retval 0 success
 * @retval non-zero an error occurred while activating the resource.
 */
static inline int
bhnd_activate_resource(device_t dev, int type, int rid,
   struct bhnd_resource *r)
{
	return BHND_BUS_ACTIVATE_RESOURCE(device_get_parent(dev), dev, type,
	    rid, r);
}

/**
 * Deactivate a previously activated bhnd resource.
 *
 * @param dev The device holding ownership of the activated resource.
 * @param type The type of the resource. 
 * @param rid The bus-specific handle identifying the resource.
 * @param r A pointer to the resource returned by bhnd_alloc_resource or
 * BHND_BUS_ALLOC_RESOURCE.
 * 
 * @retval 0 success
 * @retval non-zero an error occurred while activating the resource.
 */
static inline int
bhnd_deactivate_resource(device_t dev, int type, int rid,
   struct bhnd_resource *r)
{
	return BHND_BUS_DEACTIVATE_RESOURCE(device_get_parent(dev), dev, type,
	    rid, r);
}

/**
 * Free a resource allocated by bhnd_alloc_resource().
 *
 * @param dev The device holding ownership of the resource.
 * @param type The type of the resource. 
 * @param rid The bus-specific handle identifying the resource.
 * @param r A pointer to the resource returned by bhnd_alloc_resource or
 * BHND_ALLOC_RESOURCE.
 * 
 * @retval 0 success
 * @retval non-zero an error occurred while activating the resource.
 */
static inline int
bhnd_release_resource(device_t dev, int type, int rid,
   struct bhnd_resource *r)
{
	return BHND_BUS_RELEASE_RESOURCE(device_get_parent(dev), dev, type,
	    rid, r);
}

/**
 * Return true if @p region_num is a valid region on @p port_num of
 * @p type attached to @p dev.
 *
 * @param dev A bhnd bus child device.
 * @param type The port type being queried.
 * @param port_num The port number being queried.
 * @param region_num The region number being queried.
 */
static inline bool
bhnd_is_region_valid(device_t dev, bhnd_port_type type, u_int port_num,
    u_int region_num)
{
	return (BHND_BUS_IS_REGION_VALID(device_get_parent(dev), dev, type,
	    port_num, region_num));
}

/**
 * Return the number of ports of type @p type attached to @p def.
 *
 * @param dev A bhnd bus child device.
 * @param type The port type being queried.
 */
static inline u_int
bhnd_get_port_count(device_t dev, bhnd_port_type type) {
	return (BHND_BUS_GET_PORT_COUNT(device_get_parent(dev), dev, type));
}

/**
 * Return the number of memory regions mapped to @p child @p port of
 * type @p type.
 *
 * @param dev A bhnd bus child device.
 * @param port The port number being queried.
 * @param type The port type being queried.
 */
static inline u_int
bhnd_get_region_count(device_t dev, bhnd_port_type type, u_int port) {
	return (BHND_BUS_GET_REGION_COUNT(device_get_parent(dev), dev, type,
	    port));
}

/**
 * Return the resource-ID for a memory region on the given device port.
 *
 * @param dev A bhnd bus child device.
 * @param type The port type.
 * @param port The port identifier.
 * @param region The identifier of the memory region on @p port.
 * 
 * @retval int The RID for the given @p port and @p region on @p device.
 * @retval -1 No such port/region found.
 */
static inline int
bhnd_get_port_rid(device_t dev, bhnd_port_type type, u_int port, u_int region)
{
	return BHND_BUS_GET_PORT_RID(device_get_parent(dev), dev, type, port,
	    region);
}

/**
 * Decode a port / region pair on @p dev defined by @p rid.
 *
 * @param dev A bhnd bus child device.
 * @param type The resource type.
 * @param rid The resource identifier.
 * @param[out] port_type The decoded port type.
 * @param[out] port The decoded port identifier.
 * @param[out] region The decoded region identifier.
 *
 * @retval 0 success
 * @retval non-zero No matching port/region found.
 */
static inline int
bhnd_decode_port_rid(device_t dev, int type, int rid, bhnd_port_type *port_type,
    u_int *port, u_int *region)
{
	return BHND_BUS_DECODE_PORT_RID(device_get_parent(dev), dev, type, rid,
	    port_type, port, region);
}

/**
 * Get the address and size of @p region on @p port.
 *
 * @param dev A bhnd bus child device.
 * @param port_type The port type.
 * @param port The port identifier.
 * @param region The identifier of the memory region on @p port.
 * @param[out] region_addr The region's base address.
 * @param[out] region_size The region's size.
 *
 * @retval 0 success
 * @retval non-zero No matching port/region found.
 */
static inline int
bhnd_get_region_addr(device_t dev, bhnd_port_type port_type, u_int port,
    u_int region, bhnd_addr_t *region_addr, bhnd_size_t *region_size)
{
	return BHND_BUS_GET_REGION_ADDR(device_get_parent(dev), dev, port_type,
	    port, region, region_addr, region_size);
}

/*
 * bhnd bus-level equivalents of the bus_(read|write|set|barrier|...)
 * macros (compatible with bhnd_resource).
 *
 * Generated with bhnd/tools/bus_macro.sh
 */
#define bhnd_bus_barrier(r, o, l, f) \
    ((r)->direct) ? \
	bus_barrier((r)->res, (o), (l), (f)) : \
	BHND_BUS_BARRIER( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (l), (f))
#define bhnd_bus_read_1(r, o) \
    ((r)->direct) ? \
	bus_read_1((r)->res, (o)) : \
	BHND_BUS_READ_1( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o))
#define bhnd_bus_read_multi_1(r, o, d, c) \
    ((r)->direct) ? \
	bus_read_multi_1((r)->res, (o), (d), (c)) : \
	BHND_BUS_READ_MULTI_1( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (d), (c))
#define bhnd_bus_write_1(r, o, v) \
    ((r)->direct) ? \
	bus_write_1((r)->res, (o), (v)) : \
	BHND_BUS_WRITE_1( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (v))
#define bhnd_bus_write_multi_1(r, o, d, c) \
    ((r)->direct) ? \
	bus_write_multi_1((r)->res, (o), (d), (c)) : \
	BHND_BUS_WRITE_MULTI_1( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (d), (c))
#define bhnd_bus_read_stream_1(r, o) \
    ((r)->direct) ? \
	bus_read_stream_1((r)->res, (o)) : \
	BHND_BUS_READ_STREAM_1( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o))
#define bhnd_bus_read_multi_stream_1(r, o, d, c) \
    ((r)->direct) ? \
	bus_read_multi_stream_1((r)->res, (o), (d), (c)) : \
	BHND_BUS_READ_MULTI_STREAM_1( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (d), (c))
#define bhnd_bus_write_stream_1(r, o, v) \
    ((r)->direct) ? \
	bus_write_stream_1((r)->res, (o), (v)) : \
	BHND_BUS_WRITE_STREAM_1( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (v))
#define bhnd_bus_write_multi_stream_1(r, o, d, c) \
    ((r)->direct) ? \
	bus_write_multi_stream_1((r)->res, (o), (d), (c)) : \
	BHND_BUS_WRITE_MULTI_STREAM_1( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (d), (c))
#define bhnd_bus_read_2(r, o) \
    ((r)->direct) ? \
	bus_read_2((r)->res, (o)) : \
	BHND_BUS_READ_2( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o))
#define bhnd_bus_read_multi_2(r, o, d, c) \
    ((r)->direct) ? \
	bus_read_multi_2((r)->res, (o), (d), (c)) : \
	BHND_BUS_READ_MULTI_2( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (d), (c))
#define bhnd_bus_write_2(r, o, v) \
    ((r)->direct) ? \
	bus_write_2((r)->res, (o), (v)) : \
	BHND_BUS_WRITE_2( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (v))
#define bhnd_bus_write_multi_2(r, o, d, c) \
    ((r)->direct) ? \
	bus_write_multi_2((r)->res, (o), (d), (c)) : \
	BHND_BUS_WRITE_MULTI_2( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (d), (c))
#define bhnd_bus_read_stream_2(r, o) \
    ((r)->direct) ? \
	bus_read_stream_2((r)->res, (o)) : \
	BHND_BUS_READ_STREAM_2( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o))
#define bhnd_bus_read_multi_stream_2(r, o, d, c) \
    ((r)->direct) ? \
	bus_read_multi_stream_2((r)->res, (o), (d), (c)) : \
	BHND_BUS_READ_MULTI_STREAM_2( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (d), (c))
#define bhnd_bus_write_stream_2(r, o, v) \
    ((r)->direct) ? \
	bus_write_stream_2((r)->res, (o), (v)) : \
	BHND_BUS_WRITE_STREAM_2( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (v))
#define bhnd_bus_write_multi_stream_2(r, o, d, c) \
    ((r)->direct) ? \
	bus_write_multi_stream_2((r)->res, (o), (d), (c)) : \
	BHND_BUS_WRITE_MULTI_STREAM_2( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (d), (c))
#define bhnd_bus_read_4(r, o) \
    ((r)->direct) ? \
	bus_read_4((r)->res, (o)) : \
	BHND_BUS_READ_4( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o))
#define bhnd_bus_read_multi_4(r, o, d, c) \
    ((r)->direct) ? \
	bus_read_multi_4((r)->res, (o), (d), (c)) : \
	BHND_BUS_READ_MULTI_4( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (d), (c))
#define bhnd_bus_write_4(r, o, v) \
    ((r)->direct) ? \
	bus_write_4((r)->res, (o), (v)) : \
	BHND_BUS_WRITE_4( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (v))
#define bhnd_bus_write_multi_4(r, o, d, c) \
    ((r)->direct) ? \
	bus_write_multi_4((r)->res, (o), (d), (c)) : \
	BHND_BUS_WRITE_MULTI_4( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (d), (c))
#define bhnd_bus_read_stream_4(r, o) \
    ((r)->direct) ? \
	bus_read_stream_4((r)->res, (o)) : \
	BHND_BUS_READ_STREAM_4( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o))
#define bhnd_bus_read_multi_stream_4(r, o, d, c) \
    ((r)->direct) ? \
	bus_read_multi_stream_4((r)->res, (o), (d), (c)) : \
	BHND_BUS_READ_MULTI_STREAM_4( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (d), (c))
#define bhnd_bus_write_stream_4(r, o, v) \
    ((r)->direct) ? \
	bus_write_stream_4((r)->res, (o), (v)) : \
	BHND_BUS_WRITE_STREAM_4( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (v))
#define bhnd_bus_write_multi_stream_4(r, o, d, c) \
    ((r)->direct) ? \
	bus_write_multi_stream_4((r)->res, (o), (d), (c)) : \
	BHND_BUS_WRITE_MULTI_STREAM_4( \
	    device_get_parent(rman_get_device((r)->res)),	\
	    rman_get_device((r)->res), (r), (o), (d), (c))

#endif /* _BHND_BHND_H_ */
