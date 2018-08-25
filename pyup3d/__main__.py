from pyup3d.cnc.gmachine import GMachine, GMachineException
from pyup3d.cnc.gcode import GCode, GCodeException
from pyup3d.cnc.hal_virtual import Hal as HalVirtual
from pyup3d.cnc.hal import Hal
from pyup3d.uphandle import UP

import os, pty
import logging
from time import sleep

logger = logging.getLogger('up3d')
logger.setLevel(logging.DEBUG)
up = UP()
printer = Hal()
machine = GMachine(printer)


def do_line(line):
    logger.debug('<< {}'.format(line) )
    try:
        g = GCode.parse_line(line)
        res = machine.do_command(g)
    except (GCodeException, GMachineException) as e:
        print('error ' + str(e))
        return False, ('Error: ' + str(e))
    if res is not None:
        return True, ('ok ' + res)
    else:
        return True, ('ok')

def readline(descriptor):
    line = ""
    try:
        ch = os.read(descriptor, 1)
        # if ('\r' == ch) or ('\n' == ch):
        while True:
            line += ch.decode('utf-8')
            # print("ch >>",ch)
            if (b'\n' == ch):
                if (line.startswith('ok') or line.startswith('error')):
                    return None
                return line
            ch = os.read(descriptor, 1)
    except OSError as e:
        print(e)
        sleep(1)
    return None

def open_virtual_device():
    if up.isUsbConnected():
        master, slave = pty.openpty()
        os.fchmod(slave, 438) # 0666 a+rw
        if os.path.islink('/tmp/tty_up'):
            os.unlink('/tmp/tty_up')

        logger.debug('Start {}'.format(os.ttyname(slave)) )
        os.symlink(os.ttyname(slave), '/tmp/tty_up')
        # os.close(slave)
        up.open()
        return master
    return None

def send_resp(fd, resp):
    wlen = os.write(fd, (resp).encode())
    # logger.debug("send resp: '{}'".format(resp))
    # if wlen:
    #     rd = os.read(fd, wlen+2)
    #     logger.debug("skip({}): {}".format(wlen, rd))
    wlen = os.write(fd, '\r\n'.encode())
    # os.read(fd, wlen + 2)


def main():
    if os.path.islink('/tmp/tty_up'):
        os.unlink('/tmp/tty_up')

    virtual_fd = None
    try:
        while True:
            if virtual_fd is None:
                virtual_fd = open_virtual_device()

            if virtual_fd is None:
                sleep(5)
                continue
            # send_resp(virtual_fd, 'hello')

            while True:
                line = readline(virtual_fd)
                if line is not None:
                    res, resp = do_line(line)
                    logger.debug('>> {}'.format(resp))
                    send_resp(virtual_fd, resp)
    except KeyboardInterrupt:
        pass

    machine.release()
    if virtual_fd is not None:
        os.close(virtual_fd)
    if os.path.islink('/tmp/tty_up'):
        os.unlink('/tmp/tty_up')


if __name__ == "__main__":
    main()
