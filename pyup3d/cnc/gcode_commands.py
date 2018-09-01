import logging

# from pyup3d.cnc.hal import Hal
from pyup3d.cnc.hal_virtual import Hal
from pyup3d.cnc.gcode import GCode, GCodeException
from pyup3d.cnc.config import *
from pyup3d.cnc.sd_commands.sd_file_cmd import SdCard
# from pyup3d.cnc.coordinates import *



# supported codes for manual control
# for print job (aka SD Print) see pyup3d.cnc.sd.print
class GCodeCommand:
    def __init__(self, sd: SdCard, hal: Hal):
        self.logger = logging.getLogger(self.__class__.__name__)
        self.logger.setLevel(logging.DEBUG)

        self.hal = hal
        self.hal.init()
        self.sd = sd

        self._absoluteCoordinates = True
        self._velocity = min(MAX_VELOCITY_MM_PER_MIN_X,
                             MAX_VELOCITY_MM_PER_MIN_Y,
                             MAX_VELOCITY_MM_PER_MIN_Z,
                             MAX_VELOCITY_MM_PER_MIN_E)
        self._convertCoordinates = 1.0

    def find_cmd(self, command):
        command = "CMD_"+command.upper().strip()
        return getattr(self, command, None)

    def execute(self, gcode):
        if self.sd.isWritingFile and gcode.command() != 'M29':
            return self.sd.write_line(gcode.line)

        func = self.find_cmd(gcode.command())
        if func is None:
            self.logger.warning("Command {} is not found, {}".format(gcode.command(), gcode.line))
            # return "Error: Unknown gcode"
            return "ok"
        else:
            self.logger.debug("Command {} is found".format(gcode.line ))
            try:
                return func(gcode)
            except Exception as e:
                raise GCodeException("CMD_{} fail: {}".format(gcode.command(), e))

    # G0-G1: Linear Move
    def CMD_G0(self, gcode):
        self._velocity = gcode.get('F', self._velocity)
        # check parameters
        if self._velocity < MIN_VELOCITY_MM_PER_MIN:
            raise GCodeException("feed speed too low")
        if self._absoluteCoordinates:
            self.hal.move_to(gcode.coordinates(multiply=self._convertCoordinates), self._velocity)
        else:
            self.hal.move(gcode.coordinates(multiply=self._convertCoordinates), self._velocity)
        return "ok"

    # G1: Linear Move
    def CMD_G1(self, gcode):
        return self.CMD_G0(gcode)

    # G2-G3: Controlled Arc Move
    # G4: Dwell
    # G5: Bézier cubic spline
    # G10: Retract
    # G11: Recover
    # G12: Clean the Nozzle
    # G20: Inch Units
    def CMD_G20(self, gcode):
        self._convertCoordinates = 25.4
        return "ok"
    # G21: Millimeter Units
    def CMD_G21(self, gcode):
        self._convertCoordinates = 1.0
        return "ok"
    # G26: Mesh Validation Pattern
    # G27: Park the nozzle
    # G28: Auto Home
    def CMD_G28(self, gcode):
        self.hal.calibrate(True,True,True)
        return "ok"
    # G29: Mesh Bed Leveling
    # G29: Automatic Bed Leveling
    # G29: Unified Bed Leveling
    # G30: Single Z-Probe
    # G31: Dock Sled
    # G32: Undock Sled
    # G33: Delta Auto Calibration
    # G38.2-G38.3: Probe target
    # G42: Move to mesh coordinate
    # G90: Absolute Positioning
    def CMD_G90(self, gcode):
        self._absoluteCoordinates = True
        return "ok"
    # G91: Relative Positioning
    def CMD_G91(self, gcode):
        self._absoluteCoordinates = False
        return "ok"
    # G92: Set Position
    # M0-M1: Unconditional stop
    def CMD_M2(self, gcode):
        self.logger.debug("{}: End of Program".format(gcode.line))
        return "ok"

    # M3: Spindle CW / Laser On
    # M4: Spindle CCW / Laser On
    # M5: Spindle / Laser Off
    # M17: Enable Steppers
    # M18-M84: Disable steppers
    # M20: List SD Card
    def CMD_M20(self, gcode):
        return self.sd.list()
    # M21: Init SD card
    def CMD_M21(self, gcode):
        return self.sd.init()
    # M22: Release SD card
    # M23: Select SD file
    def CMD_M23(self, gcode):
        file = gcode.getExtra()
        return self.sd.selectFile(file)
    # M24: Start or Resume SD print
    def CMD_M24(self, gcode):
        return self.sd.startOrResumePrint()
    # M25: Pause SD print
    def CMD_M25(self, gcode):
        return self.sd.pausePrint()
    # M26: Set SD position
    def CMD_M26(self, gcode):
        return self.sd.setPosition()
    # M27: Report SD print status
    def CMD_M27(self, gcode):
        return self.sd.reportPrintStatus()
    # M28: Start SD write
    def CMD_M28(self, gcode):
        file = gcode.getExtra()
        return self.sd.startWrite(file)
    # M29: Stop SD write
    def CMD_M29(self, gcode):
        file = gcode.getExtra()
        return self.sd.stopWrite(file)
    # M30: Delete SD file
    def CMD_M30(self, gcode):
        file = gcode.getExtra()
        return self.sd.deleteFile(file)
    # M31: Print time
    def CMD_M31(self, gcode):
        return self.sd.printTime()
    # M32: Select and Start
    def CMD_M32(self, gcode):
        file = gcode.getExtra()
        return self.sd.selectAndStart(file)
    # M33: Get Long Path
    # M34: SDCard Sorting
    # M42: Set Pin State
    # M43: Debug Pins
    # M43 T: Toggle Details (Debug Pins)
    # M48: Probe Accuracy Test
    # M73: Set Print Progress
    # M75: Start Print Job
    # M76: Pause Print Job
    def CMD_M76(self, gcode):
        self.logger.debug("{}: Pause print job".format(gcode.line))
        if (not self.hal.pause()):
            raise GCodeException("Pause fail")
        else:
            return "ok"
    # M77: Stop Print Job
    # M78: Print Job Stats
    # M80: Power On
    # M81: Power Off
    # M82: E Absolute
    # M83: E Relative
    # M84: Disable idle hold
    def CMD_M84(self, gcode):
        self.logger.debug("{}: Disable idle hold".format(gcode.line))
        return "ok" # Up will loose coordinates
    # M85: Inactivity Shutdown
    # M92: Set Axis Steps-per-unit
    # M100: Free Memory
    # M104: Set Hotend Temperature
    def CMD_M104(self, gcode):
        self.logger.debug("{}: Set Hotend Temperature".format(gcode.line))
        self.hal.set_hotend_temp(gcode.get('S'))
        return "ok"

    # M105: Report Temperatures
    def CMD_M105(self, gcode):
        answer = "ok "
        try:
            et = self.hal.get_extruder_temperature()
            answer += "T:{}".format(et)
        except (self.hal.HalException):
            et = None
        try:
            bt = self.hal.get_bed_temperature()
            answer += " B:{}".format(bt)
        except (self.hal.HalException):
            bt = None
        if et is None and bt is None:
            raise GCodeException("can not measure temperature")
        return answer
    # M106: Set Fan Speed
    def CMD_M106(self, gcode):
        self.logger.debug("{}: Set fan speed".format(gcode.line))
        self.hal.set_fan_speed(gcode.get('S'))
        return "ok"
    # M107: Fan Off
    def CMD_M107(self, gcode):
        self.logger.debug("{}: Set fan speed 0".format(gcode.line))
        self.hal.set_fan_speed(0)
        return "ok"
    # M108: Break and Continue
    # M109: Wait for Hotend Temperature
    # M110: Set Line Number
    def CMD_M110(self, gcode):
        return "ok"
    # M111: Debug Level
    # M112: Emergency Stop
    # M113: Host Keepalive
    # M114: Get Current Position
    def CMD_M114(self,gcode):
        p = self.hal.get_position()
        return "ok X:{} Y:{} Z:{} E:{}".format(p.x, p.y, p.z, p.e)
    # M115: Firmware Info
    def CMD_M115(self,gcode):
        fw_ver = self.hal.get_fw_version()
        answer = "ok FIRMWARE_NAME:Cetus3D PROTOCOL_VERSION:1.0 FIRMWARE_VERSION:{}".format(fw_ver)
        return answer
    # M117: Set LCD Message
    # M118: Serial print
    # M119: Endstop States
    # M120: Enable Endstops
    # M121: Disable Endstops
    # M122: TMC Debugging
    # M125: Park Head
    # M126: Baricuda 1 Open
    # M127: Baricuda 1 Close
    # M128: Baricuda 2 Open
    # M129: Baricuda 2 Close
    # M140: Set Bed Temperature
    def CMD_M140(self, gcode):
        self.logger.debug("{}: Set Bed Temperature".format(gcode.line))
        self.hal.set_bed_temp(gcode.get('S'))
        return "ok"
    # M145: Set Material Preset
    # M149: Set Temperature Units
    # M150: Set RGB(W) Color
    # M155: Temperature Auto-Report
    # M163: Set Mix Factor
    # M164: Save Mix
    # M165: Set Mix
    # M190: Wait for Bed Temperature
    # M200: Set Filament Diameter
    # M201: Set Print Max Acceleration
    # M203: Set Max Feedrate
    # M204: Set Starting Acceleration
    # M205: Set Advanced Settings
    # M206: Set Home Offsets
    # M207: Set Firmware Retraction
    # M208: Set Firmware Recovery
    # M209: Set Auto Retract
    # M211: Software Endstops
    # M218: Set Hotend Offset
    # M220: Set Feedrate Percentage
    def CMD_M220(self, gcode):
        self.hal.set_feedrate_factor(gcode.get('S', 100)/100.)
        return "ok"
    # M221: Set Flow Percentage
    def CMD_M221(self, gcode):
        self.hal.set_extruder_factor(gcode.get('S', 100)/100.)
        return "ok"
    # M226: Wait for Pin State
    # M240: Trigger Camera
    # M250: LCD Contrast
    # M260: I2C Send
    # M261: I2C Request
    # M280: Servo Position
    # M290: Babystep
    # M300: Play Tone
    # M301: Set Hotend PID
    # M302: Cold Extrude
    # M303: PID autotune
    # M304: Set Bed PID
    # M350: Set micro-stepping
    # M351: Set Microstep Pins
    # M355: Case Light Control
    # M360: SCARA Theta A
    # M361: SCARA Theta-B
    # M362: SCARA Psi-A
    # M363: SCARA Psi-B
    # M364: SCARA Psi-C
    # M380: Activate Solenoid
    # M381: Deactivate Solenoids
    # M400: Finish Moves
    def CMD_M400(self, gcode):
        self.hal.stopAllMove()
        return "ok"
    # M401: Deploy Probe
    # M402: Stow Probe
    # M404: Set Filament Diameter
    # M405: Filament Width Sensor On
    # M406: Filament Width Sensor Off
    # M407: Filament Width
    # M410: Quickstop
    # M420: Bed Leveling State
    # M421: Set Mesh Value
    # M428: Home Offsets Here
    # M500: Save Settings
    # M501: Restore Settings
    # M502: Factory Reset
    # M503: Report Settings
    # M504: Validate EEPROM contents
    # M540: Endstops Abort SD
    # M600: Filament Change
    # M603: Configure Filament Change
    # M605: Dual Nozzle Mode
    # M665: Delta Configuration
    # M665: SCARA Configuration
    # M666: Set Delta endstop adjustments
    # M666: Set dual endstop offsets
    # M851: Z Probe Offset
    # M852: Bed Skew Compensation
    # M900: Linear Advance Factor
    # M906: TMC Motor Current
    # M907: Set Motor Current
    # M908: Set Trimpot Pins
    # M909: DAC Print Values
    # M910: Commit DAC to EEPROM
    # M911: TMC OT Pre-Warn Condition
    # M912: Clear TMC OT Pre-Warn
    # M913: Set Hybrid Threshold Speed
    # M914: TMC Bump Sensitivity
    # M915: TMC Z axis calibration
    # M928: Start SD Logging
    # M999: STOP Restart



