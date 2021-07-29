(function(url, resultCollector) {
        var x = new XMLHttpRequest;
        x.open("GET", url);
        x.setRequestHeader("Accept-Language","en-US");
        x.onreadystatechange = function() {
            if (x.readyState == XMLHttpRequest.DONE) {
                resultCollector.responseText = x.responseText
            }
        }
        x.send()
})
