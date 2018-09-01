import pyup3d.cnc.logging_config as logging_config
from pyup3d.cnc.coordinates import *
from pyup3d.cnc.enums import *
from pyup3d.cnc.config import *
from pyup3d.cnc.hal_virtual import Hal
import os
import time

import logging

logger = logging.getLogger('cnc.machine')
logger.setLevel(logging.DEBUG)

class SdCard:
    def __init__(self, hal:Hal, path):
        self._path = path
        self._selected_file = None
        self.hal = hal
        self.isWritingFile = False
        self._sel_fd = None # selected file descriptor

    def _get_path(self, file_name):
        file_name = file_name.strip()
        if file_name.startswith('/'):
            file_name = file_name[1:]
        # path =  os.path.abspath(os.path.join(self._path, file_name))
        path = (os.path.join(self._path, file_name))
        return path

    def _get_selfile_path(self):
        if self._selected_file is not None:
            return self._get_path(self._selected_file)
        else:
            return None

    # M20: List SD Card
    def list(self):
        answer = 'Begin file list\n'
        for file in os.listdir(self._path):
            answer += "{} {}\n".format(file, os.stat(self._get_path(file)).st_size)
        # answer += "\n".join(os.listdir(self._path))
        answer += 'End file list\nok'

        return answer

    # M21: Init SD card
    def init(self):
        answer = "ok"
        if self.hal.isPrinting():
            answer = self.reportPrintStatus()
        return answer

    # M22: Release SD card
    def release(self):
        return "ok"

    # M23: Select SD file
    def selectFile(self, file_name):
        if os.path.isfile(self._get_path(file_name)):
            self._selected_file = file_name
            # return "File selected {} size {}\nok".format(file_name.strip(), os.stat(self._get_selfile_path()).st_size)
            return "File opened name {} size {}\n" \
                   "File selected\n" \
                   "ok".format(file_name.strip(), os.stat(self._get_selfile_path()).st_size)
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
        logger.debug("startOrResumePrint({})".format(self._selected_file))
        return "ok"

    # M25: Pause SD print
    def pausePrint(self):
        self.hal.pause()
        return "ok"

    # M26: Set SD position
    def setPosition(self):
        return "ok"

    # M27: Report SD print status
    def reportPrintStatus(self):
        job_percent = self.hal.get_job_percent()
        if job_percent >= 100:
            return 'Done printing file\nok'
        return 'SD printing byte {}/100\nok'.format(job_percent)

    # M28: Start SD write
    def startWrite(self, file_name):
        self.isWritingFile = True
        self._sel_fd = open(self._get_path(file_name), 'w')
        if self._sel_fd:
            return "Writing to file {}\nok".format(file_name)
        else:
            return "open failed, File: {}".format(file_name)
    # M29: Stop SD write
    def stopWrite(self, file_name):
        self.isWritingFile = False
        self._sel_fd.close()
        return "Done saving file\nok"

    def write_line(self, line):
        self._sel_fd.write(line + '\r\n')
        return "ok"

    # M30: Delete SD file
    def deleteFile(self, file_name):
        path = self._get_path(file_name)
        logger.debug('remove {}'.format(path))
        os.remove(path)

    # M31: Print time
    def printTime(self):
        pass
    # M32: Select and Start
    def selectAndStart(self, file_name):
        pass
    # M33: Get Long Path
    def getLongPath(self):
        pass
    # M34: SDCard Sorting
    def sorting(self):
        pass

