(:*******************************************************:)
(: Test: K2-FunctionProlog-5                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A function requiring xs:integer but is passed an xs:decimal. :)
(:*******************************************************:)
declare function local:myFunction($arg as xs:integer) 
{
        $arg
};
local:myFunction(1.0)