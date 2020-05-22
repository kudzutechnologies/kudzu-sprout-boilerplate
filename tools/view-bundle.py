#!/usr/bin/env python
from datetime import datetime
from colorama import init, Fore, Style
import atexit
import binascii
import glob
import hashlib
import hexdump
import os
import re
import struct
import subprocess
import sys
import tempfile
import zlib
init()

def deleteFile(name):
  def handler():
    os.unlink(name)

class HardwareConfiguration:
  def getPinName(self, gpio_id):
    return "PIN {}".format(gpio_id)

  def getPinLevelDescription(self, gpio_id, gpio_level):
    levelName = "Low"
    if gpio_level:
      levelName = "High"
    return levelName

class YachtSenseHardware(HardwareConfiguration):

  PIN_CONFIG = [
    ("PIN_BUILTIN_LED", 0),
    ("PIN_UART0_TX", None),
    ("PIN_EN_VBUS", 1),
    ("PIN_UART0_RX", None),
    ("PIN_EN_HIGH_POWER", 1),
    ("PIN_EN_RFM", 0),
    ("", None),
    ("", None),
    ("", None),
    ("", None),
    ("", None),
    ("", None),
    ("PIN_I_MISO", None),
    ("PIN_I_MOSI", None),
    ("PIN_I_SCK", None),
    ("PIN_EN_MEM", 0),
    ("PIN_UART1_RX", None),
    ("PIN_UART1_TX", None),
    ("PIN_E_SCK", None),
    ("PIN_E_MISO", None),
    ("",   None),
    ("PIN_I_SDA", None),
    ("PIN_I_SCL", None),
    ("PIN_E_MOSI", None),
    ("", None),
    ("PIN_EN_IMU", 0),
    ("PIN_USER1", None),
    ("PIN_E_SDA", None),
    ("", None),
    ("", None),
    ("", None),
    ("",       None),
    ("PIN_E_SCL", None),
    ("PIN_EN_SARA", 1),
    ("PIN_INT_EXTERNAL", 1),
    ("PIN_INT_RFM", 1),
    ("PIN_USER2", None),
    ("", None),
    ("",     None),
    ("PIN_POWER_GOOD", 0),
  ]

  def getPinName(self, gpio_id):
    if gpio_id < len(YachtSenseHardware.PIN_CONFIG):
      return YachtSenseHardware.PIN_CONFIG[gpio_id][0]
    else:
      return "PIN {}".format(gpio_id)

  def getPinLevelDescription(self, gpio_id, gpio_level):
    activeLevel = None
    if gpio_id < len(YachtSenseHardware.PIN_CONFIG):
      activeLevel = YachtSenseHardware.PIN_CONFIG[gpio_id][1]

    if activeLevel != None:
      levelName = Fore.RED + "Off"
      if gpio_level == activeLevel:
        levelName = Fore.GREEN + "On"
      if gpio_level:
        levelName += Fore.WHITE + Style.DIM + " (High)"
      else:
        levelName += Fore.WHITE + Style.DIM + " (Low)"
    else:
      levelName = "Low"
      if gpio_level:
        levelName = "High"

    return levelName


class Releases:
  """
  Keeps track of the released versions in order to pick the correct elf for gdb
  """

  def __init__(self, basedir):
    self.releases = []

    r = re.compile(r"(?:^|.*/)([\w_-]+)-(\w+)-v?([0-9]+)\.([0-9]+)\.([0-9]+)\.elf$")
    for fw in glob.glob("{}/*.elf".format(basedir)):
      m = r.match(fw)
      if m:
        aid = zlib.adler32(m.group(1).encode('utf-8'))
        cpu = m.group(2)
        version = ( int(m.group(3)), int(m.group(4)), int(m.group(5)) )
        self.releases.append((fw, aid, cpu, version))

  def findRelease(self, appid, version):
    for (f_path, f_aid, f_cpu, f_ver) in self.releases:
      if f_aid == appid and f_ver == version:
        return f_path
    return None


