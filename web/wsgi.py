import os
import sys
import logging
import threading

import gevent.monkey
import mongoengine

from app import app
import db
import worker


app.config['MONGODB_SETTINGS'] = {
    'db': db.DATABASE_NAME,
    'alias': db.DATABASE_ALIAS
}

@app.before_first_request
def setup_application():
    mongoengine.connect(db.DATABASE_NAME, alias=db.DATABASE_ALIAS)

def build_application():
    """
    Starts running the Flask application.
    """
    logging.basicConfig(level=logging.DEBUG,
                        stream=sys.stderr,
                        format='%(asctime)s %(levelname)s - %(message)s')

    gevent.monkey.patch_all()

    t = threading.Thread(target=worker.main)
    t.setDaemon(True)
    t.start()

    app.config['THREADED'] = True
    if os.environ.get('WSGI_PRODUCTION', None) is not None:
        app.config['PORT'] = 9000
    else:
        app.config['PORT'] = 5000
        app.config['DEBUG'] = True
    return app

if __name__ == '__main__':
    app = build_application()
    app.run(threaded=True,
            debug=app.config.get('DEBUG', False),
            port=app.config.get('PORT', 9000))
