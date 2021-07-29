(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-11                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A sort where the for-binding is only used as a sort key. :)
(:*******************************************************:)
for $a in (1, 4, 2)
let $i := (1, 3, 2)
order by $a
return $i