class BundleSection:
  """
  @brief      Base class for bundle sections
  """

  def __init__(self, parent, id, version, data):
    self.parent = parent
    self.id = id
    self.version = version
    self.data = data
    self._tempFile = None

  def getName(self):
    return "section-{:02x}h".format(self.id)

  def getVersion(self):
    return self.version

  def getSize(self):
    return len(self.data)

  def getFilename(self):
    return "{}-{}.bin".format(self.getName(), self.getVersion())

  def getWeight(self):
    return 5

  def saveToFile(self, fname):
    with open(fname, 'wb') as f:
      f.write(self.data)

  def getTempFilename(self):
    with tempfile.NamedTemporaryFile(delete=False) as temp:
      temp.write(self.data)
      temp.flush()

      atexit.register(deleteFile(temp.name))
      return temp.name

  def getAppElf(self):
    if self.parent.defaultElf != None:
      return self.parent.defaultElf

    appid = None
    appver = None
    for s in self.parent.sections:
      if isinstance(s, FirmwareSection):
        (appid, ) = struct.unpack("<I", s.data[0:4])
        appver = struct.unpack("<HHH", s.data[4:10])

    buildDir = os.path.join(
      os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
      "build"
    )

    elfFile = glob.glob(os.path.join(buildDir, "*.elf"))[0]

    if appid != None and appver != None:
      releaseFile = self.parent.releases.findRelease(appid, appver)
      if releaseFile != None:
        elfFile = releaseFile

    return elfFile

  def addrToLine(self, addr):
    r = re.compile(r" \((.*?):(.*?)\)")
    lr = re.compile(r"^[0-9]+\s*(.*)$")

    i_file = None
    i_line = None
    i_src = None
    elfFile = self.getAppElf()

    sourceDir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    try:
      proc = subprocess.Popen(
        "xtensa-esp32-elf-gdb \"{}\" -ex 'set listsize 1' -ex 'l *{}' --batch --directory={}".format(elfFile, addr, sourceDir),
        shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE
      )
      (out, err) = proc.communicate()

      lines = out.decode("utf-8").split("\n")
      m = next(r.finditer(lines[0]))
      if m != None:
        i_file = m.group(1)
        i_line = int(m.group(2))
        if i_file.startswith(os.getcwd()):
          i_file = i_file[len(os.getcwd())+1:]
      m = lr.match(lines[1])
      if m != None:
        i_src = m.group(1)
      return (i_file, i_line, i_src)

    except Exception as e:
      return (None, None, None)

  def printValue(self, key, valueFormat, *vargs, color=Style.BRIGHT):
    print("{} : {}".format(
      "{:>16s}".format(key),
      color + valueFormat.format(*vargs) + Style.RESET_ALL
    ))

  def printFlags(self, key, value, flagBits, flagMasks={}):
    ret = []
    for name, mask in flagBits.items():
      if (value & mask) != 0:
        ret.append(name)

    for name, mask in flagMasks.items():
      v = value & mask
      while (mask & 1) == 0:
        mask = mask >> 1
        v = v >> 1

      if v > 0:
        ret.append("{}={}".format(name, v))

    if len(ret) == 0:
      ret.append(Style.DIM + "(None)" + Style.RESET_ALL)

    self.printValue(key, "{} {}", ", ".join(ret), Style.DIM + "({:02x})".format(value) + Style.RESET_ALL)

  def readCStr(self, buf):
    vstr = ""
    for b in buf:
      if b == 0:
        break;
      vstr += chr(b)
    return vstr

  def print(self):
    print("(Not implemented)")


