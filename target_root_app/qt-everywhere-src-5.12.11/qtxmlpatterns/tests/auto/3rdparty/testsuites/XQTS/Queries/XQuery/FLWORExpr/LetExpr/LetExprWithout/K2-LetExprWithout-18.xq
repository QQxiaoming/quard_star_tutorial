(:*******************************************************:)
(: Test: K2-LetExprWithout-18                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a let-variable that acts as an alias for a function argument. :)
(:*******************************************************:)
declare function local:foo($a as xs:integer)
{
    if($a = 3)
    then $a
    else let $a := $a
         return local:foo($a + 1)
};
local:foo(1)