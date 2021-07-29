(:*******************************************************:)
(: Test: K2-FunctionProlog-30                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure an invalid value, reached through conversions, is reported as invalid. :)
(:*******************************************************:)
declare function local:foo($arg) as xs:boolean
{
$arg
};
local:foo(<e/>)