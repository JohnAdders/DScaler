
/**
 * @file btwdmprop.h various ir related stuff from btwincap driver
 */
#include <ks.h>

//{C44A1A10-4A38-11D2-8328-006097BA83AB}
GUID PROPSETID_IR =
	{0xC44A1A10, 0x4A38, 0x11D2, {0x83, 0x28, 0x00, 0x60, 0x97, 0xBA, 0x83, 0xAB}};

#define KSPROPERTY_IR_GETCODE				0x20000001	/* to read the IR remote code */
#define KSPROPERTY_IR_CAPS					0x2000000f	/* to get IR caps */

typedef struct
{
	KSPROPERTY Property;
	ULONG Code;
} KSPROPERTY_IR_GETCODE_S, *PKSPROPERTY_IR_GETCODE_S;

#define KSPROPERTY_IR_CAPS_AVAILABLE		0x0001		/* means that IR control is available */

typedef struct
{
	KSPROPERTY Property;
	ULONG Caps;				// Capabilities (KSPROPERTY_IR_CAPS_...)
} KSPROPERTY_IR_CAPS_S, *PKSPROPERTY_IR_CAPS_S;