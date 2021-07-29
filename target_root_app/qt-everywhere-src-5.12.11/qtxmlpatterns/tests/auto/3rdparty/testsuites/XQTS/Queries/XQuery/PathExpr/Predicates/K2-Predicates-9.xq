(:*******************************************************:)
(: Test: K2-Predicates-9                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use an axis step on an expression which has static type item(), from within a path. :)
(:*******************************************************:)
declare function local:foo($arg as item())
{
    string($arg/@arg)
};
local:foo(<e arg="result"/>)