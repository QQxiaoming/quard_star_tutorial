(:*******************************************************:)
(: Test: K2-NodeTest-36                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Trigger XPTY0018 inside a function body(#3). :)
(:*******************************************************:)
declare function local:aFunction()
{
    (1, 2, 3, (4, <e/>/(<e/>, 2)))
};
1, local:aFunction()