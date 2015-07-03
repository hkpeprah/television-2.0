import unittest

from action import Action
from api import TimelineApi
from base import ValidatableObject
from fields import *
from layout import Layout
from notification import Notification
from pin import Pin
from reminder import Reminder
from resources import *


class TestValidatableObject(unittest.TestCase):
    def test_getter_setter(self):
        o = ValidatableObject()
        o['cat'] = 'dog'
        self.assertTrue(o['cat'] == 'dog')

    def test_delete(self):
        o = ValidatableObject()
        o['cat'] = 'dog'
        del o['cat']
        self.assertTrue('cat' not in o)

    def test_serializable(self):
        o = ValidatableObject()
        o['cat'] = 'dog'
        data = o.json()
        self.assertTrue('cat' in data)
        self.assertTrue(data['cat'] == 'dog')

    def test_iterable(self):
        o = ValidatableObject()
        keys = ['cat', 'mouse', 'walrus']
        values = ['dog', 'cat', 'seal']
        for k, v in zip(keys, values):
            o[k] = v

        count = 0
        for (i, (k, v)) in enumerate(o):
            count += 1
        self.assertTrue(count == 3)

class TestFields(unittest.TestCase):
    def assertDoesNotRaise(self, exceptionType, callable):
        try:
            callable()
        except exceptionType:
            self.fail('Raised exception when not expected')

    def test_base_field(self):
        f1 = BaseField()
        f2 = BaseField(required=True)
        f3 = BaseField(required=True, value=2)
        self.assertDoesNotRaise(ValidationException, f1.validate)
        self.assertRaises(ValidationException, f2.validate)
        self.assertDoesNotRaise(ValidationException, f3.validate)

        f2.value = 4
        self.assertDoesNotRaise(ValidationException, f2.validate)

    def test_string_field(self):
        f1 = StringField()
        f2 = StringField(value='', min_length=1)
        f3 = StringField(value='dogs', max_length=3)
        f4 = StringField(value=None, min_length=3)
        f5 = StringField(value='cat', min_length=1, max_length=3)
        self.assertDoesNotRaise(ValidationException, f1.validate)
        self.assertRaises(ValidationException, f2.validate)
        self.assertRaises(ValidationException, f3.validate)
        self.assertRaises(ValidationException, f4.validate)
        self.assertDoesNotRaise(ValidationException, f5.validate)

    def test_choices_field(self):
        choices = ['cat', 'dog', 'mouse']
        f1 = ChoicesField(choices=choices, value='cat')
        f2 = ChoicesField(choices=choices, value='walrus')
        self.assertDoesNotRaise(ValidationException, f1.validate)
        self.assertRaises(ValidationException, f2.validate)

    def test_number_field(self):
        f1 = NumberField(value='dog')
        f2 = NumberField(value=3)
        self.assertRaises(ValidationException, f1.validate)
        self.assertDoesNotRaise(ValidationException, f2.validate)

    def test_date_field(self):
        f1 = DateField(value='dog')
        f2 = DateField(value='2012-10-06T04:13:00+00:00Z')
        self.assertRaises(ValidationException, f1.validate)
        self.assertDoesNotRaise(ValidationException, f2.validate)

class TestPin(unittest.TestCase):
    def assertDoesNotRaise(self, exceptionType, callable):
        try:
            callable()
        except exceptionType:
            self.fail('Raised exception when not expected')

    def test_pin_simple(self):
        p = Pin()
        p.id = '400'
        p.time = '2012-10-06T04:13:00+00:00Z'
        p.duration = 200

        self.assertTrue(isinstance(p.id, StringField))
        self.assertTrue(p.id.value == '400')
        self.assertRaises(ValidationException, p.validate)

        p.layout.type = PIN_LAYOUTS['GENERIC']
        p.layout.title = 'Hello World!'
        p.layout.body = 'Hello'
        p.layout.tinyIcon = ICONS['MISC']['SHOW']

        p.validate()
        self.assertDoesNotRaise(ValidationException, p.validate)

        p.layout.foregroundColor = '0xdeadbeef'

        self.assertRaises(ValidationException, p.validate)

        p.layout.foregroundColor = COLOURS['INCH_WORM']
        p.layout.add_section('Hello', 'World')

        self.assertDoesNotRaise(ValidationException, p.validate)


if __name__ == '__main__':
    unittest.main()
