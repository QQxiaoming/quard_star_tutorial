(:*******************************************************:)
(: Test: K2-LetExprWithout-24                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure a name test is reported for using the undefined focus, when appearing as a child of a let binding. :)
(:*******************************************************:)
declare function local:function()
{                    
    let $b := (i/a)
    return 1
};
local:function()