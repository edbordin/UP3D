import pyup3d
from pyup3d.cnc.gcode import GCode
import unittest
from unittest import TestCase

class Up_test(TestCase):
    def test_gcode_upload_sd_file(self):
        line = "M28 /b.g"
        # line = "M28"
        gcode = GCode.parse_line(line)
        self.assertEqual(28, gcode.get('M'))


if __name__ == '__main__':
    unittest.main()