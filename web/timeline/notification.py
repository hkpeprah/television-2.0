from base import ValidatableObject
from fields import DateField
from layout import Layout


class Notification(ValidatableObject):
    """
    Notification object.
    """
    time = DateField(required=True)
    layout = Layout()
