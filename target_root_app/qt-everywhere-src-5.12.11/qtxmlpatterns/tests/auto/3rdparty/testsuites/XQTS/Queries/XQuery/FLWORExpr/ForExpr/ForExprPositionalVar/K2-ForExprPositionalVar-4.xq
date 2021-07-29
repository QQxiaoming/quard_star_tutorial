(:*******************************************************:)
(: Test: K2-ForExprPositionalVar-4                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Cache a positional variable with a let-binding. :)
(:*******************************************************:)
for $i at $pos in (3 to 6)
let $let := $pos + 1
return ($let, $let - 1)