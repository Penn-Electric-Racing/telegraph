#!/usr/bin/env python3
import os
import time
import curses
import sys
import threading

from google.protobuf.internal.encoder import _VarintBytes
from google.protobuf.message import DecodeError

# TODO: Automatically run the protobuf generator

from stream_pb2 import Packet
from common_pb2 import Empty

import serial

write_lock = threading.Lock()
refresh_lock = threading.Lock()

ser = serial.Serial('/dev/ttyACM0', 112500)

def write_packet(p):
    with write_lock:
        size = p.ByteSize()
        varint = _VarintBytes(size)
        ser.write(varint)

        buf = p.SerializeToString()
        ser.write(buf)
        ser.flush()

def read_varint32():
    mask = (1 << 32) - 1
    result = 0
    shift = 0
    while 1:
      b = int.from_bytes(ser.read(), byteorder='big')
      result |= ((b & 0x7f) << shift)
      if not (b & 0x80):
        result &= mask
        result = int(result)
        return result
      shift += 7
      if shift >= 64:
          raise ValueError('Too many bytes when decoding')

def read_packet():
    size = read_varint32()
    buf = ser.read(size)
    packet = Packet()
    packet.ParseFromString(buf)
    return packet


def fetch_tree():
    packet = Packet()
    packet.fetch_tree.SetInParent()
    write_packet(packet)

    while True:
        p = read_packet()
        if hasattr(p, 'tree'):
            return p.tree

def set_status(id, status):
    if ui:
        ui.set_status(id, status)

def subscribe(id, min_int, max_int):
    set_status(id, 'subscribing...')
    packet = Packet()
    packet.change_sub.var_id = id
    packet.change_sub.min_interval = min_int
    packet.change_sub.max_interval = max_int

    write_packet(packet)

def cancel(id):
    set_status(id, 'cancelling...')
    packet = Packet()
    packet.cancel_sub = id
    write_packet(packet)

# handlers
def on_subscribed(id):
    set_status(id, 'success')

def on_sub_failed(id):
    set_status(id, 'failed')

def on_cancelled(id):
    set_status(id, 'cancelled')

def on_update(id, val):
    set_status(id, str(val))

def run_io():
    while True:
        packet = read_packet()
        event_type = packet.WhichOneof('event')
        if not event_type:
            continue

        event = getattr(packet, event_type)
        if event_type == 'subscribed':
            on_subscribed(event)
        elif event_type == 'sub_failed':
            on_sub_failed(event)
        elif event_type == 'update':
            val = getattr(event.val, event.val.WhichOneof('type'))
            on_update(event.var_id, val)


        # make the ui 
        # write any updates to the terminal
        ui.refresh()
        # write stdout to the output logfile
        sys.stdout.flush()

def run_ping():
    while True:
        time.sleep(0.1)
        packet = Packet()
        packet.ping = 0
        write_packet(packet)
        last_ping = time.time()


# read the tree
ser.reset_input_buffer()
try:
    tree = fetch_tree()
except:
    print('failed to connect, try waiting a bit so the device stops sending data')
    sys.exit(1)

# redirect stdout
sys.stdout = open('log.txt', 'w')
sys.stderr = sys.stdout

# start io thread
io_thread = threading.Thread(target=run_io, daemon=True)
io_thread.start()

ping_thread = threading.Thread(target=run_ping, daemon=True)
ping_thread.start()

