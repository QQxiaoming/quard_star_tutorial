(: Name: Constr-compelem-nodeid-4 :)
(: Written by: Andreas Behm :)
(: Description: Copied pi node has new node identity :)

for $x in <?pi content?>,
    $y in element elem {$x}
return exactly-one($y/processing-instruction()) is $x
