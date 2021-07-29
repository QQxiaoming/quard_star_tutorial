(:*******************************************************:)
(: Test: K2-LetExprWithout-6                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The focus is undefined inside the initializing expression for a 'let' variable(#4). :)
(:*******************************************************:)
declare function local:myFunc($arg as node())
{
    let $v := aNameTest return $arg/$v
};
1