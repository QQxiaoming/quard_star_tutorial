(:*******************************************************:)
(: Test: K2-FunctionProlog-26                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A type declaration whose body doesn't match. :)
(:*******************************************************:)
declare function local:myFunc() as element(foo)
{
    <bar/>
};
local:myFunc()