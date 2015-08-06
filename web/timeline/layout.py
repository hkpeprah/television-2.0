from base import ValidatableObject
from fields import (ChoicesField, StringField, NumberField)
from resources import (ICONS, COLOURS, LAYOUT_TYPES)


class Layout(ValidatableObject):
    """
    Layout object.
    """
    def __init__(self, *args, **kwargs):
        self.type = ChoicesField(LAYOUT_TYPES, required=True)
        self.title = StringField(required=True, min_length=1)
        self.shortTitle = StringField(required=False)
        self.subtitle = StringField(required=False)
        self.body = StringField(required=True, min_length=1, max_length=1024)
        self.tinyIcon = ChoicesField(ICONS)
        self.smallIcon = ChoicesField(ICONS, required=False)
        self.largeIcon = ChoicesField(ICONS, required=False)
        self.foregroundColor = ChoicesField(COLOURS, required=False)
        self.backgroundColor = ChoicesField(COLOURS, required=False)
        self.secondaryColor = ChoicesField(COLOURS, required=False)
        self.lastUpdated = NumberField(requird=False)
        self.headings = []
        self.paragraphs = []
        super(Layout, self).__init__()

    def add_section(self, header, body):
        self.headings.append(StringField(value=header, min_length=1, required=True))
        self.paragraphs.append(StringField(value=body, min_length=1, required=True))
