(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-44                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Sort, with a where and let clause in between. :)
(:*******************************************************:)
for $a in (2, 1)
let $b := 1
where true()
order by $a
return $a