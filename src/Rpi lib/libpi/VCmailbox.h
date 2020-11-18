#ifndef VC_MB_H
#define VC_MB_H

// gpu name is videocore
// cpu and gpu communicate through mailbox 0 and 1 to query information
// there also another set of mailboxes between cpu cores, this file is only about gpu mailbox

/* a properly aligned buffer */
extern volatile unsigned int mbox[36];

#define MBOX_REQUEST    0

/* channels */
typedef enum{
    MBOX_CH_POWER = 0,
    MBOX_CH_FB    = 1,
    MBOX_CH_VUART = 2,
    MBOX_CH_VCHIQ = 3,
    MBOX_CH_LEDS  = 4,
    MBOX_CH_BTNS  = 5,
    MBOX_CH_TOUCH = 6,
    MBOX_CH_COUNT = 7,
    MBOX_CH_PROP  = 8,
} mb_channel_t;

/* tags */
typedef enum{
    MBOX_TAG_GETSERIAL = 0x10004,
    MBOX_TAG_LAST      = 0x0,
    MBOX_TAG_GETCLOCK  = 0x00030002,
} mb_tags_t;

int mbox_call(mb_channel_t ch);

#endif