class SysInfoSection(BundleSection):
  def getName(self):
    return "sysinfo"

  def resolveEspError(self, id):
    r = re.compile(r"^.*\((.*?)\).*/\*.*?([0-9]+).*$")
    idf_path = os.getenv("IDF_PATH")
    etn_c = os.path.join(idf_path, "components/esp32/esp_err_to_name.c")
    try:
      with open(etn_c, 'r') as f:
        for line in f:
          line = line.strip()
          m = r.match(line)
          if m != None:
            try:
              if int(m.group(2)) == id:
                return "{} (0x{:04x})".format(m.group(1),id)
            except Exception as e:
              continue
    except Exception as e:
      pass

    return "E_UNKNOWN (0x{:04x})".format(id)

  def getResetReason(self, v):
    reasons = [
      "UNKNOWN",
      "POWERON",
      "EXT",
      "SW",
      "PANIC",
      "INT_WDT",
      "TASK_WDT",
      "WDT",
      "DEEPSLEEP",
      "BROWNOUT",
      "SDIO",
    ]
    if v >= len(reasons):
      return str(v)
    else:
      return "{} ({})".format(reasons[v], v)

  def getPanicReason(self, v):
    reasons = [
      "E_NONE",
      "E_OUT_OF_MEMORY",
      "E_TOO_BIG",
      "E_QUEUE_FULL",
      "E_UNEXPECTED_CODE_BRANCH",
      "E_UNINITIALIZED",
      "E_PARAM_ERROR",
      "E_INTENTIONAL",
      "E_HARDWARE_ERROR",
    ]

    if (v & 0x10000000) != 0:
      return self.resolveEspError(v & 0xFFFFFFF)
    elif v >= len(reasons):
      return "E_UNKNOWN 0x{:02x}".format(v)
    else:
      return reasons[v]

  def print(self):
    print()

    (c_feat, c_type, c_mode, c_cores, c_rev) = struct.unpack("<IBBBB", self.data[0:8])
    self.printFlags("CPU Features", c_feat, {
      "EMB_FLASH"   : 0x0001,
      "WIFI_BGN"    : 0x0002,
      "BT_LE"       : 0x0008,
      "BT_CLASSIC"  : 0x0010,
    })
    self.printValue("CPU Type", "{}", c_type)
    self.printValue("CPU Cores", "{}", c_cores)
    self.printValue("CPU Revision", "{:02x}", c_rev)
    print()

    self.printValue("MAC Address", "{}", binascii.hexlify(self.data[8:16]).decode("utf-8"))
    print()

    (h_free, h_min, f_size) = struct.unpack("<III", self.data[16:28])
    self.printValue("Heap Free", "{:,d} b", h_free)
    self.printValue("Heap Min", "{:,d} b", h_min)
    self.printValue("Flash Size", "{:,d} b", f_size)

    (b_v, b_soc, b_flags) = struct.unpack("<HBB", self.data[28:32])
    print()
    self.printValue("Bat. Volt", "{:.2f} V", float(b_v) / 1000)
    self.printValue("Bat. Capacity", "{:d} %", b_soc)
    self.printFlags("Power Flags", b_flags, {
        "EXT_POWER": 0x01
      })

    print()
    if self.version == 1:
      (s_time, s_panic, s_reason) = struct.unpack("<QII", self.data[32:48])
      self.printValue("System Uptime", "{:,.2f} sec", float(s_time) / 1000000)
      self.printValue("Last Panic", "{}", self.getPanicReason(s_panic))
      self.printValue("Reset Reason", "{}", self.getResetReason(s_reason))

    elif self.version == 2:
      (s_time, s_panic, s_breason, s_reason, s_rreason) = struct.unpack("<QIBBB", self.data[32:47])
      self.printValue("System Uptime", "{:,.2f} sec", float(s_time) / 1000000)
      self.printValue("Last Panic", "{}", self.getPanicReason(s_panic))
      self.printValue("Reset Reason", "{}", self.getResetReason(s_reason))
      self.printValue("Last Rst. Reason", "{}", self.getResetReason(s_breason))
      self.printValue("Recovery Reason", "{}", self.getResetReason(s_rreason))

    elif self.version == 3:
      (s_time, s_panic, s_breason, s_reason, s_rreason, s_boots) = struct.unpack("<QIBBBB", self.data[32:48])
      self.printValue("System Uptime", "{:,.2f} sec", float(s_time) / 1000000)
      self.printValue("Last Panic", "{}", self.getPanicReason(s_panic))
      self.printValue("Reset Reason", "{}", self.getResetReason(s_reason))
      self.printValue("Last Rst. Reason", "{}", self.getResetReason(s_breason))
      self.printValue("Recovery Reason", "{}", self.getResetReason(s_rreason))
      self.printValue("Boot Count", "{}", s_boots)


class FirmwareSection(BundleSection):
  def getName(self):
    return "firmware"

  def print(self):
    (a_id, v1, v2, v3, v4) = struct.unpack("<IHHHH", self.data[0:12])
    self.printValue("App ID", "0x{:08x}", a_id)
    self.printValue("App Version", "{}.{}.{}", v1, v2, v3)
    self.printValue("Git Version", "{}", self.readCStr(self.data[12:76]))
    self.printValue("Checksum", "{}", binascii.hexlify(self.data[76:108]).decode("utf-8"))
    self.printValue("Using ELF", "{}", self.getAppElf())


