(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-10                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Cardinality error in the order spec.         :)
(:*******************************************************:)
for $a in (1, 4, 2)
let $i := (1, 3, 2)
order by $i
return $i