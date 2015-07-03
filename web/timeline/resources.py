import itertools


PIN_LAYOUTS = {
    'GENERIC': 'genericPin',
    'CALENDAR': 'calendarPin',
    'SPORTS': 'sportsPin',
    'WEATHER': 'weatherPin'
}

REMINDER_LAYOUTS = {
    'GENERIC': 'genericReminder'
}

NOTIFICATION_LAYOUTS = {
    'GENERIC': 'genericNotification'
}

LAYOUT_TYPES = list(itertools.chain(*map(lambda d: d.values(), [
    PIN_LAYOUTS, REMINDER_LAYOUTS, NOTIFICATION_LAYOUTS
])))

ACTION_TYPES = {
    'OPEN': 'openWatchApp'
}

ICONS = {
    "NOTIFICATIONS": {
        "GENERIC": "system://images/NOTIFICATION_GENERIC",
        "REMINDER": "system://images/NOTIFICATION_REMINDER",
        "FLAG": "system://images/NOTIFICATION_FLAG",
        "FACEBOOK_MESSENGER": "system://images/NOTIFICATION_FACEBOOK_MESSENGER",
        "WHATSAPP": "system://images/NOTIFICATION_WHATSAPP",
        "GMAIL": "system://images/NOTIFICATION_GMAIL",
        "FACEBOOK": "system://images/NOTIFICATION_FACEBOOK",
        "HANGOUTS": "system://images/NOTIFICATION_GOOGLE_HANGOUTS",
        "TELEGRAM": "system://images/NOTIFICATION_TELEGRAM",
        "TWITTER": "system://images/NOTIFICATION_TWITTER",
        "INBOX": "system://images/NOTIFICATION_GOOGLE_INBOX",
        "MAILBOX": "system://images/NOTIFICATION_MAILBOX",
        "OUTLOOK": "system://images/NOTIFICATION_OUTLOOK",
        "INSTAGRAM": "system://images/NOTIFICATION_INSTAGRAM",
        "BLACKBERRY": "system://images/NOTIFICATION_BLACKBERRY_MESSSENGER",
        "LINE": "system://images/NOTIFICATION_LINE",
        "SNAPCHAT": "system://images/NOTIFICATION_SNAPCHAT",
        "WECHAT": "system://images/NOTIFICAITON_WECHAT",
        "VIBER": "system://images/NOTIFICATION_VIBER",
        "SKYPE": "system://images/NOTIFICATION_SKYPE",
        "YAHOO": "system://images/NOTIFICATION_YAHOO_MAIL"
    },
    "GENERIC": {
        "EMAIL": "system://images/GENERIC_EMAIL",
        "SMS": "system://images/GENERIC_SMS",
        "WARNING": "system://images/GENERIC_WARNING",
        "CONFIRMATION": "system://images/GENERIC_CONFIRMATION",
        "QUESTION": "system://images/GENERIC_QUESTION"
    },
    "WEATHER": {
        "PARTLY_CLOUDY": "system://images/PARTLY_CLOUDY",
        "CLOUDY": "system://images/CLOUDY_DAY",
        "LIGHT_SNOW": "system://images/LIGHT_SNOW",
        "LIGHT_RAIN": "system://images/LIGHT_RAIN",
        "HEAVY_RAIN": "system://images/HEAVY_RAIN",
        "HEAVY_SNOW": "system://images/HEAVY_SNOW",
        "WEATHER": "system://images/TIMELINE_WEATHER",
        "SUN": "system://images/TIMELINE_SUN",
        "RAIN_AND_SNOW": "system://images/RAINING_AND_SNOWING"
    },
    "TIMELINE": {
        "MISSED_CALL": "system://images/TIMELINE_MISSED_CALL",
        "CALENDAR": "system://images/TIMELINE_CALENDAR",
        "SPORTS": "system://images/TIMELINE_SPORTS"
    },
    "SPORTS": {
        "BASEBALL": "system://images/TIMELINE_BASEBALL",
        "FOOTBALL": "system://images/AMERICAN_FOOTBALL",
        "CRICKET": "system://images/CRICKET_GAME",
        "SOCCER": "system://images/SOCCER_GAME",
        "HOCKEY": "system://images/HOCKEY_GAME"
    },
    "ACTIONS": {
        "DISMISSED": "system://images/RESULT_DISMISSED",
        "DELETED": "system://images/RESULT_DELETED",
        "MUTE": "system://images/RESULT_MUTE",
        "SENT": "system://images/RESULT_SENT",
        "FAILED": "system://images/RESULT_FAILED"
    },
    "MISC": {
        "STOCKS": "system://images/STOCKS_EVENTS",
        "MUSIC": "system://images/MUSIC_EVENT",
        "BIRTHDAY": "system://images/BIRTHDAY_EVENT",
        "BILL": "system://images/PAY_BILL",
        "HOTEL": "system://images/HOTEL_RESERVATION",
        "TIDE": "system://images/TIDE_IS_HIGH",
        "NEWS": "system://images/NEWS_EVENT",
        "SCHEDULED": "system://images/SCHEDULED_EVENT",
        "PHONE": "system://images/DURING_PHONE_CALL",
        "INTERNET": "system://images/CHECK_INTERNET_CONNECTION",
        "MOVIE": "system://images/MOVIE_EVENT",
        "GLUCOSE": "system://images/GLUCOSE_MONITOR",
        "ALARM": "system://images/ALARM_CLOCK",
        "RENTAL": "system://images/CAR_RENTAL",
        "DINNER": "system://images/DINNER_RESERVATION",
        "RADIO": "system://images/RADIO_SHOW",
        "CASETTE": "system://images/AUDIO_CASETTE",
        "FLIGHT": "system://images/SCHEDULED_FLIGHT",
        "NONE": "system://images/NO_EVENTS",
        "FITNESS": "system://images/REACHED_FITNESS_GOAL",
        "DAY": "system://images/DAY_SEPARATOR",
        "DISCONNECT": "system://images/WATCH_DISCONNECTED",
        "SHOW": "system://images/TV_SHOW"
    }
}

