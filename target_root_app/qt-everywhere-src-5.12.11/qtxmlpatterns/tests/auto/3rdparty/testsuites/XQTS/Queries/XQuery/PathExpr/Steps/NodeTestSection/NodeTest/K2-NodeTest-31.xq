(:*******************************************************:)
(: Test: K2-NodeTest-31                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Trigger XPTY0018 inside a function body.     :)
(:*******************************************************:)
declare function local:aFunction()
{
    <e/>/(1, <e/>)
};
1, local:aFunction()