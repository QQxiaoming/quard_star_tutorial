(:*******************************************************:)
(: Test: K2-ForExprPositionalVar-1                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Combine fn:remove() and a positional for-variable. :)
(:*******************************************************:)
for $i at $p
in remove((1, 2, 3), 10)
return $p