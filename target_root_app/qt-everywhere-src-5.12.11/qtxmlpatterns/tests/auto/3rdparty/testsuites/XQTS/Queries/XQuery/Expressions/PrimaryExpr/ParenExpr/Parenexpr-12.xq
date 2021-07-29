(: Name: Parenexpr-12 :)
(: Written by: Andreas Behm :)
(: Description: if expression :)

for $x in (1,2) return (if (($x eq 1)) then ($x) else ($x + 1))
