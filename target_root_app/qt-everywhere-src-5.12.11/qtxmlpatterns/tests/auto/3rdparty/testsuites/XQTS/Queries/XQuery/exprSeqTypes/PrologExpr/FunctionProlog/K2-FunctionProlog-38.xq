(:*******************************************************:)
(: Test: K2-FunctionProlog-38                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Trigger a crash in an implementation by having a user declared function with type, that has a text node constructor containing a call to a nonexisting function. :)
(:*******************************************************:)

declare function local:foo() as xs:boolean
{
    text {local:doesNotExist()}
    };
    1