class BacktraceSection(BundleSection):
  def getName(self):
    return "backtrace"

  def getWeight(self):
    return 9

  def parseTrace(self, core, itemsPerCore=16):
    rec = []
    ofs = core * itemsPerCore * 4
    for i in range(0, 16):
      (addr,) = struct.unpack("<I", self.data[ofs:ofs+4])
      if addr == 0:
        break
      ofs += 4
      rec.append(addr)
    return rec

  def printTrace(self, trace, core):
    for addr in trace:
      (file, line, src) = self.addrToLine(addr)
      if file == None:
        print("  {:4d}  {:08x}  ??:??".format(core, addr))
      else:
        print("  {:4d}  {:08x}  {:s}:{}".format(core, addr, file, line))

  def print(self):
    print()
    print("  Core   Address  Location")
    self.printTrace(self.parseTrace(0), 0)
    self.printTrace(self.parseTrace(1), 1)


class CoreDumpSection(BundleSection):
  def getName(self):
    return "coredump"

  def getWeight(self):
    return 10

  def print(self):
    fid = hashlib.sha1(self.data).hexdigest()
    fname = "coredump-{}.bin".format(fid)
    self.saveToFile(fname)

    path = os.getenv("IDF_PATH")

    print()
    print(Style.DIM + "========================================" + Style.RESET_ALL)

    elfFile = self.getAppElf()
    cmdline = "{}/components/espcoredump/espcoredump.py dbg_corefile \\\n".format(path) + \
              "    -t raw -r tools/esp32_rom.elf \\\n" + \
              "    -c \"{}\" \"{}\"".format(fname, elfFile)

    print(cmdline)
    print(Style.DIM + "========================================" + Style.RESET_ALL)

    sys.stdout.write("Launch GDB? [y/N]: ")
    sys.stdout.flush()
    yn = sys.stdin.readline().strip()
    if yn.lower() == "y":
      os.execlp(
        "python",
        "python",
        "{}/components/espcoredump/espcoredump.py".format(path),
        "dbg_corefile",
        "-t", "raw",
        "-r", "tools/esp32_rom.elf",
        "-c", fname,
        elfFile
      )


class TraceDumpSection(BundleSection):
  def getName(self):
    return "trace"

  def getWeight(self):
    return 6

  def parseTrace(self):
    rec = []
    ofs = 0
    while ofs < len(self.data):
      (addr, task, core, action, resv) = struct.unpack("<IBBBB", self.data[ofs:ofs+8])
      rec.append((addr, task, core, action))
      ofs += 8
    return rec

  def action2str(self, action):
    if action & 0x80 != 0:
      if action == 0x86:
        return Style.BRIGHT + Fore.RED + "PANIC" + Style.RESET_ALL
      elif action == 0x85:
        return Fore.RED + "OOPS" + Style.RESET_ALL
      elif action == 0x84:
        return Fore.RED + "Error" + Style.RESET_ALL
      elif action == 0x83:
        return Style.BRIGHT + Fore.YELLOW + "Warn" + Style.RESET_ALL
      elif action == 0x82:
        return Style.BRIGHT + Fore.GREEN + "Info" + Style.RESET_ALL
      elif action == 0x81:
        return Style.BRIGHT + Fore.CYAN + "Debug" + Style.RESET_ALL
      elif action == 0x80:
        return Style.NORMAL + "Debug" + Style.RESET_ALL
      else:
        return "Core %02x" % action
    else:
      return "#%02x" % action

  def print(self):
    recs = self.parseTrace()
    log = ""
    print("     Type      Core Task Address   Location")
    for i in range(0, len(recs)):
      (addr, task, core, action) = recs[i]
      (file, line, src) = self.addrToLine(addr)

      if file == None:
        file = "??"
        line = "??"

      print(" {:2d}) {:<18s}     {:<4d} {:<4X} {:08x}  {:s}:{}".format(
        i+1,
        self.action2str(action),
        core,
        task,
        addr,
        file,
        line
      ))

      if src != None:
        log += " {:2d}) {}\n".format(i+1, src)

    print()
    print("Composed log calls:")
    print(log)


