import re
import datetime


class ValidationException(Exception):
    pass

class BaseField(object):
    def __init__(self, *args, **kwargs):
        for (k, v) in kwargs.iteritems():
            setattr(self, k, v)
        self.value = getattr(self, 'value', None)
        self.required = getattr(self, 'required', False)

    def validate(self):
        if self.required and self.value is None:
            raise ValidationException('Error: field is required')

class ChoicesField(BaseField):
    def __init__(self, choices, *args, **kwargs):
        super(ChoicesField, self).__init__(*args, **kwargs)
        self.choices = choices
        self.required = kwargs['required'] if 'required' in kwargs else True

    def validate(self):
        super(ChoicesField, self).validate()
        choices = self.choices
        if self.required or self.value is not None:
            while not isinstance(choices, list):
                if isinstance(choices, dict):
                    choices = flatten(choices.values())
                else:
                    raise ValidationException('Error: choices are not iterable')
            if self.value not in choices:
                raise ValidationException('Error: %s not a valid choice' % self.value)

class StringField(BaseField):
    def __init__(self, *args, **kwargs):
        super(StringField, self).__init__(*args, **kwargs)

    def validate(self):
        super(StringField, self).validate()
        value = self.value
        min_length = getattr(self, 'min_length', None)
        max_length = getattr(self, 'max_length', None)
        if value is None and min_length is None:
            return None
        elif not isinstance(value, basestring):
            raise ValidationException('Error: given value is not a string')
        elif min_length is not None and len(value) < min_length:
            raise ValidationException('Error: minimum length is %s' % min_length)
        elif max_length is not None and len(value) > max_length:
            raise ValidationException('Error: maximum length is %s' % max_length)

class NumberField(BaseField):
    def __init__(self, *args, **kwargs):
        super(NumberField, self).__init__(*args, **kwargs)

    def validate(self):
        super(NumberField, self).validate()
        value = self.value
        if value is not None and not isinstance(value, int):
            raise ValidationException('Error: expected an integer')

class DateField(BaseField):
    def __init__(self, *args, **kwargs):
        super(DateField, self).__init__(*args, **kwargs)

    def validate(self):
        super(DateField, self).validate()
        value = self.value
        if value is not None:
            try:
                tz_patt = r'[+][0-9][0-9]:[0-9][0-9]Z'
                m = re.search(tz_patt, value)
                if m:
                    value = value.replace(m.group(0), '')
                value = value.replace('Z', '')
                datetime.datetime.strptime(value, '%Y-%m-%dT%H:%M:%S')
            except Exception as e:
                raise ValidationException('Error: Invalid date field: %s' % self.value)

def flatten(obj):
    """
    Flattens an object into a list of base values.
    """
    if isinstance(obj, list) or isinstance(obj, dict):
        l = []
        to_flatten = obj if isinstance(obj, list) else obj.values()
        for sublist in map(flatten, to_flatten):
            if isinstance(sublist, list):
                l += flatten(sublist)
            else:
                l.append(sublist)
        return l
    return obj
