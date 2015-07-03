from base import ValidatableObject
from fields import DateField
from layout import Layout


class Reminder(ValidatableObject):
    """
    Reminder object.
    """
    time = DateField(required=True)
    layout = Layout()
