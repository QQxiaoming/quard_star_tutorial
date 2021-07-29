(: Name: Constr-cont-nodeid-2 :)
(: Written by: Andreas Behm :)
(: Description: Copied attribute node has new node identity :)

for $x in <a b="b"/>,
    $y in <elem>{$x/@b}</elem>
return $y/@b is $x/@b
