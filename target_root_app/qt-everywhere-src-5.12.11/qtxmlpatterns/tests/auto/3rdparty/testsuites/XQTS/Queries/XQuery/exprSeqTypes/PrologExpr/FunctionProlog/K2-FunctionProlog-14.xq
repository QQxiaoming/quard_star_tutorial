(:*******************************************************:)
(: Test: K2-FunctionProlog-14                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A focus doesn't propagate into function.     :)
(:*******************************************************:)
declare function local:myFunc()
{
    e
};
<e/>/local:myFunc()/1