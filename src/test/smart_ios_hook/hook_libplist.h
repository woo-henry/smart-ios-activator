#pragma once

LONG StartHookLibplist();
LONG FinishHookLibplist();

typedef void* plist_t;

typedef void* plist_dict_iter;

typedef void* plist_array_iter;

typedef enum
{
	PLIST_BOOLEAN,	/**< Boolean, scalar type */
	PLIST_UINT,	/**< Unsigned integer, scalar type */
	PLIST_REAL,	/**< Real, scalar type */
	PLIST_STRING,	/**< ASCII string, scalar type */
	PLIST_ARRAY,	/**< Ordered array, structured type */
	PLIST_DICT,	/**< Unordered dictionary (key/value pair), structured type */
	PLIST_DATE,	/**< Date, scalar type */
	PLIST_DATA,	/**< Binary data, scalar type */
	PLIST_KEY,	/**< Key in dictionaries (ASCII String), scalar type */
	PLIST_UID,      /**< Special type used for 'keyed encoding' */
	PLIST_NONE	/**< No type */
} plist_type;