(:*******************************************************:)
(: Test: K2-FunctionProlog-12                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An argument requiring a node, but is passed an integer. :)
(:*******************************************************:)

declare function local:_($arg as node())
{
    $arg
};
local:_(1)