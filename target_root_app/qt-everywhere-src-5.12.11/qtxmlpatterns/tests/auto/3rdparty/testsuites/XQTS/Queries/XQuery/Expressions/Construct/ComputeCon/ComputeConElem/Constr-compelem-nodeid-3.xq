(: Name: Constr-compelem-nodeid-3 :)
(: Written by: Andreas Behm :)
(: Description: Copied comment node has new node identity :)

for $x in <!--comment-->,
    $y in element elem {$x}
return exactly-one($y/comment()) is $x
