(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-45                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Short key-for involved in sorting.           :)
(:*******************************************************:)
for $a in (3, 2, 1),
$b in (6)
stable order by $b
return $a