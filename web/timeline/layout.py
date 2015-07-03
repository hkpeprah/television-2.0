from base import ValidatableObject
from fields import (ChoicesField, StringField)
from resources import (ICONS, COLOURS, LAYOUT_TYPES)


class Layout(ValidatableObject):
    """
    Layout object.
    """
    type = ChoicesField(LAYOUT_TYPES, required=True)
    title = StringField(required=True, min_length=1)
    shortTitle = StringField()
    subtitle = StringField()
    body = StringField(required=True, min_length=1)
    tinyIcon = ChoicesField(ICONS)
    smallIcon = ChoicesField(ICONS, required=False)
    largeIcon = ChoicesField(ICONS, required=False)
    foregroundColor = ChoicesField(COLOURS, required=False)
    backgroundColor = ChoicesField(COLOURS, required=False)
    headings = []
    paragraphs = []

    def add_section(self, header, body):
        self.headings.append(StringField(value=header, min_length=1, required=True))
        self.paragraphs.append(StringField(value=body, min_length=1, required=True))