class GPIOSection(BundleSection):
  def getName(self):
    return "gpio"

  def print(self):
    print()

    (pins, hwid) = struct.unpack("<BB", self.data[0:2])

    hardware = YachtSenseHardware()

    ofs = 4
    for i in range(0, pins):
      (pin,pullups,direction,level) = struct.unpack("<BBBB", self.data[ofs:ofs+4])
      self.printValue(hardware.getPinName(pin), "{}", hardware.getPinLevelDescription(pin, level))
      ofs += 4


class ModuleInfoSection(BundleSection):
  def getName(self):
    return "modinfo"

  def printModule(self, chunk):
    print()

    name = self.readCStr(chunk[0:24])
    categ = self.readCStr(chunk[24:48])
    self.printValue("Module", "{}", name, color=Style.BRIGHT + Fore.CYAN)
    self.printValue("Category", "{}", categ)

    levels = []
    for b in chunk[48:56]:
      if b != 0:
        levels.append(str(b))

    self.printValue("Runlevels", "{}", ", ".join(levels))

    if self.version == 1:
      tail = 76
      (uses, activate, busy, m_flags, m_ver, m_sz, r_ver, r_sz) = struct.unpack("<BBBBIIII", chunk[56:tail])

    elif self.version == 2:
      # On version 2 we added the 32-bit module address at the end
      tail = 80
      (uses, activate, busy, m_flags, m_ver, m_sz, r_ver, r_sz, m_addr) = struct.unpack("<BBBBIIIII", chunk[56:tail])
      self.printValue("Base Address", "0x{:08x}", m_addr)

    self.printValue("Uses", "{}", uses)
    self.printValue("Activate Flags", "{:02x}", activate)
    self.printValue("Busy", "{}", "yes" if busy else "no")
    self.printFlags("NV Flags", m_flags, {
        "USER_ENABLED": 0x01,
        "DEGRADED": 0x02,
      }, {
        "FAULTS": 0xC
      })
    self.printValue("(Sav) NV Version", "{}", m_ver)
    self.printValue("(Sav) NV Size", "{}", m_sz)
    self.printValue("(Mod) NV Version", "{}", r_ver)
    self.printValue("(Mod) NV Size", "{}", r_sz)

    for line in hexdump.dumpgen(chunk[tail:tail+m_sz]):
      print("        ", line)

    return tail+m_sz

  def print(self):
    (runlevel, modules) = struct.unpack("<BB", self.data[0:2])
    self.printValue("Runlevel", "0{:02X}", runlevel)
    self.printValue("Modules", "{}", modules)

    ofs = 4
    while ofs < len(self.data):
      ofs += self.printModule(self.data[ofs:])


class DiagnosticsSection(BundleSection):
  def getName(self):
    return "diagnostics"

  def print(self):
    print()

    name = self.readCStr(self.data[0:24])
    (contentType, size) = struct.unpack("<II", self.data[24:32])

    self.printValue("Source", name)
    self.printValue("Content-Type", "0x{:02x}", contentType)
    self.printValue("Size", "{}", size)

    for line in hexdump.dumpgen(self.data[32:32+size]):
      print("        ", line)


