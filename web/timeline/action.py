from base import ValidatableObject
from fields import (StringField, ChoicesField, NumberField)
from resources import ACTION_TYPES


class Action(ValidatableObject):
    """
    Action object.
    """
    def __init__(self, *args, **kwargs):
        self.title = StringField(required=True)
        self.launchCode = NumberField(required=False)
        self.type = ChoicesField(ACTION_TYPES, required=True)
        super(Action, self).__init__()
