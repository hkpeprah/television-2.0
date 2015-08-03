import os

import mongoengine

import db
from app import app


if __name__ == '__main__':
    app.config['MONGODB_SETTINGS'] = { 'db': db.DATABASE_NAME, 'alias': db.DATABASE_ALIAS }
    mongoengine.connect(db.DATABASE_NAME, alias=db.DATABASE_ALIAS)
    if os.environ.get('WSGI_PRODUCTION') is not None:
        app.run()
    else:
        app.run(debug=True)
