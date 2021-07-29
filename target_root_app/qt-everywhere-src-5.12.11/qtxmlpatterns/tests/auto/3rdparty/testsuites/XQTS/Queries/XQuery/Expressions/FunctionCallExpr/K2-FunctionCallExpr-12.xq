(:*******************************************************:)
(: Test: K2-FunctionCallExpr-12                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Trigger an infinite recursion in one implementation. :)
(:*******************************************************:)
declare function local:func1()
{
    if(local:func2('b'))
    then 3
    else local:func1()
};
declare function local:func2($a)
{
    if(matches("",$a))
    then ()
    else 4
};
local:func1()