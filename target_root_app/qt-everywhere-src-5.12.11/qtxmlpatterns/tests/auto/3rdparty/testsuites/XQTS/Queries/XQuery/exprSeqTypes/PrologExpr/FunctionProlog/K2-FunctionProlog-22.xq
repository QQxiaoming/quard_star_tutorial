(:*******************************************************:)
(: Test: K2-FunctionProlog-22                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Conversions are invoked when a text node is returned from a function with declared return type xs:integer. :)
(:*******************************************************:)
declare function local:func() as xs:integer?
{
text {"1"}
};
local:func()