(: Name: Constr-compelem-nodeid-2 :)
(: Written by: Andreas Behm :)
(: Description: Copied attribute node has new node identity :)

for $x in <a b="b"/>,
    $y in element elem {$x/@b}
return $y/@b is $x/@b
