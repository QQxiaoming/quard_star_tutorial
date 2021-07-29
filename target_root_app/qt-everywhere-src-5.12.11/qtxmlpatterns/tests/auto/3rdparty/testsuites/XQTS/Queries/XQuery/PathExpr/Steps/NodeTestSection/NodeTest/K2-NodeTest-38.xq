(:*******************************************************:)
(: Test: K2-NodeTest-38                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Trigger XPTY0018 inside a function body(#5). :)
(:*******************************************************:)
declare function local:aFunction()
{
    (<e/>/(., 4, 5, <e/>/((<e/>, 2))))
};
1, local:aFunction()