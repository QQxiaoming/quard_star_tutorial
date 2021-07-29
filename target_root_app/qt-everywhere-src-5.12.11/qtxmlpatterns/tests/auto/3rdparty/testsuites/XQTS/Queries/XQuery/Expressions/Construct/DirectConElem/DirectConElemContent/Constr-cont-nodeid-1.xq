(: Name: Constr-cont-nodeid-1 :)
(: Written by: Andreas Behm :)
(: Description: Copied element node has new node identity :)

for $x in <a/>,
    $y in <elem>{$x}</elem>
return exactly-one($y/a) is $x
