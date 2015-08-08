import os
import logging
import multiprocessing

import mongoengine

from app import app
import db
import worker


if __name__ == '__main__':
    app.config['MONGODB_SETTINGS'] = {
        'db': db.DATABASE_NAME,
        'alias': db.DATABASE_ALIAS
    }

    mongoengine.connect(db.DATABASE_NAME,
        alias=db.DATABASE_ALIAS)


    logging.basicConfig(**{
        'level': logging.DEBUG,
        'filemode': 'w',
        'filename': 'server.log'
    })

    # Start the Worker as a Daemon
    p = multiprocessing.Process(target=worker.main)
    p.daemon = True
    p.start()

    # Start the server as a separate process, but block
    environment = os.environ.get('WSGI_PRODUCTION', None)
    debug = True if environment is not None else False
    p = multiprocessing.Process(target=app.run, kwargs={'debug': debug})
    p.start()
    p.join()
