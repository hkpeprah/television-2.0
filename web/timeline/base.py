import collections

from fields import ValidationException


def serialize(data):
    """
    Serializes the passed object to make it a valid json object.

    @param data Object to serialize
    @returns Object
    """
    if isinstance(data, SerializableObject):
        return data.json()
    elif isinstance(data, list):
        return map(serialize, data)
    elif hasattr(data, 'value'):
        return serialize(data.value)
    return data

class DictionableObject(dict):
    def __init__(self, *args, **kwargs):
        self.update(*args, **kwargs)
        for attr in dir(self):
            if not attr.startswith('__') and not callable(getattr(self, attr)):
                self.update({attr: getattr(self, attr)})

    def __iter__(self):
        for k, v in self.items():
            yield k, v

    def __getattr__(self, attr):
        return self.get(attr)

    __setattr__ = dict.__setitem__

    __delattr__ = dict.__delitem__

    def __repr__(self):
        dictrepr = dict.__repr__(self)
        return '%s(%s)' % (type(self).__name__, dictrepr)

class SerializableObject(DictionableObject):
    def __init__(self, *args, **kwargs):
        super(SerializableObject, self).__init__(*args, **kwargs)

    def __setattr__(self, name, value):
        if name in self and hasattr(self[name], 'value'):
            self[name].value = value
        else:
            self[name] = value

    def json(self):
        d = dict()
        for (attr, value) in self:
            value = serialize(value)
            if value is not None:
                d[attr] = value
        return d

class ValidatableObject(SerializableObject):
    def __init__(self, *args, **kwargs):
        super(ValidatableObject, self).__init__(*args, **kwargs)

    def validate(self):
        validatable_objects = list((k, v) for k, v in self)
        while len(validatable_objects) > 0:
            name, to_validate = validatable_objects.pop(0)
            if isinstance(to_validate, list):
                validatable_objects += map(lambda f: (name, f), to_validate)
            elif hasattr(to_validate, 'validate'):
                try:
                    to_validate.validate()
                except ValidationException as e:
                    raise ValidationException('Validation error on field %s of %s: %s' % (name, type(self).__name__, str(e)))

    def set(self, key, value):
        if key in self and hasattr(self[key], 'value'):
            self[key].value = value
        else:
            self[key] = value

    def get(self, key, default=None):
        if key in self:
            return self[key]
        return default
