import re

from pyup3d.cnc.coordinates import Coordinates

# extract letter-digit pairs
g_pattern = re.compile('([A-Z])([-+]?[0-9.]+)')
# white spaces and comments start with ';' and in '()'
# clean_pattern = re.compile('\s+|\(.*?\)|;.*')
# clean_pattern = re.compile('\s+|\(.*?\)|;.*|\*.*|^N\d*')
clean_pattern = re.compile('\(.*?\)|;.*|\*.*|^N\d*')


class GCodeException(Exception):
    """ Exceptions while parsing gcode.
    """
    pass

DEFAULT_NONE_COORDS = Coordinates(None, None, None, None)

class GCode(object):
    """ This object represent single line of gcode.
        Do not create it manually, use parse_line() instead.
    """
    def __init__(self, params):
        """ Create object.
        :param params: dict with gcode key-values.
        """
        self.params = params

    def has(self, arg_name):
        """
        Check if value is specified.
        :param arg_name: Value name.
        :return: boolean value.
        """
        return arg_name in self.params

    def get(self, arg_name, default=None, multiply=1.0):
        """ Get value from gcode line.
        :param arg_name: Value name.
        :param default: Default value if value doesn't exist.
        :param multiply: if value exist, multiply it by this value.
        :return: Value if exists or default otherwise.
        """
        return float(self.params.get(arg_name, default)) * multiply

    def coordinates(self, default = DEFAULT_NONE_COORDS, multiply = 1):
        """ Get X, Y and Z values as Coord object.
        :param default: Default values, if any of coordinates is not specified.
        :param multiply: If value exist, multiply it by this value.
        :return: Coord object.
        """
        x = self.get('X', default.x, multiply)
        y = self.get('Y', default.y, multiply)
        z = self.get('Z', default.z, multiply)
        e = self.get('E', default.e, multiply)
        return Coordinates(x, y, z, e)

    def __str__(self):
        return ", ".join(["{}:{}".format(k,v) for k,v in self.params.items()])

    def __repr__(self):
        return str(self)

    def has_coordinates(self):
        """ Check if at least one of the coordinates is present.
        :return: Boolean value.
        """
        return 'X' in self.params or 'Y' in self.params or 'Z' in self.params \
               or 'E' in self.params

    def radius(self, default, multiply):
        """ Get radius for circular interpolation(I, J, K or R).
        :param default: Default values, if any of coords is not specified.
        :param multiply: If value exist, multiply it by this value.
        :return: Coord object.
        """
        i = self.get('I', default.x, multiply)
        j = self.get('J', default.y, multiply)
        k = self.get('K', default.z, multiply)
        return Coordinates(i, j, k, 0)

    def command(self):
        """ Get value from gcode line.
        :return: String with command or None if no command specified.
        """
        if 'G' in self.params:
            return 'G' + self.params['G']
        if 'M' in self.params:
            return 'M' + self.params['M']
        return None

    @staticmethod
    def parse_line(line):
        """ Parse line.
        :param line: String with gcode line.
        :return: gcode objects.
        """
        # line = line.upper().strip()
        line = line.strip()
        line = re.sub(clean_pattern, '', line)
        print("clean line: " + line)
        if len(line) == 0:
            return None
        if line[0] == '%':
            return None
        m = g_pattern.findall(line)
        additional = re.sub(g_pattern, '', line)
        print("found: ")
        print(m)
        print("addit:" + additional)
        if not m:
            raise GCodeException('gcode not found: {}'.format(line))
        # if len(''.join(["%s%s" % i for i in m])) != len(line):
        #     raise GCodeException('extra characters in line {}'.format(m))
        # noinspection PyTypeChecker
        params = dict(m)
        params['extra'] = additional
        # if len(params) != len(m):
        #     raise GCodeException('duplicated gcode entries: {}'.format(line))
        if 'G' in params and 'M' in params:
            raise GCodeException('g and m command found: {}'.format(line))
        return GCode(params)

if __name__ == "__main__":
    line = "M28 /b.g"
    # line = "M28"
    gcode = GCode.parse_line(line)
    print(gcode)
