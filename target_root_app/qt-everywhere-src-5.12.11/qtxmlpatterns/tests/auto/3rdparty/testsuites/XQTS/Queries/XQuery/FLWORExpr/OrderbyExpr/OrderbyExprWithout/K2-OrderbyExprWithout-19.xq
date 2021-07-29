(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-19                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Multiple atomic values as sort key trigger a type error. :)
(:*******************************************************:)
let $i := (1, 3, 2)
order by $i
return $i