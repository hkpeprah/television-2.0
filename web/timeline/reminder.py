from base import ValidatableObject
from fields import DateField
from layout import Layout


class Reminder(ValidatableObject):
    """
    Reminder object.
    """
    def __init__(self, *args, **kwargs):
        self.time = DateField(required=True)
        self.layout = Layout()
        super(Reminder, self).__init__()
