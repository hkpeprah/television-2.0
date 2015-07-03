from action import Action
from base import ValidatableObject
from fields import *
from layout import Layout
from notification import Notification
from reminder import Reminder
from resources import *


class Pin(ValidatableObject):
    """
    A Timeline Pin object.  This object represents a pin what will display in a
    User's Timeline.
    """
    id = StringField(max_length=64)
    time = DateField(required=True)
    duration = NumberField()
    layout = Layout()
    createNotification = None
    updateNotification = None
    reminders = []
    actions = []

    def add_reminder(self, reminder):
        self.reminders.append(reminder)

    def add_action(self, action):
        self.actions.append(action)

    def add_notification(self, notification_type, layout, time=None):
        notification = Notification()
        if notification_type == 'create':
            notification.layout = layout
            self.createNotification = notification
        elif notification_type == 'update':
            notification.set('time', time)
            notification.layout = layout
            self.updateNotification = notification
        raise TypeError('Unknown notification type: %s', notification_type)
