import logging
from pyup3d.cnc.gcode import GCode



class gcode_commands:
    def __init__(self):
        self.logger = logging.getLogger(self.__class__.__name__)

    def find_cmd(self, command):
        command = "CMD_"+command.upper().strip()
        return getattr(self, command, None)

    def execute(self, gcode):
        func = self.find_cmd(gcode.command())
        if func is None:
            self.logger.warning("Command {} is not found, {}".format(gcode.command(), gcode))
            return "Error: Unknown gcode"
        else:
            return func(gcode)


    def CMD_G0(self, gcode):
        return "ok G0"

    def CMD_G1(self, gcode):
        return "ok G1"



if __name__ == "__main__":
    cut = gcode_commands()
    line = "G1 X200 F100"
    gcode = GCode.parse_line(line)
    print('Gcode is {}'.format(gcode))
    ret = cut.execute(gcode)
    print("executed {}".format(ret))

    line = "M1 X200 F100"
    gcode = GCode.parse_line(line)
    print('Gcode is {}'.format(gcode))
    ret = cut.execute(gcode)
    print("executed {}".format(ret))
