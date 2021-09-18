#!/usr/bin/env python2

import getopt
from gobject import *
import gtk
from tracecmd import *
import time

app = None
data_func_cnt = 0

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
    class EventRef(object):
        '''Inner class to build the trace event index'''
        def __init__(self, index, timestamp, offset, cpu):
            self.index = index
            self.offset = offset
            self.ts = timestamp
            self.cpu = cpu

        def __cmp__(self, other):
            if self.ts < other.ts:
                return -1
            if self.ts > other.ts:
                return 1
            if self.offset < other.offset:
                return -1
            if self.offset > other.offset:
                return 1
            return 0

    # The store only returns the record offset into the trace
    # The view is responsible for looking up the Event with the offset
    column_types = (long,)

    @timing
    def __init__(self, trace):
        gtk.GenericTreeModel.__init__(self)
        self.trace = trace
        self.refs = []
        self._load_trace()
        self._sort()
        self._reindex()

    @timing
    def _load_trace(self):
        print("Building trace index...")
        index = 0
        for cpu in range(0, trace.cpus):
            rec = tracecmd_read_data(self.trace._handle, cpu)
            while rec:
                offset = tep_record_offset_get(rec)
                ts = tep_record_ts_get(rec)
                self.refs.append(self.EventRef(index, ts, offset, cpu))
                index = index + 1
                rec = tracecmd_read_data(self.trace._handle, cpu)
        print("Loaded %d events from trace" % (index))

    @timing
    def _sort(self):
        self.refs.sort()

    @timing
    def _reindex(self):
        for i in range(0, len(self.refs)):
            self.refs[i].index = i

    def on_get_flags(self):
        return gtk.TREE_MODEL_LIST_ONLY | gtk.TREE_MODEL_ITERS_PERSIST

    def on_get_n_columns(self):
        return len(self.column_types)

    def on_get_column_type(self, col):
        return self.column_types[col]

    def on_get_iter(self, path):
        return self.refs[path[0]]

    def on_get_path(self, ref):
        return ref.index

    def on_get_value(self, ref, col):
        '''
        The Event record was getting deleted when passed back via this
        method, now it just returns the ref itself. Use get_event() instead.
        '''
        if col == 0:
            #return self.trace.read_event_at(ref.offset)
            return ref
        return None

    def on_iter_next(self, ref):
        try:
            return self.refs[ref.index+1]
        except IndexError:
            return None

    def on_iter_children(self, ref):
        if ref:
            return None
        return self.refs[0]

    def on_iter_has_child(self, ref):
        return False

    def on_iter_n_children(self, ref):
        if ref:
            return 0
        return len(self.refs)

    def on_iter_nth_child(self, ref, n):
        if ref:
            return None
        try:
            return self.refs[n]
        except IndexError:
            return None

    def on_iter_parent(self, child):
        return None

    def get_event(self, iter):
        '''This allocates a record which must be freed by the caller'''
        try:
            ref = self.refs[self.get_path(iter)[0]]
            ev = self.trace.read_event_at(ref.offset)
            return ev
        except IndexError:
            return None


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
        global app, data_func_cnt

        ev = model.get_event(iter)
        #ev = model.get_value(iter, 0)
        if not ev:
            return False

        if data == "ts":
            cell.set_property("markup", "%d.%09d" % (ev.ts/1000000000,
                                                     ev.ts%1000000000))
            data_func_cnt = data_func_cnt + 1
            if app:
                app.inc_data_func()
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
        self.data_func_label = gtk.Label("0")
        hbox = gtk.HBox()
        hbox.pack_start(gtk.Label("TS Data Func Calls:"), False, False)
        hbox.pack_start(self.data_func_label, False, False)

        vbox = gtk.VBox()
        vbox.pack_start(hbox, False)
        vbox.pack_end(sw)

        self.add(vbox)
        self.show_all()

    def inc_data_func(self):
        global data_func_cnt
        self.data_func_label.set_text(str(data_func_cnt))


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
