(:*******************************************************:)
(: Test: K2-LetExprWithout-5                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The focus is undefined inside the initializing expression for a 'let' variable(#3). :)
(:*******************************************************:)
declare function local:myFunc($arg as node())
{
    let $v := aNameTest return $arg/$v
};
local:myFunc(<e/>)