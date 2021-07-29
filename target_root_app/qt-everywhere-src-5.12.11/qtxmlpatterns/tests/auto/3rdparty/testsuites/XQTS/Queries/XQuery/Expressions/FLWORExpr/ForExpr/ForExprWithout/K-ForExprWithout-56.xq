(:*******************************************************:)
(: Test: K-ForExprWithout-56                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply fn:count() to a for-expression(#2).    :)
(:*******************************************************:)
count(for $i in (1, 2, timezone-from-time(current-time())) return ($i, $i)) eq 6
or
count(for $i in (1, 2, timezone-from-time(current-time())) return ($i, $i)) eq 4