if __name__ == "__main__":
    import os
    hal = Hal()
    sd = SdCard(hal, os.path.join(os.getcwd(), 'tmp'))
    cut = GCodeCommand(sd, hal)
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

    line = "M28 /beep.gcode"
    gcode = GCode.parse_line(line)
    print('Gcode is {}'.format(gcode))
    ret = cut.execute(gcode)
    print("executed {}".format(ret))

    line = "G1 X200"
    gcode = GCode.parse_line(line)
    print('Gcode is {}'.format(gcode))
    ret = cut.execute(gcode)
    print("executed {}".format(ret))

    line = "M29"
    gcode = GCode.parse_line(line)
    print('Gcode is {}'.format(gcode))
    ret = cut.execute(gcode)
    print("executed {}".format(ret))

    line = "M20"
    gcode = GCode.parse_line(line)
    print('Gcode is {}'.format(gcode))
    ret = cut.execute(gcode)
    print("executed {}".format(ret))


    line = "M30 /beep.gcode"
    gcode = GCode.parse_line(line)
    print('Gcode is {}'.format(gcode))
    ret = cut.execute(gcode)
    print("executed {}".format(ret))


    line = "M20"
    gcode = GCode.parse_line(line)
    print('Gcode is {}'.format(gcode))
    ret = cut.execute(gcode)
    print("executed {}".format(ret))


