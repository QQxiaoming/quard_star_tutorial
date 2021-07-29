(:*******************************************************:)
(: Test: K2-FunctionProlog-11                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A function whose name is only '_'.           :)
(:*******************************************************:)

declare function local:_()
{
    1
};
local:_()