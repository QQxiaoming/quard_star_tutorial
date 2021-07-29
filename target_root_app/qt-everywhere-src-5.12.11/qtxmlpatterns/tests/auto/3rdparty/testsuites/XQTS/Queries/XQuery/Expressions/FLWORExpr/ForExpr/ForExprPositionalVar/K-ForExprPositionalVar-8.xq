(:*******************************************************:)
(: Test: K-ForExprPositionalVar-8                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A positional variable causing a type error.  :)
(:*******************************************************:)
for $i at $p in (1, 2, 3) return $p + "1"