(:*******************************************************:)
(: Test: K2-ForExprWithout-10                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Make use of many syntactical combinations.   :)
(:*******************************************************:)
for $a in (3, 2, 1)
for $b in (6, 5, 4)
let $c := $a + $b
let $d := $a - $b
let $e := ()
stable order by
$a ascending,
$b descending,
$d empty greatest,
$e empty least,
$c descending empty greatest,
$d ascending empty greatest,
$e descending empty least,
$b ascending empty least
return $a