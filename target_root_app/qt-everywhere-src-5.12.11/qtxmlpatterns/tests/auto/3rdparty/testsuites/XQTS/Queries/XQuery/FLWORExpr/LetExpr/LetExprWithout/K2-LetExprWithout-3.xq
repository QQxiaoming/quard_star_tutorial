(:*******************************************************:)
(: Test: K2-LetExprWithout-3                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The focus is undefined inside the initializing expression for a 'let' variable. :)
(:*******************************************************:)
declare function local:myFunc()
{
    let $v := . return $v
};
local:myFunc()