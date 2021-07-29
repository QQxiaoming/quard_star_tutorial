(:*******************************************************:)
(: Test: K2-FunctionProlog-7                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A declared return value invokes numeric promotion. :)
(:*******************************************************:)
declare function local:myFunction() as xs:float
{
        4.0
};
(current-time(), 1, 2, "a string", local:myFunction(), 4.0, xs:double("NaN"), current-date())[5]
instance of xs:float