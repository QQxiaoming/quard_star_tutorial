(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-12                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A sort with an unused for-binding.           :)
(:*******************************************************:)
for $a in (3, 2, 1),
    $b in (6, 5, 4)
    order by $a
    return $a