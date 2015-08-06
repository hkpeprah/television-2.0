#!/usr/bin/env python
import os
import sys
import json
import mongoengine
sys.path.append(
    os.path.join(os.path.dirname(__file__), '..'))

import db
import telebble.models as models
from telebble.common import api
import telebble.utils
import timeline.api

if __name__ == '__main__':
    mongoengine.connect(db.DATABASE_NAME, alias=db.DATABASE_ALIAS)

    for media in models.Media.objects.all():
        ids = []
        if telebble.utils.is_crunchyroll_source(media):
            ids.append(str(media._id) + '-premium')
            ids.append(str(media._id) + '-free')
        elif telebble.utils.is_funimation_source(media):
            if 'premium' in media.extra_data:
                if media.extra_data['premium']:
                    ids.append(str(media._id) + '-premium')
                else:
                    ids.append(str(media._id) + '-free')
        else:
            ids.append(str(media._id))

        media.delete()

        for _id in ids:
            status, result = api.delete_shared_pin(_id)
            if not status:
                sys.stderr.write('Error in deleting pin: %s' % json.dumps(result))
                break
