(:*******************************************************:)
(: Test: K2-FunctionProlog-6                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A function requiring xs:integer as return value, but is passed xs:decimal. :)
(:*******************************************************:)
declare function local:myFunction() as xs:integer
{
        1.0
};
local:myFunction()