COLOURS = {
    'INCH_WORM': '#AAFF55',
    'MINT_GREEN': '#AAFFAA',
    'BRIGHT_GREEN': '#55FF00',
    'SPRING_BUD': '#AAFF00',
    'SCREAMING_GREEN': '#55FF55',
    'GREEB': '#00FF00',
    'MALACHITE': '#00FF55',
    'ISLAMIC_GREEN': '#00AA00',
    'KELLY_GREEN': '#55AA00',
    'BRASS': '#AAAA55',
    'LIMERICK': '#AAAA00',
    'AQUAMARINE': '#55FFAA',
    'JAEGAR_GREEN': '#00AA55',
    'MAY_GREEN': '#55AA55',
    'DARK_GREEN': '#005500',
    'ARMY_GREEN': '#555500',
    'MEDIUMSPRING_GREEN': '#00FFAA',
    'MIDNIGHT_GREEN': '#005555',
    'TIFFANY_BLUE': '#00AAAA',
    'CADET_BLUE': '#55AAAA',
    'ELECTRIC_BLUE': '#55FFFF',
    'CYAN': '#00FFFF',
    'CELESTE': '#AAFFFF',
    'COBALT_BLUE': '#0055AA',
    'CERULEAN': '#00AAFF',
    'DUKE_BLUE': '#0000AA',
    'OXFORD_BLUE': '#000055',
    'PICTON_BLUE': '#55AAFF',
    'BLUE': '#0000FF',
    'ULTRAMINE': '#5500FF',
    'BLUE_MOON': '#0055FF',
    'LIGHT_BLUE': '#5555FF',
    'BABY_BLUE': '#AAAAFF',
    'LIBERTY': '#5555AA',
    'INDIGO': '#5500AA',
    'IMPERIAL_PURPLE': '#550055',
    'PURPLE': '#AA00AA',
    'VIOLET': '#AA00FF',
    'LAVENDER': '#AA55FF',
    'JAZZBERRY': '#AA0055',
    'MAGENTA': '#FF00FF',
    'PURPUREUS': '#AA55AA',
    'SHOCKING_PINK': '#FF55FF',
    'BRILLIANT_LAVENDER': '#FFAAFF',
    'BRILLIANT_ROSE': '#FF55AA',
    'FASHION_MAGENTA': '#FF00AA',
    'BULGARIAN': '#550000',
    'APPLE_RED': '#AA0000',
    'ROSEVALUE': '#AA5555',
    'FOLLY': '#FF0055',
    'SUNSET_ORANGE': '#FF5555',
    'MELLON': '#FFAAAA',
    'RED': '#FF0000',
    'WINDSORTON': '#AA5500',
    'ORANGE': '#FF5500',
    'CHROME_YELLOW': '#FFAA00',
    'RAJAH': '#FFAA55',
    'YELLOW': '#FFFF00',
    'ICTERINE': '#FFFF55',
    'PASTEL_YELLOW': '#FFFFAA',
    'DARK_GRAY': '#555555',
    'LIGHT_GRAY': '#AAAAAA',
    'BLACK': '#000000',
    'WHITE': '#FFFFFF'
}
