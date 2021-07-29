(:*******************************************************:)
(: Test: K2-FunctionProlog-21                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Processing instructions cannot be converted into xs:integer. :)
(:*******************************************************:)
declare function local:func() as xs:integer
{
<?target 1?>
};
local:func()