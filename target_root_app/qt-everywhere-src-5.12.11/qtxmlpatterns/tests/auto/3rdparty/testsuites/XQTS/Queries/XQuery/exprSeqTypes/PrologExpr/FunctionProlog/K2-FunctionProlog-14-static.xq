(:*******************************************************:)
(: Test: K2-FunctionProlog-14-static                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A focus doesn't propagate into function.     :)
(:*******************************************************:)
declare function local:myFunc()
{
    e[1]
};
local:myFunc()

