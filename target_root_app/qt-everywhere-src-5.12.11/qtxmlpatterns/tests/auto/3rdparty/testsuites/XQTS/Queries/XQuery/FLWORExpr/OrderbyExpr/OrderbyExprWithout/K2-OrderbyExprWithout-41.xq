(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-41                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: order by, with many let bindings inbetween.  :)
(:*******************************************************:)
for $a in (3, 2, 1)
let $b := (2, 1),
$c := (2, 1),
$d := (2, 1),
$e := (2, 1)
order by $a
return $a