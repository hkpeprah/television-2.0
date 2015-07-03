from base import ValidatableObject
from fields import (StringField, ChoicesField, NumberField)
from resources import ACTION_TYPES


class Action(ValidatableObject):
    """
    Action object.
    """
    title = StringField(required=True)
    type = ChoicesField(ACTION_TYPES, required=True)
    launch_code = NumberField()
