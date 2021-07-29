(: Name: Constr-compelem-nodeid-5 :)
(: Written by: Andreas Behm :)
(: Description: Copied text node has new node identity :)

for $x in <a>text</a>,
    $y in element elem {$x/text()}
return exactly-one($y/text()) is exactly-one($x/text())
