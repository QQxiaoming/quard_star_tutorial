(:*******************************************************:)
(: Test: K2-FunctionProlog-27                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A type declaration whose body doesn't match(#2). :)
(:*******************************************************:)
declare function local:myFunc() as attribute(foo)
{
    <foo/>
};
local:myFunc()