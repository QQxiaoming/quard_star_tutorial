(:*******************************************************:)
(: Test: K2-Steps-3                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A truth predicate combined with a name test inside a function. :)
(:*******************************************************:)
declare function local:myFunc()
{
    e[true()]
};
local:myFunc()