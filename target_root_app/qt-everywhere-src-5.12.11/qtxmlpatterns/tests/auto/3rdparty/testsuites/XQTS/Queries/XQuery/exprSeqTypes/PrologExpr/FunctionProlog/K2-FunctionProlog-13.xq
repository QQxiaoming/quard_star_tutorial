(:*******************************************************:)
(: Test: K2-FunctionProlog-13                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An argument requiring a node, but is passed an integer(#2). :)
(:*******************************************************:)

declare function local:_($arg as attribute()?)
{
    1
};
local:_(1)