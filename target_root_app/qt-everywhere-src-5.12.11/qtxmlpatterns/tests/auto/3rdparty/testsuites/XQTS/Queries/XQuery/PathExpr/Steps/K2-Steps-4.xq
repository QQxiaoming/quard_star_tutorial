(:*******************************************************:)
(: Test: K2-Steps-4                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A predicate with last() combined with a name test inside a function. :)
(:*******************************************************:)
declare function local:myFunc()
{
    e[last()]
};
local:myFunc()