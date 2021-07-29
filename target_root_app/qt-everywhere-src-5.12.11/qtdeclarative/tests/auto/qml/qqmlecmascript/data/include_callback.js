function go() {
    var a = Qt.include("missing.js", function(o) { test2 = o.status == o.NETWORK_ERROR });
    test1 = (a.status == a.NETWORK_ERROR) && (a.statusText.indexOf("Error opening source file") != -1);

    var b = Qt.include("blank.js", function(o) { test4 = o.status == o.OK });
    test3 = b.status == b.OK

    var c = Qt.include("exception.js", function(o) { test6 = o.status == o.EXCEPTION });
    test5 = c.status == c.EXCEPTION
}