class Bundle:
  def __init__(self, releases, defaultElf):
    self.releases = releases
    self.defaultElf = defaultElf
    self.sections = []
    self.section_handlers = [
      SysInfoSection,
      FirmwareSection,
      CoreDumpSection,
      TraceDumpSection,
      ModuleInfoSection,
      GPIOSection,
      DiagnosticsSection,
      BacktraceSection,
    ]

  def _loadSectionV1(self, buf):
    (magic, s_type, s_ver, size) = struct.unpack("<HBBI", buf[0:8])
    if magic != 0xA5A5:
      hexdump.hexdump(buf)
      raise IOError("Unexpected section magic")

    if s_type >= len(self.section_handlers):
      SectionClass = BundleSection
    else:
      SectionClass = self.section_handlers[s_type]

    section = SectionClass(self, s_type, s_ver, buf[8:8+size])
    self.sections.append(section)

    return size + 8

  def _findNextSectionV1(self, buf):
    for i in range(0, len(buf) - 1):
      if buf[i] == 0xA5 and buf[i+1] == 0xA5:
        return i
    return None

  def _loadSectionV2(self, buf):
    (magic, size, s_type, s_ver) = struct.unpack("<IIBB", buf[0:10])
    if magic != 0xA5A55A5A:
      hexdump.hexdump(buf)
      raise IOError("Unexpected section start magic")

    print("found={:02x}.{} (sz={})".format(s_type, s_ver, size))

    scan = False
    footer = buf[12+size:12+size+8]
    if len(footer) < 8:
      print("ERROR: Premature ending of section 0x{:02x}.{:d} ".format(s_type, s_ver))
      ofs = self._findNextSectionV2(buf[4:])
      if ofs == None:
        ofs = len(buf)
      size = ofs - 8 # Footer

      for line in hexdump.dumpgen(buf):
        print("        ", line)

      # Compute new footer
      footer = buf[12+size:12+size+8]
      if len(footer) < 8:
        raise IOError("Unable to perform heuristic detection of faulty section footer")

    # Locate footer
    (crc, magic) = struct.unpack("<II", footer)
    if magic != 0x5A5AA5A5:
      print("ERROR: Invalid ending of section 0x{:02x}.{:d}, data might be junk ".format(s_type, s_ver))

    # Extract data
    data = buf[12:12+size]
    if zlib.crc32(data) != crc:
      print("ERROR: Section 0x{:02x}.{:d} CRC mismatch".format(s_type, s_ver))

    if s_type >= len(self.section_handlers):
      SectionClass = BundleSection
    else:
      SectionClass = self.section_handlers[s_type]

    section = SectionClass(self, s_type, s_ver, data)
    self.sections.append(section)

    return size + 12 + 8

  def _findNextSectionV2(self, buf, bt=(0xA5, 0xA5, 0x5A, 0x5A)):
    for i in range(0, len(buf) - 4):
      if buf[i] == bt[0] and buf[i+1] == bt[1] and \
         buf[i+2] == bt[2] and buf[i+3] == bt[3]:
        return i
    return None

  def load(self, filename):
    with open(filename, 'rb') as f:
      bt = f.read()

      (magic, version) = struct.unpack("<II", bt[0:8])
      if magic != 0xDEADBEEF:
        raise IOError("This does not look like a bundle file")
      if version > 2:
        raise IOError("Unsuported bundle version {}".format(version))

      if version == 1:
        # In version 1 (Before 1.1.249) we have a 16-bit section magic
        # and no section footer
        ofs = 8
        while ofs < len(bt):
          diff = self._loadSectionV1(bt[ofs:])
          if ofs + diff > len(bt):
            print("ERROR: Section range out of bounds, locating next section")
            end = self._findNextSectionV1(bt[ofs+2:])
            if end == None:
              ofs = len(bt)
            else:
              ofs += end + 2
          else:
            ofs += diff

      elif version == 2:
        # In version 2 (From 1.1.250 onwards) we have a 32-bit section magic,
        # and a section footer with a CRC value
        ofs = 8
        while ofs < len(bt):
          diff = self._loadSectionV2(bt[ofs:])
          if ofs + diff > len(bt):
            print("ERROR: Section range out of bounds, locating next section")
            end = self._findNextSectionV2(bt[ofs+2:])
            if end == None:
              ofs = len(bt)
            else:
              ofs += end + 2
          else:
            ofs += diff

    self.sections = sorted(self.sections, key=lambda v: v.getWeight())


def main():
  if len(sys.argv) < 2:
    print("ERROR: Usage print-trace [bundle.bin] [elf.bin]")
    sys.exit(1)

  releases = Releases("releases")

  elf = None
  if len(sys.argv) > 2:
    elf = sys.argv[2]

  bundle = Bundle(releases, elf)
  bundle.load(sys.argv[1])

  for s in bundle.sections:

    print("---===[ {} ]===---".format(Style.BRIGHT + Fore.MAGENTA + s.getName() + Style.RESET_ALL))
    s.print()
    print("")
    # s.saveToFile("tmp/{}".format(s.getFilename()))


if __name__ == '__main__':
  main()
