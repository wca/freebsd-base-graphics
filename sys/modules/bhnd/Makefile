# $FreeBSD$

.PATH: ${.CURDIR}/../../dev/bhnd
.PATH: ${.CURDIR}/../../dev/bhnd/nvram

KMOD=	bhnd
SRCS=	bhnd.c bhnd_subr.c \
	bhnd_sprom.c nvram_subr.c \
	bhnd_nvram_map.h bhnd_nvram_map_data.h

SRCS+=	bhnd_bus_if.c bhnd_bus_if.h \
	bhnd_chipc_if.c bhnd_chipc_if.h \
	bhnd_nvram_if.c bhnd_nvram_if.h

SRCS+=	device_if.h bus_if.h

SUBDIR=	bcma \
	bcma_bhndb \
	bhndb \
	bhndb_pci \
	cores \
	siba \
	siba_bhndb

.include <bsd.kmod.mk>
.include <bsd.subdir.mk>
