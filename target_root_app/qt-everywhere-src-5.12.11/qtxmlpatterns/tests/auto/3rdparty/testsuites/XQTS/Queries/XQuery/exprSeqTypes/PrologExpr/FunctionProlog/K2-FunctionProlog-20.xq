(:*******************************************************:)
(: Test: K2-FunctionProlog-20                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Comments cannot be converted into xs:integer. :)
(:*******************************************************:)
declare function local:func() as xs:integer
{
<!--1-->
};
local:func()