(:*******************************************************:)
(: Test: K2-FunctionProlog-8                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A recursive function stretching through several function calls. :)
(:*******************************************************:)
declare function local:myFunction($arg as xs:integer) as xs:integer
{
        if($arg eq 1)
        then $arg
        else local:myFunction3($arg - 1)
};
declare function local:myFunction2($arg as xs:integer) as xs:integer
{
        local:myFunction($arg)
};
declare function local:myFunction3($arg as xs:integer) as xs:integer
{
        local:myFunction2($arg)
};
local:myFunction3(3) eq 1