from base import ValidatableObject
from fields import DateField
from layout import Layout


class Notification(ValidatableObject):
    """
    Notification object.
    """
    def __init__(self, *args, **kwargs):
        self.time = DateField(required=True)
        self.layout = Layout()
        super(Notification, self).__init__()
