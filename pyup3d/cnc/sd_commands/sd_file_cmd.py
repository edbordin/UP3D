import pyup3d.cnc.logging_config as logging_config
from pyup3d.cnc.coordinates import *
from pyup3d.cnc.enums import *
from pyup3d.cnc.config import *
from pyup3d.cnc.hal import Hal
import os
import time

import logging

logger = logging.getLogger('cnc.machine')
logger.setLevel(logging.DEBUG)

class SdCard:
    def __init__(self, path):
        self._path = path
        self._selected_file = None

    def _get_path(self, file_name):
        return os.path.abspath(os.path.join(self._path, file_name))

    def _get_selfile_path(self):
        if self._selected_file is not None:
            return self._get_path(self._selected_file)
        else:
            return None

    # M20: List SD Card
    def list(self):
        answer = 'Begin file list\n'
        for file in os.listdir(self._path):
            answer += "{} {}\n".format(file, self._get_path(file))
        # answer += "\n".join(os.listdir(self._path))
        answer += 'End file list\n'

        return answer

    # M21: Init SD card
    def init(self):
        pass

    # M22: Release SD card
    def release(self):
        pass

    # M23: Select SD file
    def selectFile(self, file_name):
        if os.path.isfile(self._get_path(file_name)):
            self._selected_file = file_name
            return "File selected name {} size {}".format(file_name, os.stat(self._get_selfile_path()))
        else:
            return "Error: no file {} found on SD".format(file_name)

    # M24: Start or Resume SD print
    def startOrResumePrint(self):
        """The machine prints from the file selected with the M23 command.
        If the print was previously paused with M25, printing is resumed from that point.
        To restart a file from the beginning, use M23 to reset it, then M24.
        When this command is used to resume a print that was paused,
        RepRapFirmware runs macro file resume.g prior to resuming the print.
        """
        pass

    # M25: Pause SD print
    def pausePrint(self):
        pass

    # M26: Set SD position
    def setPosition(self):
        pass

    # M27: Report SD print status
    def reportPrintStatus(self):
        pass

    # M28: Start SD write
    def startWrite(self):
        pass
    # M29: Stop SD write
    def stopWrite(self):
        pass
    # M30: Delete SD file
    def deleteFile(self):
        pass
    # M31: Print time
    def printTime(self):
        pass
    # M32: Select and Start
    def selectAndStart(self):
        pass
    # M33: Get Long Path
    def getLongPath(self):
        pass
    # M34: SDCard Sorting
    def sorting(self):
        pass

