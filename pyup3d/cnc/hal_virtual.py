import time
from pyup3d.cnc.logging_config import *
from pyup3d.cnc.config import *


""" This is virtual device class which is very useful for debugging.
    It checks PulseGenerator with some tests.
"""
from pyup3d.cnc.config import *
from pyup3d.cnc.coordinates import Coordinates
from pyup3d.lib.up3d_params import *
import logging

logger = logging.getLogger('hal')
logger.setLevel(logging.DEBUG)



class Hal():

    class HalException(Exception):
        """ Exceptions while processing gcode line.
        """
        pass

    def init(self):
        """ Initialize GPIO pins and machine itself.
        """
        logging.info("initialize hal")
        self.extruderTemp = EXTRUDER_MAX_TEMPERATURE *0.999
        self.extruder_factor = 1.0
        self.feedrate_factor = 1.0
        self.check_temp_extruder = True
        self.job_percent = 0


    def spindle_control(self, percent):
        """ Spindle control implementation 0..100.
        :param percent: Spindle speed in percent.
        """
        logging.info("spindle control: {}%".format(percent))


    def fan_control(self, on_off):
        """Cooling fan control.
        :param on_off: boolean value if fan is enabled.
        """
        if on_off:
            logging.info("Fan is on")
        else:
            logging.info("Fan is off")


    # noinspection PyUnusedLocal
    def extruder_heater_control(self,percent):
        """ Extruder heater control.
        :param percent: heater power in percent 0..100. 0 turns heater off.
        """
        pass


    # noinspection PyUnusedLocal
    def bed_heater_control(self, percent):
        """ Hot bed heater control.
        :param percent: heater power in percent 0..100. 0 turns heater off.
        """
        pass


    def get_extruder_temperature(self):
        """ Measure extruder temperature.
        :return: temperature in Celsius.
        """
        return 20.0


    def get_bed_temperature(self):
        """ Measure bed temperature.
        :return: temperature in Celsius.
        """
        return 20.0
        # return BED_MAX_TEMPERATURE * 0.999


    def disable_steppers(self):
        """ Disable all steppers until any movement occurs.
        """
        logging.info("hal disable steppers")

    def enable_steppers(self):
        logging.info("hal enable steppers")

    def calibrate(self, x, y, z):
        """ Move head to home position till end stop switch will be triggered.
        Do not return till all procedures are completed.
        :param x: boolean, True to calibrate X axis.
        :param y: boolean, True to calibrate Y axis.
        :param z: boolean, True to calibrate Z axis.
        :return: boolean, True if all specified end stops were triggered.
        """
        logging.info("hal calibrate, x={}, y={}, z={}".format(x, y, z))
        return True

    def set_feedrate_factor(self, factor):
        self.feedrate_factor = factor

    def set_extruder_factor(self, factor):
        self.extruder_factor = factor


    def move(self, coordinates, feedrate):
        return True


    def move_to(self, coordinates, feedrate):
        return True

    def join(self):
        return True


    def reached_target_temp(self):
        return False

    def get_fw_version(self):
        return 0

    def get_position(self):
        return Coordinates(0,0,0,0)

    def get_job_percent(self):
        self.job_percent += 10
        ret = self.job_percent
        if ret >= 100:
            self.job_percent = 0
        return ret

    def pause(self):
        return True

    def resume(self):
        return True

    def get_machine_state(self) -> MachineState:
        return MachineState.unknown_status

    def get_program_state(self) -> ProgramState:
        return ProgramState.have_errors

    def get_system_state(self) -> SystemState:
        return SystemState.unknown_error

    def isPrinting(self):
        return False

    def stopAllMove(self):
        return True

    def set_hotend_temp(self, temp):
        return True

    def set_bed_temp(self, temp):
        return True

    def set_fan_speed(self, speed):
        return True

