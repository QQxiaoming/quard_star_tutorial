(:*******************************************************:)
(: Test: K2-FunctionProlog-19                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Conversions are invoked when a document is returned from a function with declared return type xs:integer. :)
(:*******************************************************:)
declare function local:func() as xs:integer
{
    document {"1"}
};
local:func()