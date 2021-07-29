(:*******************************************************:)
(: Test: K2-FunctionCallExpr-13                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Have a function which recurses infintely, but which never is called. :)
(:*******************************************************:)
declare function local:foo($arg)
{
    local:foo(local:foo(1))
};
1