class UI:
    def __init__(self, screen):
        self._screen = screen
        self._selection = None

        self._search_cursor_idx = 0
        self._search_text = ''
        self._topic_results = []

        self._scroll = 0
        self._visible_topics = []

        self._topics = []
        self._id_topic_map = {}
        self._topic_id_map = {}

        self._topic_status_map = {}

    def handle_cmd(self, topic, cmd):
        if len(cmd) < 1:
            return
        op = cmd[0]
        if op == 'sub' or op == 's':
            min_int = 0
            if len(cmd) > 1:
                try:
                    min_int = int(cmd[1])
                except ValueError:
                    pass
            if len(cmd) > 2:
                try:
                    max_int = int(cmd[2])
                except ValueError:
                    pass
            max_int = 0
            subscribe(self._topic_id_map[topic], min_int, 0)
        if op == 'cancel' or op == 'c':
            cancel(self._topic_id_map[topic])

    def set_status(self, id, status):
        topic = self._id_topic_map[id]
        self._topic_status_map[topic] = status
        self.redraw_topic(self._id_topic_map[id])

    def refresh(self):
        with refresh_lock:
            self._screen.refresh()

    def populate_topics(self, node, prefix=''):
        n = getattr(node, node.WhichOneof('node'))

        if hasattr(n, 'children') and n.children:
            topic = prefix + '/'
            for c in n.children:
                cn = getattr(c, c.WhichOneof('node'))
                self.populate_topics(c, prefix + '/' + cn.name)
        else:
            topic = prefix

        if hasattr(node, 'variable') or \
            hasattr(node, 'action'):
            id = n.id
            self._topics.append(topic)
            self._id_topic_map[id] = topic
            self._topic_id_map[topic] = id

    def recalculate_results(self):
        parts = self._search_text.split(' ')
        self._topic_results.clear()
        for t in self._topics:
            if t.startswith(parts[0]):
                self._topic_results.append(t)
        if len(self._topic_results) == 1:
            self._selection = self._topic_results[0]

    def draw_topic(self, topic, line, width):
        indicator = '*' if topic == self._selection else ' '
        status = '' if not topic in self._topic_status_map or \
                        not self._topic_status_map[topic] else \
                         self._topic_status_map[topic]
        text = indicator + ' ' + topic + (' - ' + status if len(status) > 0 else '')
        self._screen.addstr(line, 0, text)
        self._screen.clrtoeol()

    def redraw(self):
        self._screen.clear()
        height, width = self._screen.getmaxyx()
        self._visible_topics.clear()
        for i in range(self._scroll, \
                min(height - 1, len(self._topic_results))):
            self._visible_topics.append(self._topic_results[i])

        for line, topic in enumerate(self._visible_topics):
            self.draw_topic(topic, line, width)

        # draw the cursor line
        self._screen.addstr(height - 1, 0, self._search_text)
        self.move_cursor()

    def move_cursor(self):
        height, width = self._screen.getmaxyx()
        self._screen.move(height - 1, self._search_cursor_idx)

    def redraw_topic(self, topic):
        try:
            idx = self._visible_topics.index(topic)
            height, width = self._screen.getmaxyx()
            self.draw_topic(topic, idx, width)
            self.move_cursor()
        except ValueError:
            return

    def handle(self, ch):
        if ord(ch) == 410: # resize
            pass
        elif ord(ch) == 127: # backspace
            self._search_text = self._search_text[:max(0, self._search_cursor_idx - 1)] + \
                    self._search_text[self._search_cursor_idx:]
            self._search_cursor_idx = max(0, self._search_cursor_idx - 1)

            self.recalculate_results()

        elif ord(ch) == 330: # delete
            self._search_text = self._search_text[:self._search_cursor_idx] + \
                                        self._search_text[min(len(self._search_text), \
                                                    self._search_cursor_idx + 1):]
            self.recalculate_results()

        elif ord(ch) == 1: # ctr-a
            self._search_cursor_idx = 0

        elif ord(ch) == 5: # ctr-e
            self._search_cursor_idx = len(self._search_text)

        elif ord(ch) == 260: # left
            self._search_cursor_idx = max(0, self._search_cursor_idx - 1)
            
        elif ord(ch) == 261: # right
            self._search_cursor_idx = min(len(self._search_text),
                                            self._search_cursor_idx + 1)
        elif ord(ch) == 259: # up
            idx = self._topic_results.index(self._selection) \
                if self._selection in self._topic_results else -1
            idx = max(0, idx - 1) if len(self._topic_results) > 0 else -1

            self._selection = self._topic_results[idx] if idx >= 0 else None

        elif ord(ch) == 258: # down
            idx = self._topic_results.index(self._selection) \
                if self._selection in self._topic_results else -1
            idx = min(len(self._topic_results) - 1, idx + 1)

            self._selection = self._topic_results[idx] if idx >= 0 else None

        elif ch == '\n': # enter
            if self._selection is not None \
                    and self._selection in self._topic_results:

                cmd = self._search_text.split(' ')[1:]

                self._search_text = self._search_text.split(' ')[0]
                self._search_cursor_idx = len(self._search_text)

                self.handle_cmd(self._selection, cmd)

        elif ch.isalnum() or ch == '/' or ch == ' ': # insert
            self._search_text = self._search_text[:self._search_cursor_idx] + ch + \
                                self._search_text[self._search_cursor_idx:]
            self._search_cursor_idx = self._search_cursor_idx + 1
            self.recalculate_results()

        self.redraw()

    def run(self):
        self._screen.clear()
        self.recalculate_results()
        self.redraw()
        while True:
            self.refresh()
            c = self._screen.getch()
            self.handle(chr(c))

# start the curses application
stdscr = curses.initscr()
curses.noecho()
curses.cbreak()
stdscr.keypad(True)

try:
    ui = UI(stdscr)
    ui.populate_topics(tree)
    ui.run()
except KeyboardInterrupt:
    curses.endwin()
