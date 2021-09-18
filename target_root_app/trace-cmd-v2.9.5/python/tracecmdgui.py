#
# Copyright (C) International Business Machines  Corp., 2009
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# 2009-Dec-31:	Initial version by Darren Hart <dvhltc@us.ibm.com>
#

import gobject #delete me ?
import time
import sys
import gtk
from tracecmd import *
from ctracecmdgui import *

"""
Python interface for tracecmd GTK widgets

Python tracecmd applications should be written to this interface. It will be
updated as the tracecmd gui C API changes and try to minimze the impact to
python applications. The ctracecmdgui Python module is automatically generated
using SWIG and it is recommended applications not use it directly.
"""

# In a "real" app these width should be determined at runtime testing max length
# strings in the current font.
TS_COL_W    = 150
CPU_COL_W   = 35
EVENT_COL_W = 150
PID_COL_W   = 75
COMM_COL_W  = 250


def timing(func):
  def wrapper(*arg):
      start = time.time()
      ret = func(*arg)
      end = time.time()
      print('@%s took %0.3f s' % (func.func_name, (end-start)))
      return ret
  return wrapper


class EventStore(gtk.GenericTreeModel):
    # FIXME: get these from the C code: trace_view_store->column_types ...
    @timing
    def __init__(self, trace):
        gtk.GenericTreeModel.__init__(self)
        self.trace = trace
        self.cstore = trace_view_store_new(trace.handle)
        self.gtk_cstore = trace_view_store_as_gtk_tree_model(self.cstore)
        num_rows = trace_view_store_num_rows_get(self.cstore)
        print("Loaded %d events from trace" % (num_rows))

    def on_get_flags(self):
        return trace_view_store_get_flags(self.gtk_cstore)

    def on_get_n_columns(self):
        return trace_view_store_get_n_columns(self.gtk_cstore)

    def on_get_column_type(self, col):
        # I couldn't figure out how to convert the C GType into the python
        # GType. The current typemap converts the C GType into the python type,
        # which is what this function is supposed to return anyway.
        pytype = trace_view_store_get_column_type(self.gtk_cstore, col)
        return pytype

    def on_get_iter(self, path):
        if len(path) > 1 and path[1] != 1:
            return None
        n = path[0]
        rec = trace_view_store_get_row(self.cstore, n)
        return rec

    def on_get_path(self, rec):
        if not rec:
            return None
        start_row = trace_view_store_start_row_get(self.cstore)
        return (trace_view_record_pos_get(rec) - start_row,)

    def on_get_value(self, rec, col):
        # FIXME: write SWIG wrapper to marshal the Gvalue and wrap the rec in an
        # Iter
        pass
        #return trace_view_store_get_value_py(self.cstore, rec, col)

    def on_iter_next(self, rec):
        pos = trace_view_record_pos_get(rec)
        start_row = trace_view_store_start_row_get(self.cstore)
        return trace_view_store_get_row(self.cstore, pos - start_row + 1)

    def on_iter_children(self, rec):
        if rec:
            return None
        return trace_view_store_get_row(self.cstore, 0)

    def on_iter_has_child(self, rec):
        return False

    def on_iter_n_children(self, rec):
        if rec:
            return 0
        return trace_view_store_num_rows_get(self.cstore)

    def on_iter_nth_child(self, rec, n):
        if rec:
            return None
        return trace_view_store_get_row(self.cstore, n)

    def on_iter_parent(self, child):
        return None

    def get_event(self, iter):
        path = self.get_path(iter)
        if not path:
            return None
        rec = trace_view_store_get_row(self.cstore, path[0])
        if not rec:
            return None
        ev = self.trace.read_event_at(trace_view_record_offset_get(rec))
        return ev


class EventView(gtk.TreeView):
    def __init__(self, model):
        gtk.TreeView.__init__(self, model)
        self.set_fixed_height_mode(True)

        ts_col = gtk.TreeViewColumn("Time (s)")
        ts_col.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        ts_col.set_fixed_width(TS_COL_W)
        ts_cell = gtk.CellRendererText()
        ts_col.pack_start(ts_cell, False)
        ts_col.set_cell_data_func(ts_cell, self.data_func, "ts")
        self.append_column(ts_col)

        cpu_col = gtk.TreeViewColumn("CPU")
        cpu_col.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        cpu_col.set_fixed_width(CPU_COL_W)
        cpu_cell = gtk.CellRendererText()
        cpu_col.pack_start(cpu_cell, False)
        cpu_col.set_cell_data_func(cpu_cell, self.data_func, "cpu")
        self.append_column(cpu_col)

        event_col = gtk.TreeViewColumn("Event")
        event_col.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        event_col.set_fixed_width(EVENT_COL_W)
        event_cell = gtk.CellRendererText()
        event_col.pack_start(event_cell, False)
        event_col.set_cell_data_func(event_cell, self.data_func, "event")
        self.append_column(event_col)

        pid_col = gtk.TreeViewColumn("PID")
        pid_col.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        pid_col.set_fixed_width(PID_COL_W)
        pid_cell = gtk.CellRendererText()
        pid_col.pack_start(pid_cell, False)
        pid_col.set_cell_data_func(pid_cell, self.data_func, "pid")
        self.append_column(pid_col)

        comm_col = gtk.TreeViewColumn("Comm")
        comm_col.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        comm_col.set_fixed_width(COMM_COL_W)
        comm_cell = gtk.CellRendererText()
        comm_col.pack_start(comm_cell, False)
        comm_col.set_cell_data_func(comm_cell, self.data_func, "comm")
        self.append_column(comm_col)

    def data_func(self, col, cell, model, iter, data):
        ev = model.get_event(iter)
        #ev = model.get_value(iter, 0)
        if not ev:
            return False

        if data == "ts":
            cell.set_property("markup", "%d.%d" % (ev.ts/1000000000,
                                                   ev.ts%1000000000))
        elif data == "cpu":
            cell.set_property("markup", ev.cpu)
        elif data == "event":
            cell.set_property("markup", ev.name)
        elif data == "pid":
            cell.set_property("markup", ev.pid)
        elif data == "comm":
            cell.set_property("markup", ev.comm)
        else:
            print("Unknown Column:", data)
            return False

        return True


class EventViewerApp(gtk.Window):
    def __init__(self, trace):
        gtk.Window.__init__(self)

        self.set_size_request(650, 400)
        self.set_position(gtk.WIN_POS_CENTER)

        self.connect("destroy", gtk.main_quit)
        self.set_title("Event Viewer")

        store = EventStore(trace)
        view = EventView(store)

        sw = gtk.ScrolledWindow()
        sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_ALWAYS)
        sw.add(view)

        # track how often the treeview data_func is called
        self.add(sw)
        self.show_all()


# Basic builtin test, execute module directly
if __name__ == "__main__":
    if len(sys.argv) >=2:
        filename = sys.argv[1]
    else:
        filename = "trace.dat"

    print("Initializing trace...")
    trace = Trace(filename)
    print("Initializing app...")
    app = EventViewerApp(trace)
    print("Go!")
    gtk.main()
