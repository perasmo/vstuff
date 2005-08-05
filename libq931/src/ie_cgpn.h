
#ifndef _LIBQ931_IE_CGPN_H
#define _LIBQ931_IE_CGPN_H

#include "ie.h"

/*********************** Calling Party Number *************************/

enum q931_ie_calling_party_number_type_of_number
{
	Q931_IE_CGPN_TON_UNKNOWN		= 0x0,
	Q931_IE_CGPN_TON_INTERNATIONAL		= 0x1,
	Q931_IE_CGPN_TON_NATIONAL		= 0x2,
	Q931_IE_CGPN_TON_NETWORK_SPECIFIC	= 0x3,
	Q931_IE_CGPN_TON_SUBSCRIBER		= 0x4,
	Q931_IE_CGPN_TON_ABBREVIATED		= 0x6,
	Q931_IE_CGPN_TON_RESERVED_FOR_EXT	= 0x7
};

enum q931_ie_calling_party_number_numbering_plan_identificator
{
	Q931_IE_CGPN_NP_UNKNOWN			= 0x0,
	Q931_IE_CGPN_NP_ISDN_TELEPHONY		= 0x1,
	Q931_IE_CGPN_NP_DATA			= 0x3,
	Q931_IE_CGPN_NP_TELEX			= 0x4,
	Q931_IE_CGPN_NP_NATIONAL_STANDARD	= 0x8,
	Q931_IE_CGPN_NP_PRIVATE			= 0x9,
	Q931_IE_CGPN_NP_RESERVED_FOR_EXT	= 0xf
};

enum q931_ie_calling_party_number_presentation_indicator
{
	Q931_IE_CGPN_PI_PRESENTATION_ALLOWED	= 0x0,
	Q931_IE_CGPN_PI_PRESENTATION_RESTRICTED	= 0x1,
	Q931_IE_CGPN_PI_NOT_AVAILABLE		= 0x2,
	Q931_IE_CGPN_PI_RESERVED		= 0x3,
};

enum q931_ie_calling_party_number_screening_indicator
{
	Q931_IE_CGPN_SI_USER_PROVIDED_NOT_SCREENED		= 0x0,
	Q931_IE_CGPN_SI_USER_PROVIDED_VERIFIED_AND_PASSED	= 0x1,
	Q931_IE_CGPN_SI_USER_PROVIDED_VERIFIED_AND_FAILED	= 0x2,
	Q931_IE_CGPN_SI_NETWORK_PROVIDED			= 0x3,
};

struct q931_ie_calling_party_number
{
	struct q931_ie ie;

	enum q931_ie_calling_party_number_type_of_number
		type_of_number;
	enum q931_ie_calling_party_number_numbering_plan_identificator
		numbering_plan_identificator;

	char number[21];
};

#ifdef Q931_PRIVATE

struct q931_ie_calling_party_number_onwire_3_4
{
#if __BYTE_ORDER == __BIG_ENDIAN
	__u8 ext:1;
	__u8 type_of_number:3;
	__u8 numbering_plan_identificator:4;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	__u8 numbering_plan_identificator:4;
	__u8 type_of_number:3;
	__u8 ext:1;
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
	__u8 ext2:1;
	__u8 presentation_indicator:2;
	__u8 :3;
	__u8 screening_indicator:2;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	__u8 screening_indicator:2;
	__u8 :3;
	__u8 presentation_indicator:2;
	__u8 ext2:1;
#endif
} __attribute__ ((__packed__));

void q931_ie_calling_party_number_init(
	struct q931_ie_calling_party_number *ie);

void q931_ie_calling_party_number_register(
	const struct q931_ie_type *type);

int q931_ie_calling_party_number_check(
	const struct q931_ie *ie,
	const struct q931_message *msg);

int q931_ie_calling_party_number_write_to_buf(
	const struct q931_ie *generic_ie,
        void *buf,
	int max_size);

#endif
#endif
