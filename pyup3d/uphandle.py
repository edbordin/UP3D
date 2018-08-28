import pyUP3D_com as upcom
import usb.core
from pyup3d.lib.up3d_params import PARA, USB_Params as USB
import struct
import pyup3d.lib.format_helper as fh


# find our device

class UP():

    def isUsbConnected(self):
        PIDs = (USB.PID_MINI_A, USB.PID_MINI_M, USB.PID_PLUS, USB.PID_CETUS_S7)
        def isUpUsb(dev):
            if dev.idProduct in PIDs:
                return True
            return False

        return (usb.core.find(idVendor=USB.VID, custom_match=isUpUsb) is not None)

    def open(self):
        return upcom.open()

    def close(self):
        return upcom.close()

    def isIdle(self):
        return upcom.isIdle()

    def init(self):
        return upcom.init()


    def getPercentage(self):
        return upcom.getParam(PARA.PARA_REPORT_PERCENT)

    def getLayer(self):
        return upcom.getParam(PARA.PARA_REPORT_LAYER)

    def getHeight(self):
        height = upcom.getParam(PARA.PARA_REPORT_HEIGHT)
        return fh.float_from_hex(height)

    def getTimeRemaining(self):
        return upcom.getParam(PARA.PARA_REPORT_TIME_REMAIN)
