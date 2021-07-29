(:*******************************************************:)
(: Test: K2-FunctionProlog-23                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Call a function that subsequently calls a recursive function. :)
(:*******************************************************:)

declare function local:recursiveFunction($i as xs:integer) as xs:integer
{
    if($i eq 5)
    then $i
    else local:recursiveFunction($i + 1)
};
declare function local:proxy() as xs:integer
{
    local:recursiveFunction(0) + 3
};
local:proxy()