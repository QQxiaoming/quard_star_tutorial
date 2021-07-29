(:*******************************************************:)
(: Test: K2-ForExprWithout-3                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A for-expression doesn't create a focus(#2). :)
(:*******************************************************:)
declare function local:myFunc()
{
    for $i in (1, 2, 3) return position()
};
1