(:*******************************************************:)
(: Test: K-ForExprWithout-55                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply fn:count() to a for-expression.        :)
(:*******************************************************:)
count(for $i in (1, 2, current-time()) return ($i, $i)) eq 6