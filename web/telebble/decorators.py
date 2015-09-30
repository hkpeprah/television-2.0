from flask import request, abort


def require_apikey(key):
    """
    Decorator for view functions and API requests.  Requires
    that the user pass in the API key for the application.
    """
    def _wrapped_func(view_func):
        def _decorated_func(*args, **kwargs):
            passed_key = request.args.get('key', None)
            if passed_key == key:
                return view_func(*args, **kwargs)
            else:
                abort(401)
        return _decorated_func
    return _wrapped_func
