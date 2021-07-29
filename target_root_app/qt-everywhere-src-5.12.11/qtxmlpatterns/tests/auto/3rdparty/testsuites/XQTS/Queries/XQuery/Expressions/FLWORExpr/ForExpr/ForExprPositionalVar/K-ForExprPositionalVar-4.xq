(:*******************************************************:)
(: Test: K-ForExprPositionalVar-4                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Position variable which is not in scope.     :)
(:*******************************************************:)
for $a at $p1 in (1, 2), $b at $p2 in (1, 2), $c at $p3 in (1, 2) return 1, $p3