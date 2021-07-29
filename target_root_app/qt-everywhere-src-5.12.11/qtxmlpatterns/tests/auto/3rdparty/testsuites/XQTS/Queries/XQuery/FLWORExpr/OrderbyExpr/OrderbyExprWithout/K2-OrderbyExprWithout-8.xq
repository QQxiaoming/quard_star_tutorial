(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-8                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Sorting only involving a let-binding, no for-clause. :)
(:*******************************************************:)
let $i := (1, 3, 2)
order by $i
return $i