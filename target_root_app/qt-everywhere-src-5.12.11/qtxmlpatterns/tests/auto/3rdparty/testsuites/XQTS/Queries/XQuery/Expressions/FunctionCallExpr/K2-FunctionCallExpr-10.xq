(:*******************************************************:)
(: Test: K2-FunctionCallExpr-10                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that nodes, when passed through function arguments, have proper node identities. :)
(:*******************************************************:)
declare variable $a := <a/>;
declare function local:testSingleNodeIdentity($node as node())
{
    $node is $node
};
declare function local:testDoubleNodeIdentity($a as node(), $b as node())
{
    $a is $b
};
local:testSingleNodeIdentity(<a/>),
local:testDoubleNodeIdentity(<a/>, <b/>),
local:testDoubleNodeIdentity($a, $a)