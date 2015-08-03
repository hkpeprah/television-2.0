import os

import timeline.api


api = timeline.api.TimelineApi(os.environ['PEBBLE_TIMELINE_API_KEY'])
