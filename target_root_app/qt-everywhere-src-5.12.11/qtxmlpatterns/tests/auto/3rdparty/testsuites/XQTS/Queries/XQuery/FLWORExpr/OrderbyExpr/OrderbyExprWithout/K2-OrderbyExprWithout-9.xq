(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-9                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Sorting of 4 6 5.                            :)
(:*******************************************************:)
let $i := (1, 3, 2),
    $b := (4, 6, 5)
    order by $b
    return $b