(:*******************************************************:)
(: Test: K-ForExprPositionalVar-30                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Evaluate the positional and binding expression at the same time. :)
(:*******************************************************:)
deep-equal(for $i at $p in (1, 2, 3, 4) return ($i, $p),
           (1, 1, 2, 2, 3, 3, 4, 4))