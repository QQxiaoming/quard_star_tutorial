(:*******************************************************:)
(: Test: K2-Predicates-8                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use an axis step on an expression which has static type item(), from within a predicate. :)
(:*******************************************************:)
declare function local:foo($arg as item())
{
    $arg[@arg]
};
local:foo(<e arg="">result</e>)