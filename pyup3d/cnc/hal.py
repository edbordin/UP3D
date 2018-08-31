import time
from pyup3d.cnc.logging_config import *
from pyup3d.cnc.config import *
from pyup3d.uphandle import UP
from pyup3d.cnc.coordinates import Coordinates
from time import sleep
import pyUP3D_com as upcom
from pyup3d.lib.up3d_params import PARA, USB_Params as USB, AXIS
import pyup3d.lib.format_helper as fh
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
        self.printer = UP()
        self.extruder_factor = 1.0
        self.feedrate_factor = 1.0


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
        temp = upcom.getParam(PARA.GET_NOZZLE1_TEMP)
        try:
            ftemp =  fh.float_from_hex(temp)
            if ftemp < 10.0:
                target_temp = upcom.getParam(PARA.PARA_GET_TARGET_TEMP_1)
                ftemp *= target_temp

            return ftemp
        except Exception as e:
            logger.error("get_extruder_temp: {}.. rawTemp {}".format(e, temp))
            raise Hal.HalException(e)


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


    def calibrate(self, x, y, z):
        """ Move head to home position till end stop switch will be triggered.
        Do not return till all procedures are completed.
        :param x: boolean, True to calibrate X axis.
        :param y: boolean, True to calibrate Y axis.
        :param z: boolean, True to calibrate Z axis.
        :return: boolean, True if all specified end stops were triggered.
        """
        logging.info("hal calibrate, x={}, y={}, z={}".format(x, y, z))
        return upcom.init()

    def set_feedrate_factor(self, factor):
        self.feedrate_factor = factor

    def set_extruder_factor(self, factor):
        self.extruder_factor = factor

    def _move(self, coordinates, feedrate, func):
        feedrate = feedrate / 60
        if coordinates.x is not None:
            func(AXIS.X_axis, -1. * coordinates.x, feedrate * self.feedrate_factor)
        if coordinates.y is not None:
            func(AXIS.Y_axis, coordinates.y, feedrate * self.feedrate_factor)
        if coordinates.z is not None:
            func(AXIS.Z_axis, -1. * coordinates.z, feedrate * self.feedrate_factor)
        if coordinates.e is not None:
            # 22.7 found by experimenting
            func(AXIS.E_axis, coordinates.e * 22.7, feedrate * 10 * self.extruder_factor)


    def move(self, coordinates, feedrate):
        """
        Move offset (non absolute coordinates
        :param coordinates:
        :param feedrate:
        :return:
        """
        self._move(coordinates, feedrate, upcom.jog)


    def move_to(self, coordinates, feedrate):
        """
        Move to abs coordinates
        :param coordinates:
        :param feedrate:
        :return:
        """
        self._move(coordinates, feedrate, upcom.jogTo)

    def join(self):
        """ Wait till motors work.
        """
        logging.info("hal join()")
        while (not upcom.isIdle()):
            sleep(1)



    def deinit(self):
        """ De-initialise.
        """
        logging.info("hal deinit()")


    def watchdog_feed(self):
        """ Feed hardware watchdog.
        """
        pass

    def get_fw_version(self):
        return upcom.get_fw_version()

    def get_position(self):
        x = -1.0 * upcom.get_axis_pos(AXIS.X_axis)
        y = upcom.get_axis_pos(AXIS.Y_axis)
        z = -1.0 * upcom.get_axis_pos(AXIS.Z_axis)
        e = upcom.get_axis_pos(AXIS.E_axis)
        return Coordinates(x,y,z,e)

    def get_job_percent(self):
        return upcom.getParam(PARA.PARA_REPORT_PERCENT)
