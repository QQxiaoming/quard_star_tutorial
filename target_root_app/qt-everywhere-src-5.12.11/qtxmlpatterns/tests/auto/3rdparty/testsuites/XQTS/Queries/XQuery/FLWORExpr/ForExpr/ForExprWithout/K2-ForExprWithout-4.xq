(:*******************************************************:)
(: Test: K2-ForExprWithout-4                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A for-expression with a positional variable doesn't create a focus. :)
(:*******************************************************:)
declare function local:myFunc()
{
    for $i at $p in (1, 2, 3) return position()
};
local:myFunc()