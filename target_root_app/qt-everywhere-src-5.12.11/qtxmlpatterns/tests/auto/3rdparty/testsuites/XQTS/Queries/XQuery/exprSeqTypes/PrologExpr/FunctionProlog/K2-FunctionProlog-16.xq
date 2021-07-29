(:*******************************************************:)
(: Test: K2-FunctionProlog-16                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure three subsequent stars are parsed.    :)
(:*******************************************************:)
declare function local:myFunction() as item()*
{***};
1